#include "renderer.h"
#include "graphics.h"

void draw_pixel(Graphics *gfx, UINT32 x, UINT32 y, UINT8 r, UINT8 g, UINT8 b) {
    if (x >= gfx->width || y >= gfx->height) return;

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL* pixel = gfx->framebuffer + y * gfx->pixels_per_scanline + x;
    pixel->Red = r;
    pixel->Green = g;
    pixel->Blue = b;
    pixel->Reserved = 0;
}

void fill_rect(Graphics* gfx, UINT32 x, UINT32 y, UINT32 width, UINT32 height, UINT8 r, UINT8 g, UINT8 b) {
    for (UINT32 yy=0; yy<height; yy++) {
        for (UINT32 xx=0; xx<width; xx++) {
            draw_pixel(gfx, x + xx, y + yy, r, g, b);
        }
    }
}

void draw_image(Graphics *gfx, UINT32 x, UINT32 y, UINT32 width, UINT32 height, const EFI_GRAPHICS_OUTPUT_BLT_PIXEL *pixels) {
    for (UINT32 yy=0; yy<height; yy++) {
        if (y + yy >= gfx->height) break;

        for (UINT32 xx=0; xx<width; xx++) {
            if (x + xx >=gfx->width) break;

            gfx->framebuffer[(y+yy)* gfx->pixels_per_scanline + (x+xx)] = pixels[yy * width + xx];
        }
    }
}
