#include "input.h"

static EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *textInput = NULL;

EFI_STATUS input_init(EFI_SYSTEM_TABLE *SystemTable) {
    EFI_GUID guid = EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL_GUID;

    return uefi_call_wrapper(SystemTable->BootServices->LocateProtocol, 3, &guid, NULL, (void **)&textInput);
}

InputKey input_poll(void) {
    if (textInput == NULL) return KEY_NONE;

    EFI_KEY_DATA keyData;
    EFI_STATUS status = uefi_call_wrapper(textInput->ReadKeyStrokeEx, 2, textInput, &keyData);
    if (EFI_ERROR(status)) return KEY_NONE;

    switch (keyData.Key.ScanCode) {
        case SCAN_LEFT: return KEY_LEFT;
        case SCAN_RIGHT: return KEY_RIGHT;
        case SCAN_ESC: return KEY_ESCAPE;
    }

    if (keyData.Key.UnicodeChar == CHAR_CARRIAGE_RETURN) return KEY_ENTER;
    return KEY_NONE;
}
