#ifndef IMAGE_H
#define IMAGE_H

#include <efi.h>
#include <efilib.h>

typedef struct {
    UINT32 width;
    UINT32 height;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *pixels;
} Image;

EFI_STATUS image_load(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, CHAR16 *filename, Image *image);
void image_free(Image *image);

#endif
