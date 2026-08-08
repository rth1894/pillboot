#include <efi.h>
#include <efilib.h>

#include "assets.h"
#include "boot.h"
#include "chainload.h"
#include "efidef.h"
#include "efierr.h"
#include "graphics.h"
#include "image.h"
#include "input.h"
#include "linux.h"
#include "scene.h"

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);
    Print(L"PillBoot started\r\n");
    Graphics gfx;
    Print(L"Initializing graphics...\r\n");

    EFI_STATUS status = graphics_init(SystemTable, &gfx);
    if (EFI_ERROR(status)) {
        Print(L"graphics_init FAILED\r\n");
        while (1);
    }
    Print(L"Graphics OK\r\n");
    Print(L"Initializing input...\r\n");
    status = input_init(SystemTable);

    if (EFI_ERROR(status)) {
        Print(L"input_init FAILED\r\n");
        while (1);
    }

    Print(L"Input OK\r\n");
    Assets assets;
    Print(L"Loading assets...\r\n");

    status = assets_load(ImageHandle, SystemTable, &assets);

    if (EFI_ERROR(status)) {
        Print(L"assets_load FAILED\r\n");
        while (1);
    }

    Print(L"Assets OK\r\n");
    MenuState menu = { .selected = MENU_LEFT, .confirmed = 0 };
    Print(L"Rendering scene...\r\n");

    scene_render(&gfx, &assets, &menu);
    Print(L"Scene rendered\r\n");

    while (1) {
        InputKey key = input_poll();

        switch (key) {
            case KEY_LEFT:
                menu.selected = MENU_LEFT;
                break;

            case KEY_RIGHT:
                menu.selected = MENU_RIGHT;
                break;

            case KEY_ENTER:
                if (menu.selected == MENU_LEFT) {
                    status = linux_boot(ImageHandle, SystemTable);

                    if (EFI_ERROR(status)) {
                        Print(L"Linux boot failed: %r\r\n", status);
                        while(1);
                    }
                }
                break;

            case KEY_ESCAPE:
                Print(L"ESC\r\n");
                break;

            default:
                break;
        }

        scene_render(&gfx, &assets, &menu);
    }
    return EFI_SUCCESS;
}
