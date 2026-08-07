#ifndef BMP_H
#define BMP_H

#include <efi.h>
#include <efilib.h>

typedef struct {
    UINT32 width;
    UINT32 height;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL* pixels;
} Bitmap;

EFI_STATUS bmp_load(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable, CHAR16* filename, Bitmap* bmp);

void bmp_free(Bitmap* bmp);

#endif
