#include "Time.h"

#include "MemMods/Hook.h"
#include "General/ModuleInfo.h"
#include "Functions\Shared\Triggers\Event.h"
#include "Functions\Shared\TriggerEventDistributor.h"
#include "External\ExternalVariables\AAPlay\GameGlobals.h"
#include "Functions\AAPlay\Globals.h"
#include "Functions/AAPlay/GameState.h"

namespace PlayInjections {
namespace Time {


void __stdcall PeriodChangeEvent(DWORD oldPeriod) {
	Shared::Triggers::PeriodEndsData data;
	do {
		//assigns a random filled seat as the triggering card
		//or use the current PC instead
		data.card = rand() % 25;
	} while (!AAPlay::g_characters[data.card].IsValid());

	data.oldPeriod = oldPeriod;
	data.newPeriod = ExtVars::AAPlay::GameTimeData()->currentPeriod;
	g_Lua[LUA_BINDING_TABLE]["Time"]("PeriodEnds", oldPeriod, data.newPeriod);
	Shared::GameState::setIsOverriding(false);
	Shared::Triggers::ThrowEvent(&data);
}


DWORD loc_PeriodChangeOriginalFunction;
void __declspec(naked) PeriodChangeRedirect() {
	__asm {
		mov eax,[edi+0x20]
		mov eax,[eax+0x20]
		push eax //save old period

		push [esp+8] //push original function argument
		call [loc_PeriodChangeOriginalFunction]
		push eax //save return value for later

		push [esp+4]
		call PeriodChangeEvent

		pop eax //set return value
		add esp,4 //remove old period from stack
		ret 4
	}
}


void PeriodChangeInjection() {
	//jump is taken whenever the period changes.
	//eax (edi+20) is the global time data struct; the call at the bottom is changing the value in it.
	//stdcall 1 stack param, returns a bool, no register params
	/*
	AA2Play v12 FP v1.4.0a.exe+48005 - E8 76FFFFFF           - call "AA2Play v12 FP v1.4.0a.exe"+47F80 { ->AA2Play v12 FP v1.4.0a.exe+47F80 }
	AA2Play v12 FP v1.4.0a.exe+4800A - 84 C0                 - test al,al
	AA2Play v12 FP v1.4.0a.exe+4800C - 75 04                 - jne "AA2Play v12 FP v1.4.0a.exe"+48012 { ->AA2Play v12 FP v1.4.0a.exe+48012 }
	...
	AA2Play v12 FP v1.4.0a.exe+4801F - 8B 47 20              - mov eax,[edi+20]
	AA2Play v12 FP v1.4.0a.exe+48022 - 33 DB                 - xor ebx,ebx
	AA2Play v12 FP v1.4.0a.exe+48024 - 50                    - push eax
	AA2Play v12 FP v1.4.0a.exe+48025 - 88 5F 39              - mov [edi+39],bl
	AA2Play v12 FP v1.4.0a.exe+48028 - 89 5F 50              - mov [edi+50],ebx
	AA2Play v12 FP v1.4.0a.exe+4802B - E8 B0930A00           - call "AA2Play v12 FP v1.4.0a.exe"+F13E0 { ->AA2Play v12 FP v1.4.0a.exe+F13E0 }
	*/
	DWORD address = General::GameBase + 0x4802B;
	DWORD redirectAddress = (DWORD)(&PeriodChangeRedirect);
	Hook((BYTE*)address,
		{ 0xE8, 0xB0, 0x93, 0x0A, 0x00 },						//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		(DWORD*)(&loc_PeriodChangeOriginalFunction));
}

}
}

/*
	D95E0 constantly called while clothing change dialog is active
*/