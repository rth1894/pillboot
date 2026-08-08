#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <efi.h>
#include <efilib.h>

typedef struct
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

    UINT32 width;
    UINT32 height;
    UINT32 pixels_per_scanline;

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *framebuffer;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *backbuffer;
} Graphics;

EFI_STATUS graphics_init(EFI_SYSTEM_TABLE *SystemTable, Graphics *gfx);

void graphics_clear(Graphics *gfx, UINT8 r, UINT8 g, UINT8 b);
void graphics_present(Graphics* gfx);

#endif
