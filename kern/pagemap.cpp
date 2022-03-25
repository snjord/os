
#include "inc/pagemap.h"

PageMap::PageMap(size_t totalPagedBytes)
    : m_Pages(reinterpret_cast<PAGE_ENTRY *>(PAGES_ADDR)),
      m_PDT(reinterpret_cast<addr_t *>(PDT_ADDR)),
      m_PT(reinterpret_cast<addr_t *>(PT_ADDR))
{
    if (totalPagedBytes > 1073741824)
    {
        // a maximum of 1GB is currently supported
        totalPagedBytes = 1073741824;
    }

    zero(&m_Pages[0],       PTE_PER_PT * PT_PER_PDT * sizeof(PAGE_ENTRY));
    zero(&m_SuperPages[0],  sizeof(m_SuperPages));
    zero(&m_PDT[0],         PT_PER_PDT * sizeof(addr_t));
    zero(&m_PT[0],          PTE_PER_PT * PT_PER_PDT * sizeof(addr_t));

    unsigned int const totalPages = totalPagedBytes / PAGE_SIZE;

    unsigned int currPage       = 0;
    unsigned int superPageBytes = MAX_SUPERPAGE_SIZE;
    unsigned int pagesPerSuper  = superPageBytes / PAGE_SIZE;
    PAGE_ENTRY  *pPrev          = NULL;

    // give half the paged area to the largest superpage
    unsigned int superPageCount = totalPages / (2 * pagesPerSuper);

    // top list gets the first page
    m_SuperPages[SUPERPAGE_LISTS - 1] = &m_Pages[0];

    // prepare first page
    CreateSuperPage(m_Pages[currPage], pagesPerSuper);

    pPrev = &m_Pages[currPage];
    currPage += pagesPerSuper;

    for (unsigned int x = 1; x < superPageCount; x++)
    {
        CreateSuperPage(m_Pages[currPage], pagesPerSuper);

        pPrev->next = &m_Pages[currPage];
        pPrev = &m_Pages[currPage];

        currPage += pagesPerSuper;
    }

    // evenly distributing the remaining pages
    size_t       remainingPages  = totalPages - (pagesPerSuper * superPageCount);
    unsigned int maxPagesPerList = remainingPages / (SUPERPAGE_LISTS - 1);

    // loop through inner superpage lists (exclude largest and smallest)
    for (unsigned int x = 1; x < SUPERPAGE_LISTS - 1; x++)
    {
        m_SuperPages[SUPERPAGE_LISTS - (1 + x)] = &m_Pages[currPage];

        superPageBytes = superPageBytes >> SUPERPAGE_SIZE_MULT;
        pagesPerSuper  = superPageBytes / PAGE_SIZE;

        superPageCount = maxPagesPerList / pagesPerSuper;

        // prepare first page
        CreateSuperPage(m_Pages[currPage], pagesPerSuper);

        pPrev = &m_Pages[currPage];
        currPage += pagesPerSuper;

        for (unsigned int y = 1; y < superPageCount; y++)
        {
            CreateSuperPage(m_Pages[currPage], pagesPerSuper);

            pPrev->next = &m_Pages[currPage];
            pPrev = &m_Pages[currPage];

            currPage += pagesPerSuper;
        }

        remainingPages -= pagesPerSuper * superPageCount;
    }

    m_SuperPages[0] = &m_Pages[currPage];

    // prepare first page
    CreateSuperPage(m_Pages[currPage], 1);

    pPrev = &m_Pages[currPage];
    currPage++;

    // give the remaining pages to the final page list
    for (unsigned int x = 1; x < remainingPages; x++)
    {
        CreateSuperPage(m_Pages[currPage], 1);

        pPrev->next = &m_Pages[currPage];
        pPrev = &m_Pages[currPage];
        currPage++;
    }

    addr_t *PML4T = reinterpret_cast<addr_t *>(PML4T_ADDR);
    addr_t *PDPT  = reinterpret_cast<addr_t *>(RemoveFlags(PML4T[0]));

    // setup a single pdpt entry, can address up to 1gb
    unsigned short idxPDPT = (PAGE_SIZE * PTE_PER_PT * PT_PER_PDT) / START_VIRT;

    PDPT[idxPDPT] = reinterpret_cast<addr_t>(&m_PDT[0]) | 3;

    // setup pdt
    for (unsigned int x = 0; x < PT_PER_PDT; x++)
    {
        m_PDT[x] = reinterpret_cast<addr_t>(&m_PT[x * PTE_PER_PT]) | 3;
    }
}

PageMap::~PageMap(void)
{
}

void
PageMap::CreateSuperPage(PAGE_ENTRY &start, unsigned int pages)
{
    start.next          = NULL;
    start.member_pages  = pages;
    start.superpage_seg = false;

    PAGE_ENTRY *pCurr = &start + 1;

    for (unsigned int x = 1; x < pages; x++)
    {
        pCurr->super = &start;
        pCurr->member_pages  = pages;
        pCurr->superpage_seg = true;

        pCurr++;
    }

    return;
}

addr_t
PageMap::AllocPages(unsigned long pages)
{
    PAGE_ENTRY **ppSuperList = GetPageList(pages);
    PAGE_ENTRY  *pCandidate  = *ppSuperList;
    PAGE_ENTRY  *pCurr       = pCandidate;

    unsigned short const superPageMembers = pCandidate->member_pages;
    unsigned long        matchedPages     = superPageMembers;

    // search for consecutive pages
    while (matchedPages < pages)
    {
        if (pCurr->next == pCurr + superPageMembers)
        {
            matchedPages += superPageMembers;
        }
        else
        {
            pCandidate = pCurr->next;
            matchedPages = superPageMembers;
        }

        pCurr = pCurr->next;
    }

    // remove matched region from the superpage pool
    PAGE_ENTRY *pBefore = (*ppSuperList != pCandidate) ? pCandidate - 1 : NULL;
    PAGE_ENTRY *pAfter  = pCandidate + pages;

    if (pBefore)
    {
        if (pBefore->superpage_seg)
        {
            pBefore = pBefore->super;
        }

        pBefore->next = pAfter;
    }
    else
    {
        *ppSuperList = pAfter;
    }

    // commit pages
    for (pCurr = pCandidate; pCurr != pAfter; pCurr++)
    {
        pCurr->present  = true;
        pCurr->writable = true;

        CommitPage(*pCurr);
    }

    return GetVirtualAddr(*pCandidate);
}

void
PageMap::CommitPage(PAGE_ENTRY &page)
{
    addr_t virt = GetVirtualAddr(page);
    addr_t phys = GetPhysicalAddr(page);

    unsigned short flags = 0;

    if (page.present)
    {
        flags = flags | PAGE_FLAG_PRESENT;
    }

    if (page.writable)
    {
        flags = flags | PAGE_FLAG_WRITE;
    }

    MapPage(virt, phys, flags);

    return;
}

void
PageMap::MapPage(addr_t virt, addr_t phys, unsigned short flags)
{
    // virtual and physical addresses are assumed to match if there's
    // no entry in m_MappingTable for a given physical address

    addr_t         currTableVirtAddr;
    addr_t         currTablePhysAddr;

    unsigned short tableFlags = PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;

    unsigned short idxPML4T = (virt & 0xff8000000000) >> 39;
    unsigned short idxPDPT  = (virt & 0x7fc0000000)   >> 30;
    unsigned short idxPDT   = (virt & 0x3fe00000)     >> 21;
    unsigned short idxPT    = (virt & 0x1ff000)       >> 12;

    addr_t *PML4T = reinterpret_cast<addr_t *>(PML4T_ADDR);

    currTablePhysAddr = RemoveFlags(PML4T[idxPML4T]);

    if (currTablePhysAddr == NULL)
    {
        // create pdpt
        currTablePhysAddr = CreateTable();
        PML4T[idxPML4T]   = currTablePhysAddr | tableFlags;
    }

    currTableVirtAddr = m_MappingTable.contains(currTablePhysAddr)
                            ? m_MappingTable[currTablePhysAddr]
                            : currTablePhysAddr;

    addr_t *PDPT  = reinterpret_cast<addr_t *>(currTableVirtAddr);

    currTablePhysAddr = RemoveFlags(PDPT[idxPDPT]);

    if (currTablePhysAddr == NULL)
    {
        // create pdt
        currTablePhysAddr = CreateTable();
        PDPT[idxPDPT] = currTablePhysAddr | tableFlags;
    }

    currTableVirtAddr = m_MappingTable.contains(currTablePhysAddr)
                            ? m_MappingTable[currTablePhysAddr]
                            : currTablePhysAddr;

    addr_t *PDT   = reinterpret_cast<addr_t *>(currTableVirtAddr);

    currTablePhysAddr = RemoveFlags(PDT[idxPDT]);

    if (currTablePhysAddr == NULL)
    {
        // create pt
        currTablePhysAddr = CreateTable();
        PDT[idxPDT] = currTablePhysAddr | tableFlags;
    }

    currTableVirtAddr = m_MappingTable.contains(currTablePhysAddr)
                            ? m_MappingTable[currTablePhysAddr]
                            : currTablePhysAddr;

    addr_t *PT    = reinterpret_cast<addr_t *>(currTableVirtAddr);

    PT[idxPT] = (phys & ~(PAGE_ALIGNMENT - 1)) | flags;

    Invalidate(virt);

    return;
}

void
PageMap::MapRange(addr_t virt, addr_t phys, size_t bytes)
{
    addr_t vCurr = virt & ~(PAGE_ALIGNMENT - 1);
    addr_t pCurr = phys & ~(PAGE_ALIGNMENT - 1);

    // need to fit the number of bytes aligned up to the next page boundry
    addr_t vEnd  = vCurr + ((bytes & ~(PAGE_ALIGNMENT - 1)) + PAGE_ALIGNMENT);

    while (vCurr != vEnd)
    {
        MapPage(vCurr, pCurr, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE);

        vCurr += PAGE_SIZE;
        pCurr += PAGE_SIZE;
    }

    return;
}

