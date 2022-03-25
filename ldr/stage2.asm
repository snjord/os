
[BITS 16]
[ORG 0x7e00]

; Kernel page mapping
;
; 0x100000 - 0x100fff               PML4T
;   0x101000 - 0x101fff             low 512GB PDPT
;       0x102000 - 0x102fff         low 1GB PDT
;           0x103000 - 0x103fff     0MB - 2MB PT
;           .
;           .
;           .
;           0x10a000 - 0x10afff     14MB - 16MB PT
;           0x112000 - 0x112fff     30MB - 32MB PT


stage2:
        sub     sp, 512                 ; make room for vesa info block
        mov     di, sp                  ; destination
        mov     dword [di], 'VBE2'      ; request VBE 3.0
        mov     ax, 4f00h               ; get vesa controller data
        int     10h

        mov     bx,  sp                 ; store vesa block addr in ebx
        and     ebx, 0xffff

        mov     edi, 400000h            ; destination addr
        mov     esi, ebx                ; source addr
        mov     ecx, 512                ; vesa block size fixed at 512 bytes

        db      0x67                    ; modify opcode for 32-bit addressing
        rep     movsb                   ; cache vesa block into high memory

        add     sp, 512                 ; remove vesa block from stack

        mov     ebx, 400000h
        mov     ebx, dword [ebx]
        mov     eax, 'VESA'
        cmp     ebx, eax
        je      stage2.with_vesa

        cli
        hlt

.with_vesa:
        call    bios_set_vmode          ; prepare empty screen, 800x600

        cli                             ; no more interrupts until in kernel

        mov     eax, cr0                ; enter protected mode
        or      eax, 1
        mov     cr0, eax

        jmp     08h:stage2.flush_gdt32  ; use a far jump to refresh cs

[BITS 32]
.flush_gdt32:
        mov     ax, 10h                 ; use data selector for the rest
        mov     ds, ax
        mov     ss, ax
        mov     es, ax
        mov     fs, ax
        mov     gs, ax

        mov     ax, 18h                 ; tss index in gdt
        ltr     ax                      ; setup task register

        mov     esp, 9ffe0h             ; set the stack pointer to a safe area

        mov     edi, 100000h            ; starting address of kernel PML4T
        mov     cr3, edi                ; setup cr3 for the kernel's page mapping

        xor     eax, eax
        mov     ecx, 13000h             ; zero out (PML4T + PDPT + PDT + PT)
        rep     stosd

        mov     edi, cr3
        mov     dword [edi], 101003h    ; set PML4T[0] to PDPT, present + rw

        mov     edi, 101000h
        mov     dword [edi], 102003h    ; set PDPT[0] to PDT, present + rw

        mov     edi, 102000h            ; starting pdt addr
        mov     edx, 103000h            ; starting pt addr
        mov     ebx, 0h                 ; starting physical addr

        mov     ecx, 16                 ; for 16 pdt entries (32mB addressable)

.setup_pdt:
        mov     eax, edx
        or      eax, 3h                 ; mark present + rw
        mov     dword [edi], eax        ; set pdt entry to the current pt addr

        push    ebx                     ; physical addr
        push    edx                     ; pt addr
        call    identity_map_pt         ; identity map this pt (2mB)
        add     esp, 8

        add     edi, 8                  ; move to next pdt entry, each is 8 bytes
        add     edx, 1000h              ; move to next pt, 4kB per pt
        add     ebx, 200000h            ; move to next physical addr, 2mB per pt
        loop    stage2.setup_pdt

        mov     eax, cr4
        or      eax, 1 << 5             ; set bit 6 to one
        mov     cr4, eax                ; enable pae paging

        mov     ecx, 0xc0000080         ; efer msr
        rdmsr                           ; read msr into eax
        or      eax, 1 << 8             ; enable long mode (bit 9)
        wrmsr

        mov     eax, cr0
        or      eax, 1 << 31            ; enable paging (bit 32)
        mov     cr0, eax

        lgdt    [gdtr64]                ; load 64-bit gdt

        jmp     08h:stage2.flush_gdt64  ; refresh cs, leave compatibility mode

[BITS 64]
.flush_gdt64:
        mov     ax, 10h                 ; set data selector for all segments
        mov     ds, ax
        mov     es, ax
        mov     fs, ax
        mov     gs, ax
        mov     ss, ax

        ;mov     rsp, 2000000h           ; kernel stack starts at 32MB
        mov     rsp, 1ff0000h           ; not sure why the other one is busted

        mov     rax, 200000h            ; enter kernel
        jmp     rax


[BITS 32]
identity_map_pt:
        push    ebp
        mov     ebp, esp

        push    ebx
        push    ecx
        push    edi

        mov     edi, [ebp+8]            ; address of pt

        mov     ebx, [ebp+12]           ; starting physical addr
        add     ebx, 3h                 ; present + rw

        mov     ecx, 512                ; for all 512 entries (long mode)

.pte:
        mov     dword [edi], ebx        ; set pte to physical addr in ebx

        add     ebx, 1000h              ; move to next addr, 4kB paging
        add     edi, 8                  ; move to next pte, each is 8 bytes
        loop    identity_map_pt.pte

        pop     edi
        pop     ecx
        pop     ebx

        leave
        ret


[BITS 16]
bios_set_vmode:
        push    bp
        mov     bp, sp

        call    bios_find_vmode

        mov     bx, ax
        or      bx, 1 << 14             ; enable frame buffer
        mov     ax, 4f02h               ; set video mode function
        int     10h

        cmp     ax, 004fh               ; check for error
        je      bios_set_vmode.set

        cli
        hlt

.set:
        leave
        ret

bios_find_vmode:
        push    bp
        mov     bp, sp

        mov     edx, 00h                ; vmode table index

        mov     eax, 40000eh
        mov     eax, dword [eax]        ; vmode table base addr in seg:off form
        mov     ebx, eax
        and     ebx, 0xffff0000         ; extract segment
        shr     ebx, 12                 ; move segment into position
        and     eax, 0x0000ffff         ; leave only offset in eax
        add     ebx, eax                ; add in offset, now a linear address

.search_loop:
        mov     cx, word [ebx+edx]      ; retrieve video mode number
        cmp     cx, 0xffff              ; -1 marks end of vmode table
        je      bios_find_vmode.not_found

        push    cx
        push    word 800                ; search for 800x600 32 bpp
        push    word 600
        push    word 32
        call    bios_check_vmode
        add     sp, 8

        cmp     ax, 1
        je      bios_find_vmode.found

        add     dx, 2                   ; 2 bytes per mode entry
        jmp     bios_find_vmode.search_loop

.found:
        mov     ax, cx                  ; return the discovered mode number

        leave
        ret

.not_found:
        cli
        hlt

bios_check_vmode:
        push    bp
        mov     bp, sp

        push    ebx
        push    ecx
        push    edx
        push    edi
        push    esi

        sub     sp, 256                 ; make room for the mode info block
        mov     di, sp
        mov     cx, word [bp+0xa]       ; mode number
        mov     ax, 4f01h               ; get mode info function
        int     10h

        mov     dx, ax                  ; cache vesa return value
        mov     ax, 0                   ; default to bad video mode

        cmp     dx, 004fh
        jne     bios_check_vmode.bad

        mov     bx, sp
        mov     dx, word [bx]           ; retrieve mode attributes

        bt      dx, 0                   ; test if mode supported
        jnc     bios_check_vmode.bad

        bt      dx, 4                   ; test if a graphics mode
        jnc     bios_check_vmode.bad

        bt      dx, 7                   ; test for linear frame buffer
        jnc     bios_check_vmode.bad

        mov     dx, word [bx+12h]       ; mode horizontal res
        cmp     dx, word [bp+8h]        ; requested horizontal res
        jne     bios_check_vmode.bad

        mov     dx, word [bx+14h]       ; mode vertical res
        cmp     dx, word [bp+6h]        ; requested vertical res
        jne     bios_check_vmode.bad

        xor     dx, dx
        mov     dl, byte [bx+19h]       ; mode bpp
        cmp     dx, word [bp+4h]        ; requested bpp
        jne     bios_check_vmode.bad

        mov     ax, 1                   ; mode is good

        and     ebx, 0xffff             ; remove any high word from ebx
        mov     edi, 400200h            ; destination addr
        mov     esi, ebx                ; source addr
        mov     ecx, 256                ; mode block size fixed at 256 bytes

        db      0x67                    ; modify opcode for 32-bit addressing
        rep     movsb                   ; cache vesa block into high memory

.bad:
        add     sp, 256

        pop     esi
        pop     edi
        pop     edx
        pop     ecx
        pop     ebx

        leave
        ret


[BITS 32]

gdt64:      dq 0x00                                           ; null entry
            db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0x2f, 0x00 ; code selector
            db 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0x0f, 0x00 ; data selector
gdt64_end:

gdtr64:     dw gdt64_end - gdt64 - 1                          ; gdt length
            dq gdt64                                          ; gdt addr

str_foo:    db "Ending...",`\r\n`,0

TIMES   1024 - ($ - $$) db 0             ; Write 0's for the rest of stage 2

