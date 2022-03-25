
#ifndef PAGEMAP_H
#define PAGEMAP_H

#include "defs.h"
#include "map.h"

#define PAGES_ADDR          0x500000
#define PDT_ADDR            0x1000000
#define PT_ADDR             0x1001000
#define PML4T_ADDR          0x100000

#define START_VIRT          0x40000000
#define START_PHYS          0x2000000

#define PAGE_SIZE           4096
#define PAGE_ALIGNMENT      PAGE_SIZE

#define PAGE_FLAG_PRESENT   0x1
#define PAGE_FLAG_WRITE     0x2

#define PAGE_TABLE_SIZE     (512 * 8)
#define PTE_PER_PT          512
#define PT_PER_PDT          512

#define MAX_SUPERPAGE_SIZE  0x400000
#define SUPERPAGE_LISTS     6

#define SUPERPAGE_SIZE_MULT 2

// superpages must fit at least 1/(2^(n)) of the requested alloc
#define PAGE_SEG_DIVISOR    2

typedef struct pageentry_s
{
    union
    {
        // used when superpage_seg is true
        struct pageentry_s *next;

        // used when superpage_seg is false
        struct pageentry_s *super;
    };

    unsigned short member_pages;

    bool present;
    bool writable;

    bool superpage_seg;

    unsigned char unused[19];
} PAGE_ENTRY, *PPAGE_ENTRY;

typedef struct pagetable_s
{
    size_t entry[512];
    unsigned char unused[4088];
} PAGE_TABLE;

class PageMap
{
public:
    PageMap(size_t totalPagedBytes);
    virtual ~PageMap(void);

    addr_t GetVirtualAddr(PAGE_ENTRY &entry) const
    {
        return START_VIRT + (&entry - &m_Pages[0]) * PAGE_SIZE;
    }

    addr_t GetPhysicalAddr(PAGE_ENTRY &entry) const
    {
        return START_PHYS + (&entry - &m_Pages[0]) * PAGE_SIZE;
    }

    addr_t VirtualToPhysical(addr_t virt) const
    {
        unsigned int offset  = virt - Aligned(virt);
        unsigned int idxPage = (Aligned(virt) - START_VIRT) / PAGE_SIZE;
        PAGE_ENTRY  &page    = m_Pages[idxPage];

        return GetPhysicalAddr(page) + offset;
    }

    PAGE_ENTRY* GetPageForPhysicalAddr(size_t addr) const
    {
        if (addr < START_PHYS)
        {
            return NULL;
        }

        unsigned int pageIndex = (Aligned(addr) - START_PHYS) / PAGE_SIZE;

        PAGE_ENTRY *pFound = &m_Pages[pageIndex];

        if (pFound->superpage_seg == true)
        {
            pFound = pFound->super;
        }

        return pFound;
    }

    addr_t AllocPages(unsigned long pages);
    void   MapRange(addr_t virt, addr_t phys, size_t bytes);

private:
    PAGE_ENTRY** GetPageList(unsigned long pages)
    {
        unsigned char listIndex     = 0;
        size_t        largestMatch  = PAGE_SIZE;

        // an acceptable superpage can fit this portion of the alloc
        size_t        allocSizePart = (pages * PAGE_SIZE) >> PAGE_SEG_DIVISOR;

        while (largestMatch < allocSizePart && largestMatch < MAX_SUPERPAGE_SIZE)
        {
            largestMatch = largestMatch << SUPERPAGE_SIZE_MULT;
            listIndex++;
        }

        return &m_SuperPages[listIndex];
    }

    addr_t CreateTable(void)
    {
        void *ptr = malloc(PAGE_TABLE_SIZE);
        zero(ptr, PAGE_TABLE_SIZE);

        addr_t virt = reinterpret_cast<addr_t>(ptr);
        addr_t phys = VirtualToPhysical(virt);

        m_MappingTable[phys] = virt;

        return phys;
    }

    addr_t Aligned(addr_t addr) const
    {
        return addr & ~(PAGE_ALIGNMENT - 1);
    }

    addr_t RemoveFlags(addr_t pageAddr) const
    {
        return pageAddr & (~0xfffull);
    }

    void Invalidate(PAGE_ENTRY &page)
    {
        addr_t pageAddr = GetPhysicalAddr(page);

        Invalidate(pageAddr);

        return;
    }

    void Invalidate(addr_t address)
    {
        asm volatile (
                "invlpg %0\n\t"
                :
                : "m" (address)
                : "memory"
        );

        /*
        asm volatile (
                "movl   %%cr3, %%eax\n\t"
                "movl   %%eax, %%cr3\n\t"
                :
                :
                : "%%eax"
        );
        */

        return;
    }

    void CreateSuperPage(PAGE_ENTRY &start, unsigned int pages);
    void CommitPage(PAGE_ENTRY &page);
    void MapPage(addr_t virt, addr_t phys, unsigned short flags);

    PAGE_ENTRY* m_Pages;
    PAGE_ENTRY* m_SuperPages[SUPERPAGE_LISTS];

    addr_t*     m_PDT;
    addr_t*     m_PT;

    // stores physical -> virtual mapping for malloc'ed page tables
    map<addr_t, addr_t, AddressHash> m_MappingTable;
};

#endif

