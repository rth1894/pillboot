#include "efibind.h"
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

    INT32 redX = leftX + ((INT32)assets->rightHand.width - (INT32)assets->redPill.width / 2) - 230;
    INT32 redY = leftY + ((INT32)assets->rightHand.height - (INT32)assets->redPill.height / 2) - 170;

    INT32 blueX = rightX + ((INT32)assets->leftHand.width - (INT32)assets->bluePill.width / 2) - 260;
    INT32 blueY = rightY + ((INT32)assets->leftHand.height - (INT32)assets->bluePill.height / 2) - 170;

    // values adjusted manually
    renderer_draw_image(gfx, &assets->redPill, redX, redY);
    renderer_draw_image(gfx, &assets->bluePill, blueX, blueY);
    graphics_present(gfx);
}
