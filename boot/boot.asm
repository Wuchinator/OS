[org 0x7C00]
[bits 16]

KERNEL_LOAD_SEGMENT equ 0x1000
KERNEL_LOAD_OFFSET  equ 0x0000
KERNEL_ENTRY_LINEAR equ 0x10000


KERNEL_SECTORS      equ 200
KERNEL_START_LBA    equ 1

start:
    cld
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00


    mov [boot_drive], dl


    call check_a20
    cmp ax, 1
    je .a20_done
    
    mov ax, 0x2401
    int 0x15
    
.a20_done:
    call load_kernel
    call verify_kernel_signature


    cli
    mov al, 0xFF
    out 0x21, al
    out 0xA1, al
    lgdt [gdt_descriptor]

    mov eax, cr0
    or  eax, 0x1
    mov cr0, eax


    jmp 0x08:init_pm




check_a20:

    push ds
    push es
    push di
    push si

    xor ax, ax
    mov ds, ax
    mov di, 0x0500
    
    mov ax, 0xFFFF
    mov es, ax
    mov si, 0x0510

    mov al, [ds:di]
    push ax
    mov al, [es:si]
    push ax

    mov byte [ds:di], 0x00
    mov byte [es:si], 0xFF

    cmp byte [ds:di], 0xFF

    pop ax
    mov [es:si], al
    pop ax
    mov [ds:di], al

    mov ax, 0
    je .res
    mov ax, 1
.res:
    pop si
    pop di
    pop es
    pop ds
    ret

load_kernel:
    mov word [load_seg], KERNEL_LOAD_SEGMENT
    mov dword [cur_lba], KERNEL_START_LBA
    mov word [sectors_left], KERNEL_SECTORS

.read_next:
    xor ax, ax
    mov ds, ax

    mov ax, [load_seg]
    mov [dap_segment], ax
    mov word [dap_offset], KERNEL_LOAD_OFFSET

    mov eax, [cur_lba]
    mov [dap_lba_low], eax
    mov dword [dap_lba_high], 0

    mov si, disk_address_packet
    mov ah, 0x42
    mov dl, [boot_drive]
    int 0x13
    jc load_error

    add word [load_seg], 0x20
    inc dword [cur_lba]
    dec word [sectors_left]
    jnz .read_next

.done:
    ret

verify_kernel_signature:
    push ds
    mov ax, KERNEL_LOAD_SEGMENT
    mov ds, ax
    cmp byte [0], 0xBF
    jne .bad
    cmp byte [5], 0xB9
    jne .bad
    pop ds
    ret

.bad:
    pop ds
    mov si, msg_kernel_sig_error
    call print_string_rm
    jmp $

load_error:
    mov si, msg_disk_error
    call print_string_rm
    jmp $

print_string_rm:
    mov ah, 0x0E
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    ret

boot_drive      db 0
msg_disk_error  db "Disk Error!", 0
msg_kernel_sig_error db "Kernel Sig Error!", 0
load_seg        dw 0
sectors_left    dw 0
cur_lba         dd 0

disk_address_packet:
    db 0x10
    db 0x00
    dw 0x0001
dap_offset:
    dw KERNEL_LOAD_OFFSET
dap_segment:
    dw KERNEL_LOAD_SEGMENT
dap_lba_low:
    dd 0x00000000
dap_lba_high:
    dd 0x00000000

%include "boot/gdt.asm"


[bits 32]
init_pm:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax


    mov esp, 0x90000


    jmp 0x08:KERNEL_ENTRY_LINEAR

times 510-($-$$) db 0
dw 0xAA55
