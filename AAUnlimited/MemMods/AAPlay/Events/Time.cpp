#include "Time.h"

#include "MemMods/Hook.h"
#include "General/ModuleInfo.h"
#include "Functions\Shared\Triggers\Event.h"
#include "Functions\Shared\TriggerEventDistributor.h"
#include "External\ExternalVariables\AAPlay\GameGlobals.h"

namespace PlayInjections {
namespace Time {


void __stdcall PeriodChangeEvent(DWORD oldPeriod) {
	Shared::Triggers::PeriodEndsData data;
	data.oldPeriod = oldPeriod;
	data.newPeriod = ExtVars::AAPlay::GameTimeData()->currentPeriod;
	Shared::Triggers::ThrowEvent(&data);
}


DWORD loc_PeriodChangeOriginalFunction;
void __declspec(naked) PeriodChangeRedirect() {
	__asm {
		mov eax,[edi+20]
		mov eax,[eax+20]
		push eax //save old period

		push [esp+4]
		call [loc_PeriodChangeOriginalFunction]
		push eax //save return value for later

		push eax
		call PeriodChangeEvent

		pop eax
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