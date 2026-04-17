ENTRY(kernel_start)

SECTIONS {
    . = 0x10000;

    .text : {
        *kernel_entry.o(.text)
        *(.text)
    }

    .rodata : { *(.rodata) }
    .data   : { *(.data)   }
    .bss    : {
        __bss_start = .;
        *(.bss)
        *(COMMON)
        __bss_end = .;
    }

    
    kernel_end = ALIGN(4096);
}
