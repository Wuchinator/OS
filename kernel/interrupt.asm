[bits 32]
section .text




global idt_load
idt_load:
    mov eax, [esp + 4]
    lidt [eax]
    ret




extern isr_handler
extern irq_handler




%macro ISR_NOERR 1
global isr%1
isr%1:
    cli
    push 0
    push %1
    jmp isr_common
%endmacro

%macro ISR_ERR 1
global isr%1
isr%1:
    cli
    push %1
    jmp isr_common
%endmacro




ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20




%macro IRQ 2
global irq%1
irq%1:
    cli
    push 0
    push %2
    jmp irq_common
%endmacro




IRQ 0,  32
IRQ 1,  33
IRQ 2,  34
IRQ 3,  35
IRQ 4,  36
IRQ 5,  37
IRQ 6,  38
IRQ 7,  39
IRQ 8,  40
IRQ 9,  41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47




isr_common:
    pusha
    

    mov ax, ds
    push eax
    
    mov ax, es
    push eax
    
    mov ax, fs
    push eax
    
    mov ax, gs
    push eax
    

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push esp
    call isr_handler
    add esp, 4
    

    pop eax
    mov gs, ax
    
    pop eax
    mov fs, ax
    
    pop eax
    mov es, ax
    
    pop eax
    mov ds, ax
    
    popa
    
    add esp, 8
    iret




irq_common:
    pusha
    

    mov ax, ds
    push eax
    
    mov ax, es
    push eax
    
    mov ax, fs
    push eax
    
    mov ax, gs
    push eax
    

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push esp
    call irq_handler
    add esp, 4
    


    mov eax, [esp + 48]
    cmp eax, 40
    jl .pic1_only
    mov al, 0x20
    out 0xA0, al
.pic1_only:
    mov al, 0x20
    out 0x20, al
    

    pop eax
    mov gs, ax
    
    pop eax
    mov fs, ax
    
    pop eax
    mov es, ax
    
    pop eax
    mov ds, ax
    
    popa
    
    add esp, 8
    iret
