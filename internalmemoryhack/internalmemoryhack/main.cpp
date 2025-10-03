#include <windows.h>
#pragma comment(lib, "user32.lib")

static DWORD WINAPI ShowMsg(LPVOID) {
    MessageBoxW(NULL, L"Injected OK", L"Test DLL", MB_OK);
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinst);
        // NON fare MessageBox direttamente in DllMain: crea un thread
        CreateThread(NULL, 0, ShowMsg, NULL, 0, NULL);

    }
    return TRUE;
}