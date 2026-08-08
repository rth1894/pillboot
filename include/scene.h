#ifndef SCENE_H
#define SCENE_H

#include "assets.h"
#include "graphics.h"

typedef enum {
    MENU_LEFT,
    MENU_RIGHT
} MenuSelection;

typedef struct {
    MenuSelection selected;
} MenuState;

void scene_render(Graphics* gfx, Assets* assets, MenuState* state);

#endif
