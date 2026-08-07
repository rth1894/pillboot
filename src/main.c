#include <efi.h>
#include <efilib.h>

#include "graphics.h"

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    Graphics gfx;
    EFI_STATUS status = graphics_init(SystemTable, &gfx);

    if(EFI_ERROR(status)) {
        Print(L"Failed to init graphics.\r\n");
        while(1);
    }

    graphics_clear(&gfx, 0, 0, 0);
    while(1);
    return EFI_SUCCESS;
}
