[bits 32]
section .text
[global kernel_start]

kernel_start:
    extern __bss_start
    extern __bss_end
    extern kernel_main


    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, edi
    xor eax, eax
    cld
    rep stosb

    call kernel_main
    jmp $
