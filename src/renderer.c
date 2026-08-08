#include "image.h"
#include "renderer.h"
#include "efiprot.h"

void renderer_draw_image(Graphics* gfx, Image* img, INT32 x, INT32 y) {
    for (UINT32 sy=0; sy < img->height; sy++) {
        INT32 dy = y + sy;
        if (dy < 0 || dy >= (INT32)gfx->height) continue;

        for (UINT32 sx=0; sx < img->width; sx++) {
            INT32 dx = x + sx;
            if (dx < 0 || dx >= (INT32)gfx->width) continue;

            EFI_GRAPHICS_OUTPUT_BLT_PIXEL* src = &img->pixels[sy * img->width + sx];
            EFI_GRAPHICS_OUTPUT_BLT_PIXEL* dst = gfx->backbuffer + dy * gfx->pixels_per_scanline + dx;

            if (src->Reserved == 0) continue;
            *dst = *src;
        }
    }
}
