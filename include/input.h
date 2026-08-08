#ifndef INPUT_H
#define INPUT_H

#include <efi.h>
#include <efilib.h>

typedef enum {
    KEY_NONE,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_ENTER,
    KEY_ESCAPE
} InputKey;

EFI_STATUS input_init(EFI_SYSTEM_TABLE* SystemTable);

InputKey input_poll(void);

#endif
