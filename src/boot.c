#include "boot.h"
#include "efibind.h"
#include "efiprot.h"

static EFI_STATUS open_esp(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, EFI_FILE_PROTOCOL **root) {
    EFI_STATUS status;

    EFI_LOADED_IMAGE_PROTOCOL *loadedImage;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs;

    status = uefi_call_wrapper(SystemTable->BootServices->HandleProtocol, 3, ImageHandle, &gEfiLoadedImageProtocolGuid, (void **)&loadedImage);
    if (EFI_ERROR(status)) return status;

    status = uefi_call_wrapper(SystemTable->BootServices->HandleProtocol, 3, loadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void **)&fs);
    if (EFI_ERROR(status)) return status;

    return uefi_call_wrapper(fs->OpenVolume, 2, fs, root);
}

static EFI_STATUS file_exists(EFI_FILE_PROTOCOL *root, CHAR16 *path) {
    EFI_STATUS status;
    EFI_FILE_PROTOCOL *file;

    status = uefi_call_wrapper(root->Open, 5, root, &file, path, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) return status;
    uefi_call_wrapper(file->Close, 1, file);

    return EFI_SUCCESS;
}

EFI_STATUS boot_linux(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS status;
    EFI_FILE_PROTOCOL *root;

    status = open_esp(ImageHandle, SystemTable, &root);
    if (EFI_ERROR(status)) return status;

    return file_exists(root, L"\\EFI\\Linux\\grubx64.efi");
}

EFI_STATUS boot_windows(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS status;
    EFI_FILE_PROTOCOL *root;

    status = open_esp(ImageHandle, SystemTable, &root);
    if (EFI_ERROR(status)) return status;
    return file_exists(root, L"\\EFI\\Microsoft\\Boot\\bootmgfw.efi");
}
