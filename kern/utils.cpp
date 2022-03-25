
#include "inc/defs.h"

void
zero(void *addr, size_t bytes)
{
    unsigned char *curr = reinterpret_cast<unsigned char *>(addr);
    unsigned char *end  = curr + bytes;

    while (curr != end)
    {
        *curr = 0;
        curr++;
    }

    return;
}

void
memcpy(void *dst, void *src, size_t bytes)
{
    char *cdst = reinterpret_cast<char *>(dst);
    char *csrc = reinterpret_cast<char *>(src);

    for (size_t x = 0; x < bytes; x++)
    {
        *cdst = *csrc;

        cdst++;
        csrc++;
    }

    return;
}

void
cpu_vendor(char **vendor)
{
    *vendor = reinterpret_cast<char *>(malloc(13)); // 12 bytes + null
    
    char *curr = *vendor;

    asm volatile (
        "xor        %%rax, %%rax    \n\t"
        "cpuid                      \n\t"
        "movl       %%ebx, (%0)     \n\t"
        "addq       $4, %0          \n\t"
        "movl       %%edx, (%0)     \n\t"
        "addq       $4, %0          \n\t"
        "movl       %%ecx, (%0)     \n\t"
        "addq       $4, %0          \n\t"
        "movb       $0, (%0)        \n\t"
        :
        : "p" (curr)
        : "rax", "ebx", "edx", "ecx", "memory"
    );

    return;
}

void
cpu_name(char **name)
{
    char cpuidname[49]; // 16 chars across 3 parts + null
    zero(&cpuidname[0], sizeof(cpuidname));

    char *curr = &cpuidname[0];

    // cpuid with 8000000[2-4]h retrieves the processor name string in
    // three parts

    for (unsigned int x = 0x80000002; x <= 0x80000004; x++)
    {
        asm volatile (
            "xor        %%rax, %%rax    \n\t"
            "movl       %1, %%eax       \n\t"
            "cpuid                      \n\t"
            "movl       %%eax, (%0)     \n\t"
            "addq       $4, %0          \n\t"
            "movl       %%ebx, (%0)     \n\t"
            "addq       $4, %0          \n\t"
            "movl       %%ecx, (%0)     \n\t"
            "addq       $4, %0          \n\t"
            "movl       %%edx, (%0)     \n\t"
            "addq       $4, %0          \n\t"
            : "+p" (curr)
            : "r" (x)
            : "rax", "ebx", "ecx", "edx", "memory"
        );
    }

    // processor name string may be right justified with leading spaces

    curr = &cpuidname[0];
    unsigned char whitesp = 0;

    while (*curr == ' ')
    {
        whitesp++;
    }

    if (whitesp >= sizeof(cpuidname) - 1)
    {
        // empty name string
        *name = NULL;
    }
    else
    {
        unsigned int chars = sizeof(cpuidname) - whitesp;

        *name = reinterpret_cast<char *>(malloc(chars));
        zero(*name, chars);

        memcpy(*name, &cpuidname[0] + whitesp, chars);
    }

    return;
}

