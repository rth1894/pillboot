#ifndef CHAINLOAD_H
#define CHAINLOAD_H

#include <efi.h>
#include <efilib.h>

EFI_STATUS chainload(EFI_HANDLE ImageHandle, CHAR16* path);

#endif
