#ifndef LINUX_H
#define LINUX_H

#include <efi.h>
#include <efilib.h>

EFI_STATUS linux_boot(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);

#endif
