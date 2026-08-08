#include "chainload.h"

EFI_STATUS chainload(EFI_HANDLE ImageHandle, CHAR16 *path) {
    EFI_STATUS status;
    EFI_LOADED_IMAGE_PROTOCOL *loadedImage;
    EFI_DEVICE_PATH_PROTOCOL *devicePath;
    EFI_HANDLE childHandle = NULL;

    status = uefi_call_wrapper(gBS->HandleProtocol, 3, ImageHandle, &gEfiLoadedImageProtocolGuid, (void **)&loadedImage);
    if (EFI_ERROR(status)) return status;

    devicePath = FileDevicePath(loadedImage->DeviceHandle, path);

    if (devicePath == NULL) return EFI_OUT_OF_RESOURCES;
    status = uefi_call_wrapper(gBS->LoadImage, 6, FALSE, ImageHandle, devicePath, NULL, 0, &childHandle);
    if (EFI_ERROR(status)) {
        FreePool(devicePath);
        return status;
    }
    status = uefi_call_wrapper(gBS->StartImage, 3, childHandle, NULL, NULL);
    FreePool(devicePath);

    return status;
}
