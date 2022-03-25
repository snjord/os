
#include "defs.h"

#define ATEXIT_ENTRIES_MAX  128

extern "C"
{
    struct atexit_entry_s
    {
        void (*func)(void *);
        void *arg;
        void *dso_handle;

        bool complete;
    };
    
    extern struct atexit_entry_s __atexit_entries[];
    extern unsigned short        __atexit_curr;

    void __cxa_pure_virtual(void);
    
    int  __cxa_atexit(void (*func)(void *), void *arg, void *dso_handle);
    void __cxa_finalize(void *d);
};

