
#include "inc/defs.h"
#include "inc/cxxabi.h"

extern unsigned long long start_ctors;
extern unsigned long long end_ctors;

extern unsigned long long start_dtors;
extern unsigned long long end_dtors;

extern "C" void kmain(void);

extern "C" void
kstart(void)
{
    zero(&__atexit_entries[0], sizeof(struct atexit_entry_s) * ATEXIT_ENTRIES_MAX);

    for (unsigned long long *cons = &start_ctors;
         cons < &end_ctors;
         cons++)
    {
        ((void (*)(void)) *cons)();
    }

    kmain();

    while (1)
    {
        asm volatile ("hlt");
    }

    for (unsigned long long *dtor = &start_dtors;
         dtor < &end_dtors;
         dtor++)
    {
        ((void (*)(void)) *dtor)();
    }

    return;
}

