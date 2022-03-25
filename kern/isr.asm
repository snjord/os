
[BITS 64]

; interrupt descriptor table
;
; isr0  - divide by zero
; isr2  - NMI
; isr8  - double fault
; isr13 - general protection fault
; isr14 - page fault

; irq map
;
; irq0 (master pic) -> int 0x20
; irq8 (slave pic)  -> int 0x28
;
; fires on all master interrupts - irq4
; fires on all slave interrupts  - irq2

; pic ports
;
; pic1 (master) - command 0x20, data 0x21
; pic2 (slave)  - command 0xa0, data 0xa1
;

global io_wait
io_wait:
    push    ax

    mov     al, 0           ; writing to another port (0x80 is unused) ensures
    out     80h, al         ; any previous port i/o completed

    pop     ax
    ret

global remap_pic
remap_pic:
    push    ax

    mov     al, 0x11
    out     0x20, al        ; initialize pic1
    call    io_wait

    out     0xa0, al        ; initialize pic2
    call    io_wait

    mov     al, 0x20
    out     0x21, al        ; start pic1 at int 0x20
    call    io_wait

    mov     al, 0x28
    out     0xa1, al        ; start pic2 at int 0x28
    call    io_wait

    mov     al, 4
    out     0x21, al        ; any master int also fires irq4
    call    io_wait

    mov     al, 2
    out     0xa1, al        ; any slave int also fires irq2
    call    io_wait

    mov     al, 1
    out     0x21, al        ; use 8086 mode on pic1
    call    io_wait

    out     0xa1, al        ; use 8086 mode on pic2
    call    io_wait

    mov     al, 0x0
    out     0x21, al        ; enable irqs 0 - 7
    call    io_wait

    out     0xa1, al        ; enable irqs 8 - 15
    call    io_wait

    pop     ax
    ret

    ; acknowledge interrupt on pic1 (irqs 0 - 7)
ack_interrupt1:
    push    ax

    mov     al, 20h         ; end of interrupt code
    out     20h, al

    pop     ax
    ret

    ; acknowledge interrupt on pic2 (irqs 8 - 15)
ack_interrupt2:
    push    ax

    call    ack_interrupt1  ; slave needs to acknowledge both pics
    call    io_wait

    mov     al, 20h         ; end of interrupt code
    out     0xa0, al        ; ack on slave pic

    pop     ax
    ret

global isr0
isr0:
    iretq

global isr2
isr2:
    iretq

global isr8
isr8:
    iretq

global isr13
isr13:
    iretq

global isr14
isr14:
    iretq

    ; irq0
global isr32
isr32:
    call    ack_interrupt1

    iretq

global isr33
isr33:
    call    ack_interrupt1

    iretq

global isr34
isr34:
    call    ack_interrupt1

    iretq

global isr35
isr35:
    call    ack_interrupt1

    iretq

global isr36
isr36:
    call    ack_interrupt1

    iretq

global isr37
isr37:
    call    ack_interrupt1

    iretq

global isr38
isr38:
    call    ack_interrupt1

    iretq

global isr39
isr39:
    call    ack_interrupt1

    iretq

    ; irq 8
global isr40
isr40:
    call    ack_interrupt2

    iretq

global isr41
isr41:
    call    ack_interrupt2

    iretq

global isr42
isr42:
    call    ack_interrupt2

    iretq

global isr43
isr43:
    call    ack_interrupt2

    iretq

global isr44
isr44:
    call    ack_interrupt2

    iretq

global isr45
isr45:
    call    ack_interrupt2

    iretq

global isr46
isr46:
    call    ack_interrupt2

    iretq

global isr47
isr47:
    call    ack_interrupt2

    iretq

global isr_default
isr_default:
    iretq

