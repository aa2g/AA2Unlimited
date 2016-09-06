#include "FileDump.h"

#include "Files\Config.h"
#include "MemMods\Hook.h"
#include "General\ModuleInfo.h"
#include "Functions\Shared\Overrides.h"

namespace SharedInjections {
namespace FileDump {

/*
 * File Dump Event. This even fires when the game requests certain files on the file system (not inside a pp archive)
 * to be dumped into a BYTE array. Return a valid byte array and change *readBytes to its size to skip the
 * original call. make sure to use the IllusionAlloc function for the buffer
 */
BYTE* __stdcall FileDumpEvent(wchar_t* fileName, DWORD* readBytes) {
	BYTE* ret = Shared::EyeTextureDump(fileName, readBytes);
	if (ret != NULL) return ret;
	return NULL;
}

void __declspec(naked) FileDumpStartRedirect() {
	__asm {
		push eax //rescue values
		push edx

		mov ecx, [esp+0x8 + 0xC]
		push ecx
		mov eax, [eax]
		push eax
		call FileDumpEvent
		test eax, eax
		je FileDumpStartRedirect_normal

		pop edx
		add esp, 0xC //remove saved eax, return value and the ecx from the original call
		ret
	FileDumpStartRedirect_normal:

		pop edx
		pop eax 
		mov eax, [eax]
		cmp dword ptr [eax-0xC], 00
		ret
	}
}


void FileDumpStartInject() {
	if (General::IsAAEdit) {
		//this function reads a file, allocates a buffer for it, dumps the file and returns the buffer and its size.
		//__cdecl BYTE* DumpFile([esp+4] = DWORD* outSize, eax = WCHAR_T** fileName)
		/*AA2Edit.exe+1FDC30 - 51                    - push ecx
		AA2Edit.exe+1FDC31 - 8B 00                 - mov eax,[eax]
		AA2Edit.exe+1FDC33 - 83 78 F4 00           - cmp dword ptr [eax-0C],00 { 0 }
		AA2Edit.exe+1FDC37 - 55                    - push ebp	*/
		DWORD address = General::GameBase + 0x1FDC31;
		DWORD redirectAddress = (DWORD)(&FileDumpStartRedirect);
		Hook((BYTE*)address,
			{ 0x8B, 0x00, 0x83, 0x78, 0xF4, 0x00 },						//expected values
			{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress, 0x90 },	//redirect to our function
			NULL);
	}
	else if (General::IsAAPlay) {
		/*AA2Play v12 FP v1.4.0a.exe+21B770 - 51                    - push ecx
		AA2Play v12 FP v1.4.0a.exe+21B771 - 8B 00                 - mov eax,[eax]
		AA2Play v12 FP v1.4.0a.exe+21B773 - 83 78 F4 00           - cmp dword ptr [eax-0C],00 { 0 }
		AA2Play v12 FP v1.4.0a.exe+21B777 - 55                    - push ebp	*/
		DWORD address = General::GameBase + 0x21B771;
		DWORD redirectAddress = (DWORD)(&FileDumpStartRedirect);
		Hook((BYTE*)address,
			{ 0x8B, 0x00, 0x83, 0x78, 0xF4, 0x00 },						//expected values
			{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress, 0x90 },	//redirect to our function
			NULL);
	}
	

	
}

}
}
