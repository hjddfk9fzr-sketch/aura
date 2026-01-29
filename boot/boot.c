#include <efi.h>
#include <efilib.h>
#include "../kernel/core/bootinfo.h"

// Minimal ELF64 headers used by the loader
typedef struct {
    unsigned char e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} Elf64_Phdr;

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    EFI_STATUS Status;

    ST->ConOut->ClearScreen(ST->ConOut);
    Print(L"AURA OS Bootloader\n");
    Print(L"Loading microkernel...\n");

    // загрузка ELF и передача framebuffer info

    // Locate Graphics Output Protocol
    EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
    Status = uefi_call_wrapper(BS->LocateProtocol, 3, &gEfiGraphicsOutputProtocolGuid, NULL, (void**)&Gop);
    if (EFI_ERROR(Status) || Gop == NULL) {
        Print(L"GOP not found\n");
        while (1);
    }

    struct BootInfo binfo;
    binfo.fb.base = (uint64_t)Gop->Mode->FrameBufferBase;
    binfo.fb.width = Gop->Mode->Info->HorizontalResolution;
    binfo.fb.height = Gop->Mode->Info->VerticalResolution;
    binfo.fb.pixels_per_scanline = Gop->Mode->Info->PixelsPerScanLine;
    binfo.fb.pixel_format = (uint32_t)Gop->Mode->Info->PixelFormat;

    Print(L"Framebuffer: Base=0x%lx, %ux%u, ppsl=%u\n", binfo.fb.base, binfo.fb.width, binfo.fb.height, binfo.fb.pixels_per_scanline);

    // Open kernel.elf from same device
    EFI_LOADED_IMAGE *LoadedImage;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Fs;
    EFI_FILE_PROTOCOL *Root = NULL;
    EFI_FILE_PROTOCOL *File = NULL;

    Status = uefi_call_wrapper(BS->HandleProtocol, 3, ImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&LoadedImage);
    if (EFI_ERROR(Status)) { Print(L"HandleProtocol(LoadedImage) failed\n"); while (1); }

    Status = uefi_call_wrapper(BS->HandleProtocol, 3, LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&Fs);
    if (EFI_ERROR(Status)) { Print(L"HandleProtocol(SimpleFS) failed\n"); while (1); }

    Status = uefi_call_wrapper(Fs->OpenVolume, 2, Fs, &Root);
    if (EFI_ERROR(Status) || Root == NULL) { Print(L"OpenVolume failed\n"); while (1); }

    Status = uefi_call_wrapper(Root->Open, 5, Root, &File, L"kernel.elf", EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status) || File == NULL) { Print(L"Failed to open kernel.elf\n"); while (1); }

    // Get file size
    UINTN FileInfoSize = sizeof(EFI_FILE_INFO) + 200;
    EFI_FILE_INFO *FileInfo = NULL;
    Status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, FileInfoSize, (void**)&FileInfo);
    if (EFI_ERROR(Status)) { Print(L"AllocatePool(FileInfo) failed\n"); while (1); }

    Status = uefi_call_wrapper(File->GetInfo, 4, File, &gEfiFileInfoGuid, &FileInfoSize, FileInfo);
    if (EFI_ERROR(Status)) { Print(L"GetInfo failed\n"); while (1); }

    UINTN FileSize = (UINTN)FileInfo->FileSize;
    Print(L"kernel.elf size = %u bytes\n", FileSize);

    // Read entire file into a temporary buffer
    void *FileBuf = NULL;
    Status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, FileSize, &FileBuf);
    if (EFI_ERROR(Status)) { Print(L"AllocatePool(filebuf) failed\n"); while (1); }

    UINTN ReadSize = FileSize;
    Status = uefi_call_wrapper(File->Read, 3, File, &ReadSize, FileBuf);
    if (EFI_ERROR(Status) || ReadSize != FileSize) { Print(L"File read failed\n"); while (1); }

    // Parse ELF header
    Elf64_Ehdr *ehdr = (Elf64_Ehdr*)FileBuf;
    if (ehdr->e_ident[0] != 0x7F || ehdr->e_ident[1] != 'E' || ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F') {
        Print(L"Not an ELF file\n"); while (1);
    }

    // Load segments
    Elf64_Phdr *ph = (Elf64_Phdr*)((char*)FileBuf + ehdr->e_phoff);
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (ph[i].p_type != 1) continue; // PT_LOAD

        EFI_PHYSICAL_ADDRESS Dest = ph[i].p_vaddr;
        UINTN Pages = (ph[i].p_memsz + 0xfff) / 0x1000;
        Status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAddress, EfiLoaderData, Pages, &Dest);
        if (EFI_ERROR(Status)) { Print(L"AllocatePages failed for segment %d\n", i); while (1); }

        // Copy file data
        void *Src = (char*)FileBuf + ph[i].p_offset;
        void *Dst = (void*)(uintptr_t)ph[i].p_vaddr;
        CopyMem(Dst, Src, ph[i].p_filesz);
        // Zero-bss
        if (ph[i].p_memsz > ph[i].p_filesz) {
            UINTN ZeroSize = (UINTN)(ph[i].p_memsz - ph[i].p_filesz);
            SetMem((char*)Dst + ph[i].p_filesz, ZeroSize, 0);
        }
    }

    // Allocate BootInfo in loader memory and copy
    struct BootInfo *Boot = NULL;
    Status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, sizeof(struct BootInfo), (void**)&Boot);
    if (EFI_ERROR(Status)) { Print(L"AllocatePool(BootInfo) failed\n"); while (1); }
    CopyMem(Boot, &binfo, sizeof(struct BootInfo));

    // Get memory map and ExitBootServices
    UINTN MemoryMapSize = 0;
    EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;

    Status = uefi_call_wrapper(BS->GetMemoryMap, 5, &MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        // allocate buffer
        MemoryMapSize += 2 * DescriptorSize;
        Status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, MemoryMapSize, (void**)&MemoryMap);
        if (EFI_ERROR(Status)) { Print(L"AllocatePool(MemoryMap) failed\n"); while (1); }
        Status = uefi_call_wrapper(BS->GetMemoryMap, 5, &MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
        if (EFI_ERROR(Status)) { Print(L"GetMemoryMap failed\n"); while (1); }
    } else {
        Print(L"GetMemoryMap unexpected status\n"); while (1);
    }

    Status = uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, MapKey);
    if (EFI_ERROR(Status)) { Print(L"ExitBootServices failed\n"); while (1); }

    Print(L"Jumping to kernel entry: 0x%lx\n", ehdr->e_entry);

    // Jump to kernel entry and pass pointer to BootInfo in x0
    void (*kernel_entry)(void*) = (void(*)(void*)) (uintptr_t) ehdr->e_entry;
    kernel_entry((void*)Boot);

    // Should not return
    while (1) __asm__ volatile ("wfe");
    return EFI_SUCCESS;
}