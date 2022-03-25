
#ifndef MEMORY_H
#define MEMORY_H

#include "defs.h"
#include "pagemap.h"

// ptr to a dword of total memory as 64kb blocks
#define TOTAL_MEMORY_ADDR   0x80000

#define UNMANAGED_BYTES     0x2000000

void* operator new(long unsigned int bytes);
void* operator new[](long unsigned int bytes);

void operator delete(void *ptr);

class Memory
{
public:
    Memory(size_t totalMemBytes);
    virtual ~Memory(void);

    void Map(addr_t virt, addr_t phys, size_t bytes)
    {
        m_Mapper.MapRange(virt, phys, bytes);

        return;
    }

    unsigned long long GetTotalMemory(void)
    {
        return m_TotalBytes;
    }

    void* Alloc(size_t bytes, unsigned int alignment);

private:
    unsigned long long m_TotalBytes;

    PageMap m_Mapper;
};

#endif

