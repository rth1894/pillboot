#ifndef BOOT_H
#define BOOT_H

#include <efi.h>
#include <efilib.h>

EFI_STATUS boot_linux(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);
EFI_STATUS boot_windows(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);

#endif
