#include "SaveCard.h"

#include <Windows.h>

#include "MemMods\Hook.h"
#include "General\ModuleInfo.h"
#include "General\Buffer.h"
#include "Functions\AAEdit\Globals.h"

namespace EditInjections {
namespace SaveCard {


void __stdcall AddUnlimitData(wchar_t* fileName) {
	//open card
	HANDLE hFile = CreateFile(fileName, FILE_GENERIC_READ | FILE_GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE || hFile == NULL) return;

	//generate unlimited data buffer
	int size = 0;
	char* buffer = NULL;
	int retSize = AAEdit::g_cardData.ToBuffer(&buffer, &size, true, false);
	if (retSize == 0) return;

	DWORD foo = 0;

	//read data offset from end
	SetFilePointer(hFile, -4, NULL, FILE_END);
	DWORD aa2DataOffset = 0;
	ReadFile(hFile, &aa2DataOffset, 4, &foo, NULL);

	//add our data
	WriteFile(hFile, buffer, retSize, &foo, NULL);

	//write new offset to end
	aa2DataOffset += retSize + 4; //+4 cause the old offset is also between it
	WriteFile(hFile, &aa2DataOffset, 4, &foo, NULL);

	delete[] buffer;
	CloseHandle(hFile);
}

DWORD AddUnlimitDataOriginalFunction;
void __declspec(naked) AddUnlimitDataRedirect() {
	__asm {
		push eax //save file path

		push [esp+8]
		call [AddUnlimitDataOriginalFunction]
		push eax //save return value

		mov eax, [esp+4]
		push eax //push rescued parameter
		call AddUnlimitData

		pop eax //get return value back
		add esp, 4 //get rid of file path that was saved
		ret 4
	}
}


void AddUnlimitDataInject() {
	//the save card data function call. one stack parameter stdcall, as well as eax (wchar_t* fileName) and ecx (?)
	/*AA2Edit.exe+1BEC8 - 50                    - push eax
	AA2Edit.exe+1BEC9 - 8D 84 24 34050000     - lea eax,[esp+00000534]
	AA2Edit.exe+1BED0 - E8 EBA21000           - call AA2Edit.exe+1261C0	*/
	DWORD address = General::GameBase + 0x1BED0;
	DWORD redirectAddress = (DWORD)(&AddUnlimitDataRedirect);
	Hook((BYTE*)address,
	{ 0xE8, 0xEB, 0xA2, 0x10, 0x00 },						//expected values
	{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		&AddUnlimitDataOriginalFunction);
}



}
}