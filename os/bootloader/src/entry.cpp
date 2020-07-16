#include "vendor/Uefi/Uefi.h"

EFI_STATUS EFIAPI efi_main(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *conOut = SystemTable->ConOut;

    conOut->ClearScreen(conOut);

    conOut->SetCursorPosition(conOut, 0, 0);
    conOut->OutputString(conOut, (CHAR16 *) u"Tova mai e operacionna sistema ! S O T I  E  H A K E R");

    conOut->SetCursorPosition(conOut, 0, 1);
    conOut->OutputString(conOut, (CHAR16 *) u"Press any key to type.");

    conOut->SetCursorPosition(conOut, 0, 2);
    conOut->OutputString(conOut, (CHAR16 *) u"Or press F12 to quit. Or don't.. I DON'T CARE I AM SO HAPPY LOL. BYE WINDOWS.");

    conOut->SetCursorPosition(conOut, 0, 4);
    conOut->EnableCursor(conOut, TRUE);

    EFI_SIMPLE_TEXT_INPUT_PROTOCOL *conIn = SystemTable->ConIn;

    for (;;) {
        UINTN index = 0;
        EFI_EVENT keyEvent = conIn->WaitForKey;
        SystemTable->BootServices->WaitForEvent(1, &keyEvent, &index);

        EFI_INPUT_KEY key;
        key.ScanCode = 0;
        key.UnicodeChar = 0;
        if (conIn->ReadKeyStroke(conIn, &key) != EFI_SUCCESS)
            continue;

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
        conOut->OutputString(conOut, text);
    }

    return EFI_SUCCESS;
}