# Compiler & linker
ASM           = nasm
LIN           = ld
CC            = gcc

# Directory
SOURCE_FOLDER = src
OUTPUT_FOLDER = bin
ISO_NAME      = OS2024
CHAPTER_0     = ch0
CHAPTER_1     = ch1
CHAPTER_2     = ch2
CHAPTER_3     = ch3

# Flags
WARNING_CFLAG = -Wall -Wextra -Werror
DEBUG_CFLAG   = -fshort-wchar -g
STRIP_CFLAG   = -nostdlib -fno-stack-protector -nostartfiles -nodefaultlibs -ffreestanding
CFLAGS        = $(DEBUG_CFLAG) $(WARNING_CFLAG) $(STRIP_CFLAG) -m32 -c
AFLAGS        = -f elf32 -g -F dwarf
LFLAGS        = -T $(SOURCE_FOLDER)/linker.ld -melf_i386

# Disk
DISK_NAME     = storage

run: all
	@# chapter 1
	@qemu-system-i386 -s -drive file=$(OUTPUT_FOLDER)/sample-image.bin,format=raw,if=ide,index=0,media=disk -cdrom $(OUTPUT_FOLDER)/$(ISO_NAME).iso

all: build

build: iso

iso: kernel
	@mkdir -p $(OUTPUT_FOLDER)/iso/boot/grub
	@cp $(OUTPUT_FOLDER)/kernel     $(OUTPUT_FOLDER)/iso/boot/
	@cp other/grub1                 $(OUTPUT_FOLDER)/iso/boot/grub/
	@cp $(SOURCE_FOLDER)/menu.lst   $(OUTPUT_FOLDER)/iso/boot/grub/

	@# chapter 0
	@genisoimage -R                         \
	    -b boot/grub/grub1                  \
	    -no-emul-boot                       \
	    -boot-load-size 4                   \
	    -A os                               \
	    -input-charset utf8                 \
		-quiet                              \
		-boot-info-table                    \
		-o $(OUTPUT_FOLDER)/$(ISO_NAME).iso \
		$(OUTPUT_FOLDER)/iso

	@rm -r $(OUTPUT_FOLDER)/iso/

kernel:
	@# chapter 0
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/$(CHAPTER_0)/gdt/gdt.c -o $(OUTPUT_FOLDER)/gdt.o
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/$(CHAPTER_0)/kernel-entrypoint/kernel-entrypoint.s -o $(OUTPUT_FOLDER)/kernel-entrypoint.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/$(CHAPTER_0)/stdlib/string.c -o $(OUTPUT_FOLDER)/string.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/kernel.c -o $(OUTPUT_FOLDER)/kernel.o

	@# chapter 1
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/$(CHAPTER_1)/disk/disk.c -o $(OUTPUT_FOLDER)/disk.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/$(CHAPTER_1)/fat32/fat32.c -o $(OUTPUT_FOLDER)/fat32.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/$(CHAPTER_1)/framebuffer/framebuffer.c -o $(OUTPUT_FOLDER)/framebuffer.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/$(CHAPTER_1)/idt/idt.c -o $(OUTPUT_FOLDER)/idt.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/$(CHAPTER_1)/interrupt/interrupt.c -o $(OUTPUT_FOLDER)/interrupt.o
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/$(CHAPTER_1)/intsetup/intsetup.s -o $(OUTPUT_FOLDER)/intsetup.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/$(CHAPTER_1)/keyboard/keyboard.c -o $(OUTPUT_FOLDER)/keyboard.o
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/$(CHAPTER_1)/portio/portio.c -o $(OUTPUT_FOLDER)/portio.o

	@# chapter 2
	@$(CC) $(CFLAGS) $(SOURCE_FOLDER)/$(CHAPTER_2)/paging/paging.c -o $(OUTPUT_FOLDER)/paging.o

	@$(LIN) $(LFLAGS) $(OUTPUT_FOLDER)/*.o -o $(OUTPUT_FOLDER)/kernel
	@echo Linking object files and generate elf32...
	@rm -f $(OUTPUT_FOLDER)/*.o

clean:
	@rm -f \
		$(OUTPUT_FOLDER)/*.o \
		$(OUTPUT_FOLDER)/*.iso \
		$(OUTPUT_FOLDER)/kernel \
		$(OUTPUT_FOLDER)/inserter \
		$(OUTPUT_FOLDER)/shell

disk:
	@qemu-img create -f raw $(OUTPUT_FOLDER)/$(DISK_NAME).bin 4M

insert-shell: inserter user-shell
	@echo Inserting shell into root directory...
	@cd $(OUTPUT_FOLDER); ./inserter shell 2 $(DISK_NAME).bin

inserter:
	@$(CC) -Wno-builtin-declaration-mismatch -g -I$(SOURCE_FOLDER) \
		$(SOURCE_FOLDER)/$(CHAPTER_0)/stdlib/string.c \
		$(SOURCE_FOLDER)/$(CHAPTER_1)/fat32/fat32.c \
		$(SOURCE_FOLDER)/$(CHAPTER_2)/external-inserter/external-inserter.c \
		-o $(OUTPUT_FOLDER)/inserter

user-shell:
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/$(CHAPTER_2)/crt0/crt0.s -o crt0.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/$(CHAPTER_2)/user-shell/user-shell.c -o user-shell.o
	@$(CC)  $(CFLAGS) -fno-pie $(SOURCE_FOLDER)/$(CHAPTER_0)/stdlib/string.c -o string.o
	@$(LIN) -T $(SOURCE_FOLDER)/user-linker.ld -melf_i386 --oformat=binary \
		crt0.o user-shell.o string.o -o $(OUTPUT_FOLDER)/shell
	@echo Linking object shell object files and generate flat binary...
	@$(LIN) -T $(SOURCE_FOLDER)/user-linker.ld -melf_i386 --oformat=elf32-i386 \
		crt0.o user-shell.o string.o -o $(OUTPUT_FOLDER)/shell_elf
	@echo Linking object shell object files and generate ELF32 for debugging...
	@size --target=binary $(OUTPUT_FOLDER)/shell
	@rm -f *.o
