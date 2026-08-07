#include <efi.h>
#include <efilib.h>

#include "bmp.h"
#include "graphics.h"

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    InitializeLib(ImageHandle, SystemTable);
    Graphics gfx;

    EFI_STATUS status = graphics_init(SystemTable, &gfx);
    if (EFI_ERROR(status)) {
        Print(L"graphics_init failed\r\n");
        while (1);
    }

    graphics_clear(&gfx, 255, 255, 255);

    Bitmap bmp;
    status = bmp_load(ImageHandle, SystemTable, L"\\EFI\\BOOT\\assets\\left_hand.bmp", &bmp);

    if (EFI_ERROR(status)) graphics_clear(&gfx, 255, 0, 0);
    else {
        graphics_clear(&gfx, 0, 255, 0);
        Print(L"Loaded!\r\n");
    }

    while (1);
    return EFI_SUCCESS;
}
