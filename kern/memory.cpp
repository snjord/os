
#include "inc/memory.h"

Memory heap((*reinterpret_cast<unsigned int *>(TOTAL_MEMORY_ADDR)) * static_cast<size_t>(64 * 1024));

void *
malloc(size_t bytes)
{
    return aligned_malloc(bytes, 4);
}

void *
aligned_malloc(size_t bytes, unsigned int alignment)
{
    return heap.Alloc(bytes, alignment);
}

void
mmap(addr_t virt, addr_t phys, size_t bytes)
{
    heap.Map(virt, phys, bytes);

    return;
}

void *
operator new(long unsigned int bytes)
{
    return malloc(bytes);
}

void *
operator new[](long unsigned int bytes)
{
    return malloc(bytes);
}

void
operator delete(void *ptr)
{
    UNREFERENCED(ptr);

    return;
}

Memory::Memory(size_t totalMemBytes)
    : m_TotalBytes(totalMemBytes),
      m_Mapper(totalMemBytes - UNMANAGED_BYTES)
{
}

Memory::~Memory(void)
{
}

void *
Memory::Alloc(size_t bytes, unsigned int alignment)
{
    // pages already provide 4kb alignment
    if (alignment > 4096)
    {
        bytes += alignment - 4096;
    }

    unsigned long pages = bytes / PAGE_SIZE;

    if (bytes % PAGE_SIZE != 0)
    {
        pages++;
    }

    addr_t pageAlloc = m_Mapper.AllocPages(pages);
    addr_t offset    =
        (alignment > 4096)
            ? ((pageAlloc & ~(alignment - 1)) + alignment) - pageAlloc
            : 0;

    void *alloc = reinterpret_cast<void *>(pageAlloc + offset);

    return alloc;
}

