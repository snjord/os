
#include "inc/cxxabi.h"

extern "C"
{
    void *__dso_handle = NULL;

    struct atexit_entry_s  __atexit_entries[ATEXIT_ENTRIES_MAX];
    unsigned short         __atexit_curr = 0;
    
    // "Missing method" dispatch for pure virtual functions. Mandated by the C++
    // standard but should never actually be called
    void
    __cxa_pure_virtual(void)
    {
        return;
    }
    
    int
    __cxa_atexit(void (*func)(void *), void *arg, void *dso_handle)
    {
        if (__atexit_curr == ATEXIT_ENTRIES_MAX)
        {
            // non-zero means failure
            return -1;
        }

        __atexit_entries[__atexit_curr].func       = func;
        __atexit_entries[__atexit_curr].arg        = arg;
        __atexit_entries[__atexit_curr].dso_handle = dso_handle;
        __atexit_entries[__atexit_curr].complete   = false;

        __atexit_curr++;

        return 0;
    }
    
    void
    __cxa_finalize(void *d)
    {
        for (int x = 0; x < __atexit_curr; x++)
        {
            if ((d == NULL || __atexit_entries[x].dso_handle == d) &&
                !__atexit_entries[x].complete)
            {
                __atexit_entries[x].func(__atexit_entries[x].arg);
                __atexit_entries[x].complete = true;
            }
        }

        return;
    }
};

