#include "NpcActions.h"

#include "MemMods/Hook.h"
#include "General/ModuleInfo.h"

#include "External\ExternalClasses\CharacterStruct.h"
#include "Functions\Shared\TriggerEventDistributor.h"
#include "Functions\AAPlay\Globals.h"

namespace PlayInjections {
namespace NpcActions {


using namespace ExtClass;

void ClothesChangeInjection() {
	//the last line sets the clothes
	/*AA2Play v12 FP v1.4.0a.exe+16F0BD - E8 CED4F7FF           - call "AA2Play v12 FP v1.4.0a.exe"+EC590 { ->AA2Play v12 FP v1.4.0a.exe+EC590 }
	AA2Play v12 FP v1.4.0a.exe+16F0C2 - 83 F8 FF              - cmp eax,-01 { 255 }
	AA2Play v12 FP v1.4.0a.exe+16F0C5 - 74 48                 - je "AA2Play v12 FP v1.4.0a.exe"+16F10F { ->AA2Play v12 FP v1.4.0a.exe+16F10F }
	AA2Play v12 FP v1.4.0a.exe+16F0C7 - 8B 55 04              - mov edx,[ebp+04]
	AA2Play v12 FP v1.4.0a.exe+16F0CA - 8B 4A 04              - mov ecx,[edx+04]
	AA2Play v12 FP v1.4.0a.exe+16F0CD - 8B 54 29 2C           - mov edx,[ecx+ebp+2C]
	AA2Play v12 FP v1.4.0a.exe+16F0D1 - 8B 8A E0000000        - mov ecx,[edx+000000E0]
	AA2Play v12 FP v1.4.0a.exe+16F0D7 - 88 41 44              - mov [ecx+44],al
	*/

}

int __stdcall NpcAnswerEvent(CharacterActivity* answerChar, CharacterActivity* askingChar, void* unknownStruct, DWORD unknownParameter, int originalReturn) {
	using namespace Shared::Triggers;
	using namespace AAPlay;
	NpcResponseData data;
	data.card = GetSeatFromStruct(askingChar->m_thisChar);
	data.answeredTowards = GetSeatFromStruct(answerChar->m_thisChar);
	data.originalResponse = originalReturn;
	data.changedResponse = data.originalResponse;
	data.conversationId = answerChar->m_currConversationId;
	ThrowEvent(&data);
	return data.changedResponse;
}

DWORD loc_NpcAnswerOriginalFunction;
void __declspec(naked) NpcAnswerRedirect() {
	__asm {
		push eax
		push ecx //save these 2 parameters for later

		push [esp+8 + 8]
		push [esp+8 + 8]
		call [loc_NpcAnswerOriginalFunction]

		push eax //original return
		mov eax, [esp+8] //get original parameters back
		mov ecx, [esp+4]
		push [esp+4+8+8] //the dword param (4 from eax param, 8 from saved eax and ecx, 8 for actual parameter)
		push [esp+4+8+8]
		push ecx
		push eax
		call NpcAnswerEvent

		add esp, 8 //remoce saved parameters

		ret 8
	}
	
}

void NpcAnswerInjection() {
	/*
	AA2Play v12 FP v1.4.0a.exe+3C7CC - E8 2FB41400           - call "AA2Play v12 FP v1.4.0a.exe"+187C00{ ->AA2Play v12 FP v1.4.0a.exe+187C00 }
	AA2Play v12 FP v1.4.0a.exe+5B3DB - E8 20C81200           - call "AA2Play v12 FP v1.4.0a.exe"+187C00{ ->AA2Play v12 FP v1.4.0a.exe+187C00 }
	AA2Play v12 FP v1.4.0a.exe+19FAB4 - E8 4781FEFF           - call "AA2Play v12 FP v1.4.0a.exe"+187C00{ ->AA2Play v12 FP v1.4.0a.exe+187C00 }*/

	//these 3 places call this function.
	//2 stack params, stdcall. returns bool for answer
	/*AA2Play v12 FP v1.4.0a.exe+187C00 - 55                    - push ebp{ eax = npcdata1, ecx = npcdata2 }*/
	DWORD address = General::GameBase + 0x3C7CC;
	DWORD redirectAddress = (DWORD)(&NpcAnswerRedirect);
	Hook((BYTE*)address,
		{ 0xE8, 0x2F, 0xB4, 0x14, 0x00 },								//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		NULL);

	address = General::GameBase + 0x5B3DB;
	Hook((BYTE*)address,
		{ 0xE8, 0x20, 0xC7, 0x12, 0x00 },								//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		NULL);

	address = General::GameBase + 0x19FAB4;
	Hook((BYTE*)address,
		{ 0xE8, 0x47, 0x81, 0xFE, 0xFF },								//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		&loc_NpcAnswerOriginalFunction);
}

}
}



//16F0D7 changes clothes for npcs