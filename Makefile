CC      := $(shell if command -v i686-elf-gcc >/dev/null 2>&1; then echo i686-elf-gcc; else echo gcc; fi)
AS      := nasm
LD      := $(shell if command -v i686-elf-ld >/dev/null 2>&1; then echo i686-elf-ld; else echo ld; fi)

CFLAGS  = -ffreestanding -m32 -fno-pic -fno-stack-protector -fno-tree-vectorize -mno-mmx -mno-sse -mno-sse2 -nostdlib -O2 -Wall -Wextra
LDFLAGS = -m elf_i386 --oformat binary

C_SOURCES = $(wildcard kernel/*.c drivers/*.c clib/src/*.c)
HEADERS   = $(wildcard kernel/*.h drivers/*.h clib/include/*.h)
OBJ       = ${C_SOURCES:.c=.o} kernel/interrupt.o

CFLAGS += -Iclib/include

all: os.img

os.img: boot/boot.bin kernel.bin
	cat $^ > os.img
	truncate -s 1440K os.img

kernel.bin: kernel/kernel_entry.o $(OBJ) kernel/linker.ld
	$(LD) -o $@ -T kernel/linker.ld kernel/kernel_entry.o $(OBJ) $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

kernel/kernel_entry.o: kernel/kernel_entry.asm
	$(AS) -f elf32 $< -o $@

kernel/interrupt.o: kernel/interrupt.asm
	$(AS) -f elf32 $< -o $@

boot/boot.bin: boot/boot.asm
	$(AS) -f bin $< -o $@

run: os.img
	qemu-system-i386 -drive format=raw,file=os.img

run-serial: os.img
	qemu-system-i386 -drive format=raw,file=os.img -serial stdio -display none -monitor none

run-headless: os.img
	qemu-system-i386 -drive format=raw,file=os.img -serial stdio -display none -monitor none

clean:
	rm -f kernel/*.o drivers/*.o clib/src/*.o boot/boot.bin kernel.bin os.img
