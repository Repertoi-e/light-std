#include "vendor/Uefi/Uefi.h"

extern "C" EFI_STATUS EFIAPI efi_main(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {
    auto *out = SystemTable->ConOut;

    out->ClearScreen(out);

    out->SetCursorPosition(out, 0, 0);
    out->OutputString(out, (CHAR16 *) u"This is an EFI application running... Bye Windows soon TM I guess? ");

    out->SetCursorPosition(out, 0, 1);
    out->OutputString(out, (CHAR16 *) u"Press F12 to quit. Or don't..");

    out->SetCursorPosition(out, 0, 3);
    out->EnableCursor(out, TRUE);

    auto *in = SystemTable->ConIn;
    while (true) {
        UINTN index = 0;
        auto keyEvent = in->WaitForKey;
        SystemTable->BootServices->WaitForEvent(1, &keyEvent, &index);

        EFI_INPUT_KEY key;
        key.ScanCode = 0;
        key.UnicodeChar = 0;
        if (in->ReadKeyStroke(in, &key) != EFI_SUCCESS) continue;

        if (key.UnicodeChar == 0) {
            // Non-printable character
            switch (key.ScanCode) {
                case SCAN_UP:
                    break;
                case SCAN_DOWN:
                    break;
                case SCAN_F12:
                    return EFI_SUCCESS;
                default:
                    break;
            }
            continue;
        }

        CHAR16 text[2] = {key.UnicodeChar, L'\0'};
        out->OutputString(out, text);
    }
    return EFI_SUCCESS;
}