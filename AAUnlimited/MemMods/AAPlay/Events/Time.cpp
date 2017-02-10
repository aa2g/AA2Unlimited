#include "Time.h"

#include "MemMods/Hook.h"
#include "General/ModuleInfo.h"

namespace PlayInjections {
namespace Time {






void PeriodChangeInjection() {
	//jump is taken whenever the period changes.
	//dword ptr [ecx + 0x18] is an int that signifies the period BEFORE this change (0 = sleep, 1 = day, 2 = "nothing to talk", 3 = first lesson,
	//																				4 = first break, 5 = sports, 6 = second break, 7 = club, 8 = end)
	/*
	AA2Play v12 FP v1.4.0a.exe+48005 - E8 76FFFFFF           - call "AA2Play v12 FP v1.4.0a.exe"+47F80 { ->AA2Play v12 FP v1.4.0a.exe+47F80 }
	AA2Play v12 FP v1.4.0a.exe+4800A - 84 C0                 - test al,al
	AA2Play v12 FP v1.4.0a.exe+4800C - 75 04                 - jne "AA2Play v12 FP v1.4.0a.exe"+48012 { ->AA2Play v12 FP v1.4.0a.exe+48012 }
	...
	AA2Play v12 FP v1.4.0a.exe+48012 - 8B 4F 20              - mov ecx,[edi+20]
	AA2Play v12 FP v1.4.0a.exe+48015 - BE 20030000           - mov esi,00000320 { 800 }
	AA2Play v12 FP v1.4.0a.exe+4801A - E8 D1150000           - call "AA2Play v12 FP v1.4.0a.exe"+495F0{ ->AA2Play v12 FP v1.4.0a.exe+495F0 }
	*/
	/*DWORD address = General::GameBase + 0xACDEF;
	DWORD redirectAddress = (DWORD)(&InitRedirect);
	Hook((BYTE*)address,
		{ 0xE8, 0x3C, 0x01, 0x00, 0x00 },						//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		(DWORD*)(&loc_InitOriginalFunction));*/
}

}
}

/*
	D95E0 constantly called while clothing change dialog is active
*/