#ifndef RENDERER_H
#define RENDERER_H

#include "efibind.h"
#include "graphics.h"
#include "qoi.h"
#include "image.h"

void renderer_draw_image(Graphics* gfx, Image* img, INT32 x, INT32 y);

#endif
