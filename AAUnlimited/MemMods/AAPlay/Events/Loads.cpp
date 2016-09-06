#include "Loads.h"

#include "MemMods\Hook.h"
#include "General\ModuleInfo.h"
#include "External\ExternalClasses\CharacterStruct.h"

#include "Functions\Shared\Overrides.h"

namespace PlayInjections {
/*
 * Events for the Loading of stuff, such as hi-poly models, lo-poly models etc
 */
namespace Loads {


void __stdcall HiPolyLoadStartEvent(ExtClass::CharacterStruct* loadCharacter) {
	Shared::MeshTextureCharLoadStart(loadCharacter);
}

void __stdcall HiPolyLoadEndEvent() {
	Shared::MeshTextureCharLoadEnd();
}

void __declspec(naked) HiPolyLoadsStartRedirect() {
	__asm {
		pushad
		push ecx
		call HiPolyLoadStartEvent
		popad
		//emulate original code (the push 518)
		push [esp]
		mov dword ptr [esp+4], 518
		ret
	}
}

void __declspec(naked) HiPolyLoadsEndRedirect() {
	__asm {
		pushad
		call HiPolyLoadEndEvent
		popad
		mov esp, ebp
		pop ebp
		ret 0x10
	}
}


void HiPolyLoadsInjection() {
	/*AA2Play v12 FP v1.4.0a.exe+111B70 - 55                    - push ebp			<-- beginning of function
	AA2Play v12 FP v1.4.0a.exe+111B71 - 8B EC                 - mov ebp,esp
	AA2Play v12 FP v1.4.0a.exe+111B73 - 83 E4 F8              - and esp,-08 { 248 }
	...
	AA2Play v12 FP v1.4.0a.exe+111BAA - 64 A3 00000000        - mov fs:[00000000],eax { 0 }
	AA2Play v12 FP v1.4.0a.exe+111BB0 - 68 06020000           - push 00000206 { 518 }
	AA2Play v12 FP v1.4.0a.exe+111BB5 - 8B D9                 - mov ebx,ecx
	AA2Play v12 FP v1.4.0a.exe+111BB5 - 8B D9                 - mov ebx,ecx
	AA2Play v12 FP v1.4.0a.exe+111BB7 - 33 FF                 - xor edi,edi
	AA2Play v12 FP v1.4.0a.exe+111BB9 - 8D 8C 24 B2000000     - lea ecx,[esp+000000B2]

		...

	AA2Play v12 FP v1.4.0a.exe+113627 - E8 2D641700           - call "AA2Play v12 FP v1.4.0a.exe"+289A59 { ->AA2Play v12 FP v1.4.0a.exe+289A59 }
	AA2Play v12 FP v1.4.0a.exe+11362C - 8B E5                 - mov esp,ebp			<-- ending of function
	AA2Play v12 FP v1.4.0a.exe+11362E - 5D                    - pop ebp
	AA2Play v12 FP v1.4.0a.exe+11362F - C2 1000               - ret 0010 { 16 }
	*/
	DWORD address = General::GameBase + 0x111BB0;
	DWORD redirectAddress = (DWORD)(&HiPolyLoadsStartRedirect);
	Hook((BYTE*)address,
		{ 0x68, 0x06, 0x02, 0x00, 0x00 },							//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress  },	//redirect to our function
		NULL);

	address = General::GameBase + 0x11362C;
	redirectAddress = (DWORD)(&HiPolyLoadsEndRedirect);
	Hook((BYTE*)address,
		{ 0x8B, 0xE5, 
		0x5D,
		0xC2, 0x10, 0x00 },							//expected values
		{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress, 0x90 },	//redirect to our function
		NULL);
}


}
}