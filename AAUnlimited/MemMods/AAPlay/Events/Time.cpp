#include "StdAfx.h"





namespace PlayInjections {
namespace Time {


void __stdcall PeriodChangeEvent(DWORD oldPeriod) {
	auto timedata = ExtVars::AAPlay::GameTimeData();
	LOGPRIO(Logger::Priority::SPAM) << "Period changed to " << timedata->currentPeriod << "\n";
	if (timedata->currentPeriod == 1) {
		LOGPRIO(Logger::Priority::INFO) << "Day has changed, day of week " << timedata->day << ", " << timedata->nDays << " total days.\n";
	}

	// fast-forward all the required delayed events
	auto delayedEvents = Shared::GameState::GetDelayedEvents();
	for (auto it = delayedEvents->begin(); it != delayedEvents->end();) {
		if (it->required) {
			ThrowEvent(&(*it));
			delayedEvents->erase(it++);
		}
	}

	Shared::Triggers::PeriodEndsData data;

	auto pc = Shared::GameState::getPlayerCharacter();
	if (pc->IsValid()) {
		data.card = pc->m_char->m_seat;
	}

	for (int i = 0; i < 25; i++) {
		auto character = AAPlay::g_characters[data.card];
		if (character.IsValid()) {
			character.m_char->m_charData->m_character.strengthClassRank = (character.m_char->m_charData->m_character.strengthValue - 100) / 100;
			character.m_char->m_charData->m_character.intelligenceClassRank = (character.m_char->m_charData->m_character.intelligenceValue - 100) / 100;
			character.m_char->m_charData->m_character.clubClassRanking = (character.m_char->m_charData->m_character.clubValue - 100) / 100;
		}
	}

	data.oldPeriod = oldPeriod;
	data.newPeriod = timedata->currentPeriod;
	LUA_EVENT("period", timedata->currentPeriod, oldPeriod);
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
