
VMDK=/virtual/os/x64/MyOS\ x64-flat.vmdk

CFLAGS=-c -W -Wall -Wextra -m64 -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow -nostdlib -fno-builtin -ffreestanding -nostartfiles -nodefaultlibs -fno-exceptions -fno-rtti -fno-stack-protector
SOURCES=kern/kstart.c kern/kmain.cpp kern/cxxabi.cpp kern/utils.cpp kern/video.cpp kern/pagemap.cpp kern/memory.cpp kern/font.cpp kern/fontdef.cpp kern/interrupts.cpp kern/isr.asm

all: bin/stage1.bin bin/stage2.bin bin/kern.bin
	cat bin/stage1.bin bin/stage2.bin bin/kern.bin > bin/bootimg.bin
	dd if=bin/bootimg.bin bs=512 conv=notrunc of=$(VMDK)

bin/stage1.bin: ldr/stage1.asm
	nasm -o bin/stage1.bin ldr/stage1.asm

bin/stage2.bin: ldr/stage2.asm
	nasm -o bin/stage2.bin ldr/stage2.asm

bin/kern.bin: $(SOURCES) kern/link.ld
	nasm -f elf64 -o kern/obj/isr.o kern/isr.asm
	g++ $(CFLAGS) -o kern/obj/kstart.o kern/kstart.c
	g++ $(CFLAGS) -o kern/obj/kmain.o kern/kmain.cpp
	g++ $(CFLAGS) -o kern/obj/cxxabi.o kern/cxxabi.cpp
	g++ $(CFLAGS) -o kern/obj/utils.o kern/utils.cpp
	g++ $(CFLAGS) -o kern/obj/video.o kern/video.cpp
	g++ $(CFLAGS) -o kern/obj/pagemap.o kern/pagemap.cpp
	g++ $(CFLAGS) -o kern/obj/memory.o kern/memory.cpp
	g++ $(CFLAGS) -o kern/obj/font.o kern/font.cpp
	g++ $(CFLAGS) -o kern/obj/fontdef.o kern/fontdef.cpp
	g++ $(CFLAGS) -o kern/obj/interrupts.o kern/interrupts.cpp
	ld -nostdlib -nodefaultlibs -T kern/link.ld -o bin/kern.bin kern/obj/kstart.o kern/obj/kmain.o kern/obj/cxxabi.o kern/obj/utils.o kern/obj/video.o kern/obj/pagemap.o kern/obj/memory.o kern/obj/font.o kern/obj/fontdef.o kern/obj/interrupts.o kern/obj/isr.o

