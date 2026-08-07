#ifndef RENDERER_H
#define RENDERER_H

#include "graphics.h"

void draw_pixel(Graphics* gfx, UINT32 x, UINT32 y, UINT8 r, UINT8 g, UINT8 b);

void fill_rect(Graphics* gfx, UINT32 x, UINT32 y, UINT32 width, UINT32 height, UINT8 r, UINT8 g, UINT8 b);

void draw_image(Graphics* gfx, UINT32 x, UINT32 y, UINT32 width, UINT32 height, const EFI_GRAPHICS_OUTPUT_BLT_PIXEL* pixels);

#endif
