
#include "inc/interrupts.h"
#include "inc/video.h"

IDTENTRY idttable[IDT_TABLE_ENTRIES];

void
initialize_idt(void)
{
    // exceptions
    idttable[0]  = IDTENTRY(isr0);  // divide by zero
    idttable[2]  = IDTENTRY(isr2);  // nmi
    idttable[8]  = IDTENTRY(isr8);  // double fault
    idttable[13] = IDTENTRY(isr13); // general protection fault
    idttable[14] = IDTENTRY(isr14); // page fault

    // irqs 0 - 7
    idttable[32] = IDTENTRY(isr32);
    idttable[33] = IDTENTRY(isr33);
    idttable[34] = IDTENTRY(isr34);
    idttable[35] = IDTENTRY(isr35);
    idttable[36] = IDTENTRY(isr36);
    idttable[37] = IDTENTRY(isr37);
    idttable[38] = IDTENTRY(isr38);
    idttable[39] = IDTENTRY(isr39);

    // irqs 8 - 15
    idttable[40] = IDTENTRY(isr40);
    idttable[41] = IDTENTRY(isr41);
    idttable[42] = IDTENTRY(isr42);
    idttable[43] = IDTENTRY(isr43);
    idttable[44] = IDTENTRY(isr44);
    idttable[45] = IDTENTRY(isr45);
    idttable[46] = IDTENTRY(isr46);
    idttable[47] = IDTENTRY(isr47);

    // reinitialize pic with above irq -> idt map
    remap_pic();

    IDTR idtr;
    idtr.tablebytes = sizeof(IDTENTRY) * IDT_TABLE_ENTRIES;
    idtr.tableaddr  = reinterpret_cast<addr_t>(&idttable[0]);

    asm volatile (
        "lidtq   %0\n\t"
        :
        : "m" (idtr) // input
        : "memory"
    );

    return;
}

void
enable_interrupts(void)
{
    asm volatile (
        "sti"
        : : : "memory"
    );

    return;
}

void
disable_interrupts(void)
{
    asm volatile (
        "cli"
        : : : "memory"
    );

    return;
}

void
report_interrupt(unsigned char id)
{
    UNREFERENCED(id);

    return;
}

