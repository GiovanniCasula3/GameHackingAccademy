#include <Windows.h>

//puntatori a indirizzi di memoria importanti
DWORD* player_base;
DWORD* game_base;
DWORD* gold;
DWORD ret_address = 0xCCAF90;


__declspec(naked) void codecave() {
	//con __asm posso scrivere codice assembly
	__asm {
		pushad
	}

	player_base = (DWORD*)0x017EED18;
	game_base = (DWORD*)(*player_base + 0xA90);
	gold = (DWORD*)(*game_base + 4);
	*gold = 888;

	_asm {
		popad
		mov eax, dword ptr ds : [ecx]
		lea esi, dword ptr ds : [esi]
		jmp ret_address
	}
}

// Quando il DLL viene collegato, rimuovere la protezione dalla memoria nell’area del codice su cui vogliamo scrivere
// Poi imposta il primo opcode a 0xE9, cioè un jump
// Calcolo la posizione usando la formula: new_location - (original_location + 5)
// Infine, dato che le istruzioni originali occupavano in totale 6 byte, devo mettere NOP nell’ultimo byte rimasto

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	DWORD old_protect;
	unsigned char* hook_location = (unsigned char*)0x00CCAF8A;

	if (fdwReason == DLL_PROCESS_ATTACH) {
		VirtualProtect((void*)hook_location, 6, PAGE_EXECUTE_READWRITE, &old_protect);
		*hook_location = 0xE9;
		*(DWORD*)(hook_location + 1) = (DWORD)&codecave - ((DWORD)hook_location + 5);
		*(hook_location + 5) = 0x90;
	}

	return true;
}