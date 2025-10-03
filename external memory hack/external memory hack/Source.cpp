#include <Windows.h>

typedef const void* LPCVOID;

int main(int argc, char** argv) {
	DWORD gold_value = 0;
	DWORD bytes_read = 0;
	DWORD process_id = 0;

	HWND wesnoth_window = FindWindow(NULL, L"The Battle for Wesnoth - 1.14.9");

	GetWindowThreadProcessId(wesnoth_window, &process_id);

	HANDLE wesnoth_process = OpenProcess(PROCESS_ALL_ACCESS, true, process_id);
	ReadProcessMemory(wesnoth_process, (void*)0x017EED18, &gold_value, 4, &bytes_read);
	
	gold_value += 0xA90;
	ReadProcessMemory(wesnoth_process, (void*)gold_value, &gold_value, 4, &bytes_read);

	gold_value += 4;
	DWORD new_gold_value = 555;
	DWORD bytes_written = 0;
	WriteProcessMemory(wesnoth_process, (void*)gold_value, &new_gold_value, 4, &bytes_written);
	return 0;
}