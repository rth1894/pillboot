#include "bmp.h"
#include "efibind.h"
#include "efierr.h"
#include "efilib.h"
#include "efiprot.h"

#pragma pack(push, 1)

typedef struct {
    UINT16 type;
    UINT32 size;
    UINT16 reserved1;
    UINT16 reserved2;
    UINT32 offset;
} BMPFileHeader;

typedef struct
{
    UINT32 size;
    INT32 width;
    INT32 height;
    UINT16 planes;
    UINT16 bitsPerPixel;
    UINT32 compression;
    UINT32 imageSize;
    INT32 xPixelsPerMeter;
    INT32 yPixelsPerMeter;
    UINT32 colorsUsed;
    UINT32 importantColors;
} BMPInfoHeader;

typedef struct {
    UINT8 Blue;
    UINT8 Green;
    UINT8 Red;
    UINT8 Alpha;
} BMPPixel;

#pragma pack(pop)

EFI_STATUS bmp_load(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, CHAR16 *filename, Bitmap *bmp) {
    EFI_STATUS status;

    EFI_LOADED_IMAGE_PROTOCOL *loadedImage;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs;
    EFI_FILE_PROTOCOL *root;
    EFI_FILE_PROTOCOL *file;

    status = uefi_call_wrapper(SystemTable->BootServices->HandleProtocol, 3, ImageHandle, &gEfiLoadedImageProtocolGuid, (void **)&loadedImage);

    if (EFI_ERROR(status)) return status;

    status = uefi_call_wrapper(SystemTable->BootServices->HandleProtocol, 3, loadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void **)&fs);
    if (EFI_ERROR(status)) return status;

    status = uefi_call_wrapper(fs->OpenVolume, 2, fs, &root);
    if (EFI_ERROR(status)) return status;

    status = uefi_call_wrapper(root->Open, 5, root, &file, filename, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) return status;

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    UINTN size = sizeof(fileHeader);
    status = uefi_call_wrapper(file->Read, 3, file, &size, &fileHeader);
    if (EFI_ERROR(status)) return status;

    size = sizeof(infoHeader);
    status = uefi_call_wrapper(file->Read, 3, file, &size, &infoHeader);

    if (EFI_ERROR(status)) {
        uefi_call_wrapper(file->Close, 1, file);
        return status;
    }

    if (fileHeader.type != 0x4d42) {
        uefi_call_wrapper(file->Close, 1, file);
        return EFI_UNSUPPORTED;
    }

    if (infoHeader.bitsPerPixel != 32) {
        uefi_call_wrapper(file->Close, 1, file);
        return EFI_UNSUPPORTED;
    }

    if (infoHeader.compression != 0 && infoHeader.compression != 3) {
        uefi_call_wrapper(file->Close, 1, file);
        return EFI_UNSUPPORTED;
    }

    bmp->width = infoHeader.width;
    bmp->height = infoHeader.height;

    UINTN pixelCount = bmp->width * bmp->height;

    status = uefi_call_wrapper(SystemTable->BootServices->AllocatePool, 3, EfiLoaderData, pixelCount * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL), (void**)&bmp->pixels);

    if (EFI_ERROR(status)) {
        uefi_call_wrapper(file->Close, 1, file);
        return status;
    }

    status = uefi_call_wrapper(file->SetPosition, 2, file, fileHeader.offset);
    if (EFI_ERROR(status)) {
        bmp_free(bmp);
        uefi_call_wrapper(file->Close, 1, file);
        return status;
    }

    for (INT32 y = bmp->height - 1; y >= 0; y--) {
        for (UINT32 x = 0; x < bmp->width; x++) {
            BMPPixel pixel;
            UINTN size = sizeof(pixel);

            status = uefi_call_wrapper(file->Read, 3, file, &size, &pixel);
            if (EFI_ERROR(status)) {
                bmp_free(bmp);
                uefi_call_wrapper(file->Close, 1, file);
                return status;
            }

            EFI_GRAPHICS_OUTPUT_BLT_PIXEL *dst = &bmp->pixels[y * bmp->width + x];

            dst->Red = pixel.Red;
            dst->Green = pixel.Green;
            dst->Blue = pixel.Blue;
            dst->Reserved = pixel.Alpha;
        }
    }

    uefi_call_wrapper(file->Close, 1, file);
    return EFI_SUCCESS;
}

void bmp_free(Bitmap *bmp) {
    if(bmp->pixels) {
        FreePool(bmp->pixels);
        bmp->pixels = NULL;
    }
}
