#include "graphics.h"
#include "efibind.h"
#include "efidef.h"
#include "efierr.h"
#include "efiprot.h"
#include "legacy/efilib.h"

EFI_STATUS graphics_init(EFI_SYSTEM_TABLE* SystemTable, Graphics* gfx) {
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_STATUS status;

    status = SystemTable->BootServices->LocateProtocol(&gopGuid, NULL, (void**)&gfx->gop);

    if (EFI_ERROR(status)) return status;

    gfx->width = gfx->gop->Mode->Info->HorizontalResolution;
    gfx->height = gfx->gop->Mode->Info->VerticalResolution;
    gfx->pixels_per_scanline = gfx->gop->Mode->Info->PixelsPerScanLine;
    gfx->framebuffer = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL*)gfx->gop->Mode->FrameBufferBase;

    UINTN size = gfx->width * gfx->height * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
    status = uefi_call_wrapper(SystemTable->BootServices->AllocatePool, 3, EfiLoaderData, size, (void**)&gfx->backbuffer);

    if (EFI_ERROR(status)) return status;

    return EFI_SUCCESS;
}

void graphics_clear(Graphics* gfx, UINT8 r, UINT8 g, UINT8 b) {
    for (UINT32 y=0; y < gfx->height; y++) {
        EFI_GRAPHICS_OUTPUT_BLT_PIXEL* row = gfx->backbuffer + y * gfx->pixels_per_scanline;

        for (UINT32 x=0; x<gfx->width; x++) {
            row[x].Red = r;
            row[x].Green= g;
            row[x].Blue= b;
            row[x].Reserved = 0;
        }
    }
}

void graphics_present(Graphics *gfx) {
    UINTN pixels = gfx->width * gfx->height;
    CopyMem(gfx->framebuffer, gfx->backbuffer, pixels * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
}
