#include "graphics.h"
#include "renderer.h"
#include "scene.h"

void scene_render(Graphics *gfx, Assets *assets, MenuState *state) {
    graphics_clear(gfx, 0, 0, 0);

    INT32 spacing = 140;
    INT32 leftX = gfx->width / 2 - assets->leftHand.width - spacing / 2;
    INT32 rightX = gfx->width / 2 + spacing / 2;

    INT32 leftY = gfx->height / 2 - assets->leftHand.height / 2;
    INT32 rightY = leftY;

    if (state->selected == MENU_LEFT) leftY -= 20;
    else rightY -= 20;

    renderer_draw_image(gfx, &assets->rightHand, leftX, leftY);
    renderer_draw_image(gfx, &assets->leftHand, rightX, rightY);
    renderer_draw_image(gfx, &assets->redPill, leftX, leftY);
    renderer_draw_image(gfx, &assets->bluePill, rightX, rightY);
    graphics_present(gfx);
}
