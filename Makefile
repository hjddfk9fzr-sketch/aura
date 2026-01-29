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