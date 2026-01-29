CC = aarch64-elf-gcc
LD = aarch64-elf-ld

CFLAGS = -ffreestanding -nostdlib -O2 -Wall -Wextra
LDFLAGS = -T linker.ld

OBJS = entry.o kernel.o fb.o

.PHONY: all clean boot.efi image.img run

all: kernel.elf

entry.o:
	$(CC) $(CFLAGS) -c kernel/arch/arm64/entry.S -o entry.o

kernel.o:
	$(CC) $(CFLAGS) -c kernel/core/kernel.c -o kernel.o

fb.o:
	$(CC) $(CFLAGS) -c kernel/video/framebuffer.c -o fb.o

kernel.elf: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o kernel.elf

clean:
	rm -f *.o kernel.elf

boot.efi:
	@echo "Building BOOTAA64.EFI (scripts/build_boot.sh)"
	@bash scripts/build_boot.sh

image.img: boot.efi kernel.elf
	@echo "Creating FAT image (scripts/make_image.sh)"
	@bash scripts/make_image.sh

run: kernel.elf
	@if [ -z "$(QEMU_EFI)" ]; then echo "Set QEMU_EFI to your AArch64 UEFI firmware (AAVMF or QEMU_EFI.fd)"; exit 1; fi
	@if [ -z "$(IMAGE)" ]; then echo "Set IMAGE to your FAT image (image.img)"; exit 1; fi
	qemu-system-aarch64 -M virt -cpu cortex-a57 -m 1024 -nographic -bios $(QEMU_EFI) -drive file=$(IMAGE),format=raw,if=none,id=hd0 -device virtio-blk-device,drive=hd0