CC = aarch64-elf-gcc
LD = aarch64-elf-ld

CFLAGS = -ffreestanding -nostdlib -O2 -Wall -Wextra
LDFLAGS = -T linker.ld

OBJS = entry.o kernel.o fb.o

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
	@echo "Build boot.efi with gnu-efi or EDK2 (see README.md)"

image.img:
	@echo "Create FAT image and copy EFI bootloader + kernel (see README.md)"

run: kernel.elf
	@if [ -z "$(QEMU_EFI)" ]; then echo "Set QEMU_EFI to your AArch64 UEFI firmware (AAVMF or QEMU_EFI.fd)"; exit 1; fi
	@if [ -z "$(IMAGE)" ]; then echo "Set IMAGE to your FAT image (image.img)"; exit 1; fi
	qemu-system-aarch64 -M virt -cpu cortex-a57 -m 1024 -nographic -bios $(QEMU_EFI) -drive file=$(IMAGE),format=raw,if=none,id=hd0 -device virtio-blk-device,drive=hd0

boot.efi:
	@echo "Build boot.efi with gnu-efi or EDK2 (see README.md)"

image.img:
	@echo "Create FAT image and copy EFI bootloader + kernel (see README.md)"

run: kernel.elf
	@if [ -z "$(QEMU_EFI)" ]; then echo "Set QEMU_EFI to your AArch64 UEFI firmware (AAVMF or QEMU_EFI.fd)"; exit 1; fi
	@if [ -z "$(IMAGE)" ]; then echo "Set IMAGE to your FAT image (image.img)"; exit 1; fi
	qemu-system-aarch64 -M virt -cpu cortex-a57 -m 1024 -nographic -bios $(QEMU_EFI) -drive file=$(IMAGE),format=raw,if=none,id=hd0 -device virtio-blk-device,drive=hd0