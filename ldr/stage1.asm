
[BITS 16]
[ORG 0x7c00]

; Memory layout: First 1MB
;
; 0x000000 - 0x0003ff       real mode ivt
; 0x000400 - 0x0004ff       bios data area
; 0x000500 - 0x000fff       free
; 0x001000 - 0x006fff       kernel load area
; 0x007000 - 0x007bff       free
; 0x007c00 - 0x007dff       boot sector (stage 1)
; 0x007e00 - 0x0085ff       stage 2
; 0x008600 - 0x08ffff       free
;     except 0x080000       total memory in 64kb blocks
; 0x090000 - 0x09ffff       stage 2 stack area
; 0x0a0000 - 0x0bffff       video memory
; 0x0c0000 - 0x0c7fff       video bios
; 0x0c8000 - 0x0effff       bios shadow area
; 0x0f0000 - 0x0fffff       system bios

; High memory: >1MB
;
; 0x100000  - 0x1fffff      stage2 mapped page tables
; 0x200000  - 0x3fffff      kernel
; 0x400000  - 0x4001ff      vesa info block
; 0x400200  - 0x4002ff      vesa mode info
; 0x500000  - 0xffffff      heap page info blocks
; 0x1000000 - 0x1000fff     heap pdt
; 0x1001000 - 0x1200fff     heap page tables
; 0x1201000 - 0x17fffff     free
; 0x1800000 - 0x1ffffff     kernel stack area

; 0x0000000 - 0x1ffffff     identity mapped memory
; 0x2000000 -               heap managed memory


start:
        cli

        xor     ax, ax                  ; initialize segment registers
        mov     ds, ax
        mov     ss, ax
        mov     es, ax

        mov     bp, 7c00h               ; initialize frame to boot entry point
        mov     sp, 7c00h

        cld                             ; forward direction string ops

        push    dx                      ; save boot disk id in dl

        in      al, 92h                 ; fast enable a20 gate
        or      al, 2
        out     92h, al

        lgdt    [gdtr32]                ; load gdt

        mov     eax, cr0                ; enter protected mode
        or      eax, 1
        mov     cr0, eax

        mov     bx, 10h                 ; use data selector for segments
        mov     ds, bx
        mov     ss, bx
        mov     es, bx

        mov     eax, cr0                ; enter unreal mode
        and     al, 0feh
        mov     cr0, eax

        mov     bx, 0                   ; reset segment values, cached values
        mov     ds, bx                  ; from gdt still used
        mov     ss, bx
        mov     es, bx

        sti

        xor     eax, eax

        mov     ax, 0xe801              ; get memory size function
        int     15h

        mov     eax, 256                ; assume at least 16MB (16MB / 64KB = 256)
        add     eax, ebx                ; memory beyond 16MB

        mov     ebx, 80000h
        mov     dword [ebx], eax        ; store total memory blocks to 0x80000

        mov     dx, word [bp-2]         ; copy boot disk id into dl

        mov     ah, 2                   ; read sectors function
        mov     al, 2                   ; read 2 sectors, 512 * 2 = 1KB of data
        mov     ch, 0                   ; cylinder 0
        mov     dh, 0                   ; head 0
        mov     cl, 2                   ; sector 2, past this boot sector
        mov     bx, 7e00h               ; read destination
        int     13h                     ; initiate read of the stage 2 loader

        mov     ah, 2                   ; read sectors function
        mov     al, 1                   ; read 1 sector, 512 bytes
        mov     ch, 0                   ; cylinder 0
        mov     dh, 0                   ; head 0
        mov     cl, 4                   ; sector 4, past stage 2
        mov     bx, 1000h               ; read destination
        int     13h                     ; initiate read of kernel

        xor     edx, edx
        mov     eax, dword [1000h]      ; kernel byte size
        mov     bx, 512
        div     bx

        cmp     dx, 0
        je      start.no_remainder

        inc     ax                      ; sector for the remainder

.no_remainder:
        pop     dx                      ; restore boot disk id to dl

        mov     ah, 2                   ; read sectors function
        mov     ch, 0                   ; cylinder 0
        mov     dh, 0                   ; head 0
        mov     cl, 4                   ; sector 4, past stage 2
        mov     bx, 1000h               ; read destination
        int     13h                     ; initiate read of kernel

        mov     edi, 200000h            ; destination addr
        mov     esi, 1004h              ; source addr, past kernel size
        mov     ecx, dword [1000h]      ; kernel byte size

        db      0x67                    ; modify opcode for 32-bit addressing
        rep     movsb                   ; copy kernel to higher memory

        jmp     00h:7e00h               ; enter stage 2


bios_print_char:
        push    bp
        mov     bp, sp

        mov     al, [bp+4]              ; char to output
        mov     ah, 0eh                 ; teletype mode
        mov     bh, 00h                 ; black background
        mov     bl, 07h                 ; light grey foreground

        int     10h

        leave
        ret

bios_print_string:
        push    bp
        mov     bp, sp

        mov     bx, [bp+4]              ; get addr of the first char
.str_loop:
        xor     ax, ax
        mov     al, [bx]                ; get the char from memory

        cmp     al, 0                   ; check for end of string
        je      bios_print_string.str_loop_done

        push    bx                      ; save bx

        push    ax                      ; char param
        call    bios_print_char
        add     sp, 2

        pop     bx                      ; restore bx

        inc     bx                      ; next char address
        jmp     bios_print_string.str_loop

.str_loop_done:
        leave
        ret


str_boot:   db "Started...",`\r\n`,0

gdt32:      dq 0x00                                           ; null entry
            db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xcf, 0x00 ; code selector
            db 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xcf, 0x00 ; data selector

            dw 0x68, tss0                                     ; tss entry
            db 0x00, 0x89, 0x40, 0x00
gdt32_end:

gdtr32:     dw gdt32_end - gdt32 - 1                          ; gdt length
            dd gdt32                                          ; gdt addr

tss0:       ; tss entry for cpu0
            dd 0x00     ; not using
            dd 0xb000   ; esp0: the syscall stack. growing down from 0xb000
            dw 0x10     ; ss0: third gdt entry for kernel data segment
            dw 0x00     ; not using this, and the next 88 bytes
            dd 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
            dd 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
            dw 0x68     ; size of this tss (104 bytes)
            dw 0x00     ; unused


TIMES   510 - ($ - $$) db 0             ; Write 0's to the rest of the sector
DW      0xAA55                          ; Write boot signature to the end

