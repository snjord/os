
#include "inc/interrupts.h"
#include "inc/video.h"

extern "C" void
kmain(void)
{
    Video vid;

    initialize_idt();
    enable_interrupts();

    return;
}

