
#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "defs.h"

#define IDT_TABLE_ENTRIES   256

#define IDT_INTATTR         0x8e
#define IDT_TRAPATTR        0x8f

typedef void (*isrentry_t)(void);

extern "C"
{
    void isr0(void);
    void isr2(void);
    void isr8(void);
    void isr13(void);
    void isr14(void);

    void isr32(void);
    void isr33(void);
    void isr34(void);
    void isr35(void);
    void isr36(void);
    void isr37(void);
    void isr38(void);
    void isr39(void);

    void isr40(void);
    void isr41(void);
    void isr42(void);
    void isr43(void);
    void isr44(void);
    void isr45(void);
    void isr46(void);
    void isr47(void);

    void isr_default(void);

    void remap_pic(void);
    void io_wait(void);
}

typedef struct idtentry_s
{
    idtentry_s(isrentry_t handler = isr_default, bool allowInterrupts = false)
        : baselow(reinterpret_cast<addr_t>(handler) & 0xffff),
          cs(CS_INDEX),
          basemid((reinterpret_cast<addr_t>(handler) & 0xffff0000) >> 16),
          basehigh((reinterpret_cast<addr_t>(handler) & 0xffffffff00000000) >> 32)
    {
        attr = (allowInterrupts) ? IDT_TRAPATTR
                                 : IDT_INTATTR;

        zero(&reserved, sizeof(reserved));
        zero(&reserved2, sizeof(reserved2));
    }

    unsigned short baselow;     // low 16 bits of the base addr
    unsigned short cs;          // index to the code selector
    unsigned char  reserved;    // must be 0
    unsigned char  attr;        // type and attributes
    unsigned short basemid;     // bits 16-31 of base addr
    unsigned int   basehigh;    // bits 32-63 of base addr
    unsigned int   reserved2;   // must be 0

} __attribute__((__packed__)) IDTENTRY;

extern IDTENTRY idttable[IDT_TABLE_ENTRIES];

typedef struct idtr_s
{
    unsigned short  tablebytes;
    addr_t          tableaddr;
} __attribute__((__packed__)) IDTR;

void initialize_idt(void);
void enable_interrupts(void);
void disable_interrupts(void);
void report_interrupt(unsigned char id);

#endif

