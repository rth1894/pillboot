#include "assets.h"
#include "image.h"
#include "efidef.h"
#include "efierr.h"

EFI_STATUS assets_load(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable, Assets *assets) {
    EFI_STATUS status;
    Print(L"Loading right_hand.qoi...\r\n");

    status = image_load(ImageHandle, SystemTable, L"\\EFI\\BOOT\\assets\\right_hand.qoi", (Image *)&assets->rightHand);
    if (EFI_ERROR(status)) {
        Print(L"FAILED right_hand.qoi: %r\r\n", status);
        return status;
    }

    Print(L"right_hand.qoi OK\r\n");
    Print(L"Loading left_hand.qoi...\r\n");

    status = image_load(ImageHandle, SystemTable, L"\\EFI\\BOOT\\assets\\left_hand.qoi", (Image *)&assets->leftHand);
    if (EFI_ERROR(status)) {
        Print(L"FAILED left_hand.qoi: %r\r\n", status);
        return status;
    }

    Print(L"left_hand.qoi OK\r\n");
    assets->rightShadow = assets->leftShadow;

    Print(L"Loading red_pill.qoi...\r\n");
    status = image_load(ImageHandle, SystemTable, L"\\EFI\\BOOT\\assets\\red_pill.qoi", (Image *)&assets->redPill);
    if (EFI_ERROR(status)) {
        Print(L"FAILED red_pill.qoi: %r\r\n", status);
        return status;
    }

    Print(L"red_pill.qoi OK\r\n");
    Print(L"Loading blue_pill.qoi...\r\n");

    status = image_load(ImageHandle, SystemTable, L"\\EFI\\BOOT\\assets\\blue_pill.qoi", (Image *)&assets->bluePill);
    if (EFI_ERROR(status)) {
        Print(L"FAILED blue_pill.qoi: %r\r\n", status);
        return status;
    }

    Print(L"blue_pill.qoi OK\r\n");
    return EFI_SUCCESS;
}

void assets_free(Assets *assets) {
    image_free((Image *)&assets->leftHand);
    image_free((Image *)&assets->rightHand);
    image_free((Image *)&assets->redPill);
    image_free((Image *)&assets->bluePill);
}
