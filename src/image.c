#include "image.h"

#define QOI_NO_STDIO
#define QOI_IMPLEMENTATION
#include "qoi.h"

#include <efibind.h>
#include <efierr.h>
#include <efiprot.h>


/*
 * CANNOT USE hosted C functions, program is freestanding

 * read qoi through uefi filesystem api
 * AllocatePool() instead of malloc()
 * FreePool() instead of free()
 * decode qoi in memory
*/

static EFI_SYSTEM_TABLE* gSystemTable = NULL;
void* malloc(size_t size) {
    void* ptr = NULL;

    EFI_STATUS status = uefi_call_wrapper(gSystemTable->BootServices->AllocatePool, 3, EfiLoaderData, size, &ptr);
    if (EFI_ERROR(status)) return NULL;
    return ptr;
}

void free(void* ptr) {
    if (ptr != NULL) FreePool(ptr);
}

static EFI_STATUS read_file(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable, CHAR16* filename, UINT8** data, UINTN* size) {
    EFI_STATUS status;

    EFI_LOADED_IMAGE_PROTOCOL* loadedImage;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;
    EFI_FILE_PROTOCOL* root;
    EFI_FILE_PROTOCOL* file;

    status = uefi_call_wrapper(SystemTable->BootServices->HandleProtocol, 3, ImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&loadedImage);
    if (EFI_ERROR(status)) return status;

    status = uefi_call_wrapper(SystemTable->BootServices->HandleProtocol, 3, loadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&fs);
    if (EFI_ERROR(status)) return status;

    status = uefi_call_wrapper(fs->OpenVolume, 2, fs, &root);
    if (EFI_ERROR(status)) return status;

    status = uefi_call_wrapper(root->Open, 5, root, &file, filename, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) return status;

    EFI_FILE_INFO* info;
    UINTN infoSize = 0;

    status = uefi_call_wrapper(file->GetInfo, 4, file, &gEfiFileInfoGuid, &infoSize, NULL);
    if (status != EFI_BUFFER_TOO_SMALL) {
        uefi_call_wrapper(file->Close, 1, file);
        return status;
    }

    status = uefi_call_wrapper(SystemTable->BootServices->AllocatePool, 3, EfiLoaderData, infoSize, (void**)&info);
    if (EFI_ERROR(status)) {
        uefi_call_wrapper(file->Close, 1, file);
        return status;
    }

    status = uefi_call_wrapper(file->GetInfo, 4, file, &gEfiFileInfoGuid, &infoSize, info);
    if (EFI_ERROR(status)) {
        FreePool(info);
        uefi_call_wrapper(file->Close, 1, file);
        return status;
    }

    *size = info->FileSize;
    FreePool(info);

    status = uefi_call_wrapper(SystemTable->BootServices->AllocatePool, 3, EfiLoaderData, *size, (void**)data);

    if (EFI_ERROR(status)) {
        uefi_call_wrapper(file->Close, 1, file);
        return status;
    }

    UINTN readSize = *size;
    status = uefi_call_wrapper( file->Read, 3, file, &readSize, *data);
    uefi_call_wrapper(file->Close, 1, file);
    if (EFI_ERROR(status)) {
        FreePool(*data);
        *data = NULL;
        return status;
    }

    *size = readSize;
    return EFI_SUCCESS;
}


EFI_STATUS image_load(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable, CHAR16* filename, Image* image) {
    EFI_STATUS status;

    image->width = 0;
    image->height = 0;
    image->pixels = NULL;

    gSystemTable = SystemTable;

    UINT8* fileData = NULL;
    UINTN fileSize = 0;

    status = read_file(ImageHandle, SystemTable, filename, &fileData, &fileSize);
    if (EFI_ERROR(status)) return status;

    qoi_desc desc;
    void* decoded = qoi_decode(fileData, (int)fileSize, &desc, 4);
    FreePool(fileData);
    if (decoded == NULL) return EFI_UNSUPPORTED;


    UINTN pixelCount = (UINTN)desc.width * (UINTN)desc.height;

    UINTN imageSize = pixelCount * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);

    status = uefi_call_wrapper(SystemTable->BootServices->AllocatePool, 3, EfiLoaderData, imageSize, (void**)&image->pixels);

    if (EFI_ERROR(status)) {
        free(decoded);
        return status;
    }
    image->width = desc.width;
    image->height = desc.height;

    UINT8* src = (UINT8*)decoded;

    for (UINTN i=0; i<pixelCount; i++) {
        image->pixels[i].Red = src[i * 4 + 0];
        image->pixels[i].Green = src[i * 4 + 1];
        image->pixels[i].Blue = src[i * 4 + 2];
        image->pixels[i].Reserved = src[i * 4 + 3];
    }
    free(decoded);
    return EFI_SUCCESS;
}


void image_free(Image* image)
{
    if (image->pixels != NULL) {
        FreePool(image->pixels);
        image->pixels = NULL;
    }

    image->width = 0;
    image->height = 0;
}
