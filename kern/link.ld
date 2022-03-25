
OUTPUT_FORMAT(binary)
ENTRY(kstart)

SECTIONS
{
    . = 0x1ffffc;

    .ksig :
    {
        /*
        LONG(SIZEOF(.text) + SIZEOF(.rodata) + SIZEOF(.data) + SIZEOF(.bss))
        */

        LONG(image_end - image_begin)
    }

    image_begin = .;

    .text :
    {
        *(.text*)
        *(.gnu.linkonce.t*)

        . = ALIGN(4096);
    }

    .rodata :
    {
        *(.rodata*)
        *(.gnu.linkonce.r*)

        start_ctors = .;
        *(.ctor*)
        end_ctors = .;

        start_dtors = .;
        *(.dtor*)
        end_dtors = .;

        . = ALIGN(4096);
    }

    .data :
    {
        *(.data*)
        *(gnu.linkonce.d*)

        image_end = .;

        . = ALIGN(4096);
    }

    .bss :
    {
        *(.COMMON*)
        *(.bss*)
        *(.gnu.linkonce.b*)
    }

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)    /* Remove when adding support for C++ exceptions */
    }
}

