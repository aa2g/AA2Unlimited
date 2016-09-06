#include "EyeTexture.h"

#include "MemMods\Hook.h"
#include "General\ModuleInfo.h"
#include "Functions\Shared\Overrides.h"

namespace SharedInjections {
namespace EyeTexture {



void __declspec(naked) EyeTextureStartRedirect() {
	__asm {
		pushad
		lea eax, [esp + 0x20 + 4 + 0x18]
		push eax
		push esi
		call Shared::EyeTextureStart
		popad
		//original code
		mov ecx, [esp + esi*4 + 0x28]
		cmp ecx, -1
		ret
	}
}

DWORD EyeTextureJLExit;
void __declspec(naked) EyeTextureEndRedirect() {
	__asm {
		pushad
		lea eax, [esp + 0x20 + 4 + 0x18]
		push eax
		dec esi
		push esi
		call Shared::EyeTextureEnd
		popad
		cmp esi, 02
		jl EyeTextureEndRedirect_JLExit
		ret
	EyeTextureEndRedirect_JLExit:
		mov edx, [EyeTextureJLExit]
		mov [esp], edx
		ret
	}
}

void EyeTextureInject() {
	if (General::IsAAEdit) {
		//start, [esp+18] is eye texture
		//AA2Edit.exe + 118A16 - 33 F6 - xor esi, esi
		//AA2Edit.exe + 118A18 - 8B 4C B4 24 - mov ecx, [esp + esi * 4 + 24]{ start of eye loop }
		//AA2Edit.exe+118A1C - 83 F9 FF              - cmp ecx,-01 { 255 }
		//...
		//still [esp+18]
		//AA2Edit.exe + 118B06 - 46 - inc esi
		//AA2Edit.exe + 118B07 - 83 FE 02 - cmp esi, 02 { 2 }
		//AA2Edit.exe+118B0A - 0F8C 08FFFFFF         - jl AA2Edit.exe+118A18 { loop end }
		DWORD address = General::GameBase + 0x118A18;
		DWORD redirectAddress = (DWORD)(&EyeTextureStartRedirect);
		Hook((BYTE*)address,
			{ 0x8B, 0x4C, 0xB4, 0x24,
			  0x83, 0xF9, 0xFF},						//expected values
			{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress, 0x90, 0x90 },	//redirect to our function
			NULL);
		address = General::GameBase + 0x118B0A;
		redirectAddress = (DWORD)(&EyeTextureEndRedirect);
		Hook((BYTE*)address,
		{ 0x0F, 0x8C, 0x08, 0xFF, 0xFF, 0xFF },						//expected values
			{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress, 0x90 },	//redirect to our function
			NULL);
		EyeTextureJLExit = General::GameBase + 0x118A18;

	}
	else if (General::IsAAPlay) {
		//AA2Play v12 FP v1.4.0a.exe+12A508 - 8B 4C B4 24           - mov ecx,[esp+esi*4+24] { start if eye loop }
		//AA2Play v12 FP v1.4.0a.exe + 12A50C - 83 F9 FF - cmp ecx, -01 { 255 }
		//...
		//AA2Play v12 FP v1.4.0a.exe+12A5FA - 0F8C 08FFFFFF         - jl "AA2Play v12 FP v1.4.0a.exe"+12A508 { loop end }
		DWORD address = General::GameBase + 0x12A508;
		DWORD redirectAddress = (DWORD)(&EyeTextureStartRedirect);
		Hook((BYTE*)address,
			{ 0x8B, 0x4C, 0xB4, 0x24,
			  0x83, 0xF9, 0xFF },						//expected values
			{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress, 0x90, 0x90 },	//redirect to our function
			NULL);
		address = General::GameBase + 0x12A5FA;
		redirectAddress = (DWORD)(&EyeTextureEndRedirect);
		Hook((BYTE*)address,
		{ 0x0F, 0x8C, 0x08, 0xFF, 0xFF, 0xFF },						//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress, 0x90 },	//redirect to our function
			NULL);
		EyeTextureJLExit = General::GameBase + 0x12A508;
	}
}

	
}
}