#include "StdAfx.h"

namespace PlayInjections {
namespace ClothingDialog {

void InitEvent() {
	Shared::GameState::setIsClothesScreen(true);
}

void ExitEvent() {
	Shared::GameState::setIsClothesScreen(false);
}

DWORD loc_InitOriginalFunction;
void __declspec(naked) InitRedirect() {
	__asm {
		pushad
		call InitEvent
		popad
		push [esp+4]
		call [loc_InitOriginalFunction]
		ret 4
	}
}

void InitInjection() {
	//this part is called very early. cant find the character from here tho
	/*AA2Play v12 FP v1.4.0a.exe+ACDEE - 50                    - push eax
	AA2Play v12 FP v1.4.0a.exe+ACDEF - E8 3C010000           - call "AA2Play v12 FP v1.4.0a.exe"+ACF30 { ->AA2Play v12 FP v1.4.0a.exe+ACF30 }
	AA2Play v12 FP v1.4.0a.exe+ACDF4 - 84 C0                 - test al,al
	*/
	DWORD address = General::GameBase + 0xACDEF;
	DWORD redirectAddress = (DWORD)(&InitRedirect);
	Hook((BYTE*)address,
		{ 0xE8, 0x3C, 0x01, 0x00, 0x00 },						//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		(DWORD*)(&loc_InitOriginalFunction));
}


DWORD loc_ExitOriginalFunction;
void __declspec(naked) ExitRedirect() {
	__asm {
		pushad
		call ExitEvent
		popad
		mov dword ptr [esi+0xE288], 0
		ret
	}
}

void ExitInjection() {
	//end the cloting dialog
	/*AA2Play v12 FP v1.4.0a.exe+ACE1F - 8B 01                 - mov eax,[ecx]
	AA2Play v12 FP v1.4.0a.exe+ACE21 - 8B 10                 - mov edx,[eax]
	AA2Play v12 FP v1.4.0a.exe+ACE23 - 6A 01                 - push 01 { 1 }
	AA2Play v12 FP v1.4.0a.exe+ACE25 - FF D2                 - call edx
	AA2Play v12 FP v1.4.0a.exe+ACE27 - C7 86 88E20000 00000000 - mov [esi+0000E288],00000000 { 0 }
	*/
	DWORD address = General::GameBase + 0xACE27;
	DWORD redirectAddress = (DWORD)(&ExitRedirect);
	Hook((BYTE*)address,
		{ 0xC7, 0x86, 0x88,0xE2,0x00,0x00, 00,00,00,00 },						//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress, 0x90,0x90,0x90,0x90,0x90 },	//redirect to our function
		(DWORD*)(&loc_ExitOriginalFunction));
}

}
}