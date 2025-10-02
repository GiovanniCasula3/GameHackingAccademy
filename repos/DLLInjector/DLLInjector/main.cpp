// inject.cpp
#include <windows.h>
#include <tlhelp32.h>
#include <cstdio>

static void die(const char* where) {
    DWORD e = GetLastError();
    LPSTR msg = nullptr;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, e, 0, (LPSTR)&msg, 0, NULL);
    fprintf(stderr, "%s failed: %lu (%s)\n", where, e, msg ? msg : "");
    if (msg) LocalFree(msg);
}

int main() {
    // questo da cambiare ogni volta che devo iniettare un dll
    const char* dll_path = "C:\\Users\\Docker\\source\\repos\\dllcodecave\\Debug\\dllcodecave.dll";
    const wchar_t* target = L"wesnoth.exe"; // per prova puoi usare L"notepad.exe"

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) { die("CreateToolhelp32Snapshot"); return 1; }

    PROCESSENTRY32W pe = { sizeof(pe) };
    if (!Process32FirstW(snap, &pe)) { die("Process32FirstW"); CloseHandle(snap); return 1; }

    DWORD pid = 0;
    do {
        if (_wcsicmp(pe.szExeFile, target) == 0) { pid = pe.th32ProcessID; break; }
    } while (Process32NextW(snap, &pe));
    CloseHandle(snap);

    if (!pid) { fprintf(stderr, "Process %S not found.\n", target); return 1; }

    HANDLE proc = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
        FALSE, pid);
    if (!proc) { die("OpenProcess"); return 1; }

    SIZE_T sz = strlen(dll_path) + 1;
    LPVOID remote = VirtualAllocEx(proc, NULL, sz, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remote) { die("VirtualAllocEx"); CloseHandle(proc); return 1; }

    if (!WriteProcessMemory(proc, remote, dll_path, sz, NULL)) {
        die("WriteProcessMemory"); VirtualFreeEx(proc, remote, 0, MEM_RELEASE); CloseHandle(proc); return 1;
    }

    HMODULE k32 = GetModuleHandleA("kernel32.dll");
    if (!k32) { die("GetModuleHandleA"); VirtualFreeEx(proc, remote, 0, MEM_RELEASE); CloseHandle(proc); return 1; }

    FARPROC pLoadLibA = GetProcAddress(k32, "LoadLibraryA");
    if (!pLoadLibA) { die("GetProcAddress(LoadLibraryA)"); VirtualFreeEx(proc, remote, 0, MEM_RELEASE); CloseHandle(proc); return 1; }

    HANDLE th = CreateRemoteThread(proc, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibA, remote, 0, NULL);
    if (!th) { die("CreateRemoteThread"); VirtualFreeEx(proc, remote, 0, MEM_RELEASE); CloseHandle(proc); return 1; }

    WaitForSingleObject(th, INFINITE);

    DWORD exitCode = 0;
    if (!GetExitCodeThread(th, &exitCode)) die("GetExitCodeThread");
    else printf("Remote thread exit code (HMODULE) = 0x%08lX\n", exitCode);

    VirtualFreeEx(proc, remote, 0, MEM_RELEASE);
    CloseHandle(th);
    CloseHandle(proc);

    puts("Done.");
    return 0;
}