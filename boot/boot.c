#include <efi.h>
#include <efilib.h>

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    ST->ConOut->ClearScreen(ST->ConOut);
    Print(L"AURA OS Bootloader\n");
    Print(L"Loading microkernel...\n");

    // позже: загрузка ELF, framebuffer info
    while (1);
    return EFI_SUCCESS;
}