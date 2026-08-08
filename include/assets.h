#ifndef ASSETS_H
#define ASSETS_H

#include "image.h"

typedef struct {
    Image leftHand;
    Image rightHand;

    Image leftShadow;
    Image rightShadow;

    Image redPill;
    Image bluePill;
} Assets;

EFI_STATUS assets_load(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable, Assets* assets);
void assets_free(Assets* assets);

#endif
