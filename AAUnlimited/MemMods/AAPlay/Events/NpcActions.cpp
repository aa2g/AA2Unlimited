#include "StdAfx.h"

#define NEW_HOOK 1
#define REGRESSIONS 1

namespace PlayInjections {
namespace NpcActions {

using namespace ExtClass;

#if NEW_HOOK
DWORD AnswerAddress, AnswerLowAddress;
DWORD* LowPolyInjectionReturnAddress = 0;
DWORD* SomeVanillaAddress = 0;
void __stdcall Answer(AnswerStruct*);
BYTE __stdcall AnswerLow(AnswerStruct*);

// Wraps a call to our custom handler, redirect jump is pointed into here
void __declspec(naked) WrapAnswer() {
	__asm {
		pushad
		push esi
		call Answer
		popad
		mov al, 1
		ret
	}
}

// This calls the original answer function we patched
void CallAnswer(AnswerStruct *as) {
	__asm {
		mov esi, as
		pushad
		call goback
		jmp skip
goback:
		// part of function we destroyed
		push ecx
		mov  eax, [esi + 440]
		// Original function
		jmp [AnswerAddress]
skip:
		popad
	}
}

// Wraps a call to our custom handler, redirect jump is pointed into here
void __declspec(naked) WrapAnswerLow() {
	__asm {
		pushad
		push edx
		call AnswerLow
		mov[esp + 28], eax
		popad
		ret
	}
}

// This calls the original answer (low chance) function we patched. It returns bool we need.
BYTE CallAnswerLow(AnswerStruct *as) {
	BYTE retv;
	__asm {
		mov edx, as
		pushad
		call goback
		jmp skip
goback:
		// part of function we destroyed
		push 0xffffffff
		// Original function
		jmp [AnswerLowAddress]
skip:
		mov[esp + 28], al
		popad
		mov retv, al
	}
	return retv;
}
///////////////////////////////////////////////

	using namespace Shared::Triggers;
	using namespace AAPlay;


// When this routine is called, the roll result (0 or 1) is in as->answer
// and roll number in ->answerChar->m_lastConversationAnswerPercent.
// Now if you want to change the outcome of a roll, do it BEFORE CallAnswer - that one
// acts on the actual data of the answer.
//
// Acting on it after will change dialog result, but scores will be applied by CallAnswer
// with the original unmodified answer data.
//
void __stdcall Answer(AnswerStruct *as) {

	LUA_EVENT("answer", as->answer, as);


	NpcResponseData data;
	data.card = GetSeatFromStruct(as->answerChar->m_thisChar);
	data.absoluteChance = -1;
	data.absoluteResponse = -1;
	data.strongChance = -1;
	data.strongResponse = -1;
	data.answeredTowards = GetSeatFromStruct(as->askingChar->m_thisChar);
	data.originalResponse = as->answer;
	data.changedResponse = as->answer;
	data.conversationId = as->askingChar->m_currConversationId; //this id is set to -1 for the answerChar in case of minna
	data.originalChance = as->answerChar->m_lastConversationAnswerPercent;
	data.changedChance = data.originalChance;
	data.substruct = as;
	ThrowEvent(&data);

	// we can't just carry over because the data is DWORD (with potential deaf=2 state), not bool.
	// plaster over it only if something actually wants it changed.
	as->answer = data.changedResponse;
	as->answerChar->m_lastConversationAnswerPercent = data.changedChance;
	
	if (data.strongResponse != -1) {
		as->answer = data.strongResponse;
		if (data.strongChance != -1) {
			as->answerChar->m_lastConversationAnswerPercent = data.strongChance;
		}
	}

	if (data.absoluteResponse != -1) {
		as->answer = data.absoluteResponse;
		if (data.absoluteChance != -1) {
			as->answerChar->m_lastConversationAnswerPercent = data.absoluteChance;
		}
	}

	CallAnswer(as);
	LUA_EVENT("answer_after", as->answer, as);
	NPCAfterResponseData afterResponseData;
	afterResponseData.card = GetSeatFromStruct(as->answerChar->m_thisChar);
	afterResponseData.answeredTowards = GetSeatFromStruct(as->askingChar->m_thisChar);
	afterResponseData.conversationId = as->askingChar->m_currConversationId;
	afterResponseData.substruct = as;
	afterResponseData.effectiveChance = as->answerChar->m_lastConversationAnswerPercent;
	afterResponseData.effectiveResponse = as->answer;
	ThrowEvent(&afterResponseData);

}

// Some speculation what this does:
// This routine is called to apply modifiers for the case when roll below chance of 100% succeeds.
//
// Numerous mood (?) biases are applied in that case and can override the answer back to failure or
// unknown state 2. It returns answer value, which is shoveled into as->answer by the caller, but in some cases
// offset by 1 (!), thus final as->answer becomes 2 or 3, not 1.
// In other cases, it is implicitly assumed success (since this branch is success rolls).
//
// Answer() is called just right after this, picks up value from as->answer and proapgates further into
// character structs. If AnswerLow() function isn't called, chances were either above 100
// (and as->answer later is implicit 1), or the roll didn't succeed in which case as->answer became
// implicit 0, in both cases Answer() has a chance to change the outcome.
//
BYTE __stdcall AnswerLow(AnswerStruct *as) {
	int override = -1;
#if REGRESSIONS
	LUA_EVENT("answer_lucky", override, as);
	if (override != -1)
		return override;
#endif
	return CallAnswerLow(as);
}


void NpcAnswerInjection() {

	// Answer
	AnswerAddress = General::GameBase + 0x188A85;
	Hook((BYTE*)AnswerAddress-5,
	{ 0x51, 0x8b, 0x86, 0xb8, 0x01, 0x00, 0x00},
	{ 0xE9, HookControl::RELATIVE_DWORD, (DWORD)&WrapAnswer, 0x90, 0x90}, NULL);

	// AnswerLow
	AnswerLowAddress = General::GameBase + 0x18F042;
	Hook((BYTE*)AnswerLowAddress - 7,
	{ 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x6a, 0xff },
	{ 0xE9, HookControl::RELATIVE_DWORD, (DWORD)&WrapAnswerLow, 0xeb, 0xf9 }, NULL);
}

#endif
	
///////////////////////////////////////////////


BYTE __stdcall ClothesChangedEvent(ExtClass::CharacterStruct* npc, BYTE newClothes) {
	LUA_EVENT("clothes", newClothes, npc->m_seat);
	return newClothes;
}

void __declspec(naked) ClothesChangedRedirect() {
	__asm {
		mov ecx, [edx+0xE0]
		push ecx
		push edx

		push eax
		push ecx
		call ClothesChangedEvent

		pop edx
		pop ecx
		ret
	}
}

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
	DWORD address = General::GameBase + 0x16F0D1;
	DWORD redirectAddress = (DWORD)(&ClothesChangedRedirect);
	Hook((BYTE*)address,
		{ 0x8B, 0x8A, 0xE0, 0x00, 0x00, 0x00 },								//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress, 0x90 },	//redirect to our function
		NULL);
}

void __stdcall RoomChange(NpcData* param) {
	for (int seat = 0; seat < 25; seat = seat + 1) {
		CharInstData* instance = &AAPlay::g_characters[seat];
		if (instance->IsValid()) {
			if (instance->m_char->m_npcData == param) {
				Shared::Triggers::RoomChangeData roomChangeData;
				roomChangeData.action = instance->m_forceAction.conversationId;
				roomChangeData.roomTarget = instance->m_forceAction.roomTarget;
				if (instance->m_forceAction.target1 != nullptr) {
					roomChangeData.convotarget = int(instance->m_forceAction.target1->m_thisChar);
				}

				roomChangeData.card = instance->m_char->m_seat;
				Shared::Triggers::ThrowEvent(&roomChangeData);
				return;
			}
		}
	}
}


void __declspec(naked) roomChangeRedirect() {
	__asm {
		pushad
		push edi
		call RoomChange
		popad
		//original code
		mov eax, [edi]
		mov ecx, [eax+04]
		ret
	}
}


void RoomChangeInjection() {
	//edi is the m_npcData
	//AA2Play.exe +174CF4 - 8B 07 - mov eax,[edi]
	//AA2Play.exe + 174CF6 - 8B 48 04 - mov ecx, [eax + 04]

	DWORD address = General::GameBase + 0x174CF4;
	DWORD redirectAddress = (DWORD)(&roomChangeRedirect);
	Hook((BYTE*)address,
	{ 0x8B, 0x07,
		0x8B, 0x48, 0x04 },						//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		NULL);
}

void __stdcall LowPolyUpdateStart(CharacterStruct* param) {
	//This is the common function for boys and girls
	if (ExtVars::AAPlay::GameTimeData()) {
		if (ExtVars::AAPlay::GameTimeData()->currentPeriod >= 0) {
			Shared::g_currentChar = &AAPlay::g_characters[param->m_seat];
			Shared::GameState::setIsOverriding(true);
		}
	}
}

void __stdcall LowPolyUpdateStartForGirls(CharacterStruct* param) {
	//calling the common function
	LowPolyUpdateStart(param);

	//Setting the address to return to after the hook
	const DWORD offset[]{ 0x116812 };
	LowPolyInjectionReturnAddress = (DWORD*)ExtVars::ApplyRule(offset);
}

void __declspec(naked) LowPolyUpdateStartRedirectForGirls() {
	__asm {
		pushad
		push ebx
		call LowPolyUpdateStartForGirls
		popad
		//original code
		mov eax, [ebp + 0x0C]
		mov edx, [edx + 0x4C]
		jmp LowPolyInjectionReturnAddress
	}
}


void LowPolyUpdateStartInjectForGirls() {
	//AA2Play.exe + 11680C - 8B 45 0C - mov eax, [ebp + 0C]
	//AA2Play.exe + 11680F - 8B 52 4C - mov edx, [edx + 4C]
	//AA2Play.exe + 116812 - 50 - push eax
	//AA2Play.exe + 116813 - 8B CB - mov ecx, ebx
	//AA2Play.exe + 116815 - FF D2 - call edx
	//AA2Play.exe + 116817 - 8A 43 44 - mov al, [ebx + 44]


	DWORD address = General::GameBase + 0x11680C;
	DWORD redirectAddress = (DWORD)(&LowPolyUpdateStartRedirectForGirls);
	Hook((BYTE*)address,
	{ 0x8B, 0x45, 0x0C,
		0x8B, 0x52, 0x4C },						//expected values
		{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress, 0x90 },	//redirect to our function, unlike hackanon's code it's jumping, not calling so it doesn't mess up the stack
		NULL);
}


void __stdcall LowPolyUpdateStartForBoys(CharacterStruct* param) {
	//calling the common function
	LowPolyUpdateStart(param);

	//Setting the address to return to after the hook
	const DWORD offset[]{ 0x10DF3C };
	LowPolyInjectionReturnAddress = (DWORD*)ExtVars::ApplyRule(offset);
}

void __declspec(naked) LowPolyUpdateStartRedirectForBoys() {
	__asm {
		pushad
		push ebx
		call LowPolyUpdateStartForBoys
		popad
		//original code
		mov eax, [ebx]
		mov edx, [eax + 0x4C]
		jmp LowPolyInjectionReturnAddress
	}
}


void LowPolyUpdateStartInjectForBoys() {
	//AA2Play.exe + 10DF37 - 8B 03 - mov eax, [ebx]
	//AA2Play.exe + 10DF39 - 8B 50 4C - mov edx, [eax + 4C]
	//AA2Play.exe + 10DF3C - 51 - push ecx
	//AA2Play.exe + 10DF3D - 8B CB - mov ecx, ebx
	//AA2Play.exe + 10DF3F - FF D2 - call edx
	//AA2Play.exe + 10DF41 - 83 3E 02 - cmp dword ptr[esi], 02 { 2 }


	DWORD address = General::GameBase + 0x10DF37;
	DWORD redirectAddress = (DWORD)(&LowPolyUpdateStartRedirectForBoys);
	Hook((BYTE*)address,
	{ 0x8B, 0x03,
		0x8B, 0x50, 0x4C },						//expected values
		{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function, unlike hackanon's code it's jumping, not calling so it doesn't mess up the stack
		NULL);
}



void __stdcall LowPolyUpdateEnd() {
	//If you are wondering which character finished loading, just push the character from LowPolyUpdateStart into GameState. The character that started updating is the one that finished updating.
	Shared::GameState::setIsOverriding(false);

	//Setting the address to return to after the hook
	const DWORD offset[]{ 0x14A3C3 }; //There are vanilla common functions, but it's hard to find any that are called before files are opened
	LowPolyInjectionReturnAddress = (DWORD*)ExtVars::ApplyRule(offset);
}

void __declspec(naked) LowPolyUpdateEndRedirect() {
	__asm {
		pushad
		call LowPolyUpdateEnd
		popad
		//original code
		pop ecx
		pop edi
		pop esi
		pop ebp
		pop ebx
		jmp LowPolyInjectionReturnAddress
	}
}


void LowPolyUpdateEndInject() {
	
	//AA2Play.exe + 14A3BE - 59 - pop ecx
	//AA2Play.exe + 14A3BF - 5F - pop edi
	//AA2Play.exe + 14A3C0 - 5E - pop esi
	//AA2Play.exe + 14A3C1 - 5D - pop ebp
	//AA2Play.exe + 14A3C2 - 5B - pop ebx
	//AA2Play.exe + 14A3C3 - 83 C4 18 - add esp, 18 { 24 } //return point
	//AA2Play.exe + 14A3C6 - C2 0800 - ret 0008 { 00000008 }


	DWORD address = General::GameBase + 0x14A3BE;
	DWORD redirectAddress = (DWORD)(&LowPolyUpdateEndRedirect);
	Hook((BYTE*)address,
	{ 0x59, 
		0x5F, 
		0x5E, 
		0x5D, 
		0x5B },						//expected values
		{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		NULL);
}


void __stdcall hPositionChange(BYTE param) {
	const DWORD offsetdom[]{ 0x3761CC, 0x28, 0x38, 0xe0, 0x6c, 0xe0, 0x00, 0x3c };
	DWORD* actor0 = (DWORD*)ExtVars::ApplyRule(offsetdom);

	const DWORD offsetsub[]{ 0x3761CC, 0x28, 0x38, 0xe0, 0x6c, 0xe4, 0x00, 0x3c };
	DWORD* actor1 = (DWORD*)ExtVars::ApplyRule(offsetsub);

	if (actor0 && actor1) {
		Shared::Triggers::HPositionData hPositionData;
		hPositionData.card = Shared::GameState::getPlayerCharacter()->m_char->m_seat;
		hPositionData.actor0 = *actor0;
		hPositionData.actor1 = *actor1;
		auto hInfo = Shared::GameState::getHInfo();
		hPositionData.dominantParticipant = hInfo->m_activeParticipant->m_charPtr->m_seat;
		hPositionData.submissiveParticipant = hInfo->m_passiveParticipant->m_charPtr->m_seat;
		hPositionData.position = param;
		Shared::Triggers::ThrowEvent(&hPositionData);
	}
}


void __declspec(naked) hPositionChangeRedirect() {
	__asm {
		pushad
		push esi
		call hPositionChange
		popad
		//original code
		mov [edi + 0x000005F4], esi
		ret
	}
}

void hPositionChangeInjection() {
	//H Position ID is in esi
	//AA2Play.exe + 8197F - 89 B7 F4050000 - mov[edi + 000005F4], esi


	DWORD address = General::GameBase + 0x8197F;
	DWORD redirectAddress = (DWORD)(&hPositionChangeRedirect);
	Hook((BYTE*)address,
	{ 0x89, 0xB7, 0xF4, 0x05, 0x00, 0x00 },						//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress, 0x90 },	//redirect to our function
		NULL);
}




#if !NEW_HOOK
DWORD __stdcall NpcAnswerEvent2(bool result, CharacterActivity* answerChar, CharacterActivity* askingChar)
{
	LUA_EVENT("answer", result, answerChar, askingChar);
	return result;
}

int __stdcall NpcAnswerEvent(CharacterActivity* answerChar, CharacterActivity* askingChar, void* unknownStruct, DWORD unknownParameter, int originalReturn) {
	using namespace Shared::Triggers;
	using namespace AAPlay;
	NpcResponseData data;
	data.card = GetSeatFromStruct(answerChar->m_thisChar);
	data.answeredTowards = GetSeatFromStruct(askingChar->m_thisChar); 
	data.originalResponse = originalReturn;
	data.changedResponse = data.originalResponse;
	data.conversationId = askingChar->m_currConversationId; //this id is not set for the answerChar in case of minna
	data.originalChance = answerChar->m_lastConversationAnswerPercent;
	data.changedChance = data.originalChance;
	ThrowEvent(&data);
	originalReturn = data.changedResponse; //after potential modifications in triggers, percentage and response goes back to answerChar Activity
	answerChar->m_lastConversationAnswerPercent = data.changedChance;

	LUA_EVENT("answer", data.changedResponse, answerChar, askingChar);
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

DWORD orig_answer2;
DWORD orig_answer3;

void __declspec(naked) NpcAnswerRedirect2() {
	__asm {
		pushad
		push[eax + 444]
		push[eax + 440]
		call[orig_answer2]
		push eax
		call NpcAnswerEvent2
		mov[esp + 28], eax
		popad
		ret
	}
}

void __declspec(naked) NpcAnswerRedirect3() {
	__asm {
		pushad
		push[eax + 444]
		push[eax + 440]
		call[orig_answer3]
		push eax
		call NpcAnswerEvent2
		mov[esp + 28], eax
		popad
		ret
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
		{ 0xE8, HookControl::ANY_DWORD },								//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		NULL);

	address = General::GameBase + 0x19FAB4;
	Hook((BYTE*)address,
		{ 0xE8, HookControl::ANY_DWORD },								//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		&loc_NpcAnswerOriginalFunction);

	


	orig_answer2 = General::GameBase + 0x187FB0;
	
	address = General::GameBase + 0x165A52;
	Hook((BYTE*)address,
	{ 0xE8, 0x59, 0x25, 0x02, 0x00 },								//expected values
	{ 0xE8, HookControl::RELATIVE_DWORD, (DWORD)&NpcAnswerRedirect2},	//redirect to our function
		NULL);



	
	orig_answer3 = General::GameBase + 0x187EA0;

	address = General::GameBase + 0xBA687;
	Hook((BYTE*)address,
	{ 0xE8, 0x14, 0xD8, 0x0C, 0x00 },								//expected values
	{ 0xE8, HookControl::RELATIVE_DWORD, (DWORD)&NpcAnswerRedirect3 },	//redirect to our function
		NULL);
	address = General::GameBase + 0x165BF5;
	Hook((BYTE*)address,
	{ 0xE8, 0xA6, 0x22, 0x02, 0x00 },								//expected values
	{ 0xE8, HookControl::RELATIVE_DWORD, (DWORD)&NpcAnswerRedirect3 },	//redirect to our function
		NULL);

}

#endif

void __stdcall NpcMovingActionEvent(void* moreUnknownData, ExtClass::ActionParamStruct* params) {

	ExtClass::CharacterStruct* user = NULL;
	for(int i = 0; i < 25; i++) {
		ExtClass::CharacterStruct* it = AAPlay::g_characters[i].m_char;
		if(it != NULL) {
			if(it->m_moreUnknownData == moreUnknownData) {
				user = it;
				break;
			}
		}
	}
	if (user == NULL) return;

	LUA_EVENT_NORET("move", params);

	using namespace Shared::Triggers;

	switch(params->movementType) {
	case 2: {
		//walk somewhere
		NpcWalkToRoomData data;
		data.substruct = params;
		data.card = AAPlay::GetSeatFromStruct(user);
		data.targetRoom = params->roomTarget;
		ThrowEvent(&data);
		break; }
	case 3: {
		//talk to someone
		if(params->target1 == NULL) {
			NpcWantActionNoTargetData data;
			data.substruct = params;
			data.card = AAPlay::GetSeatFromStruct(user);
			data.action = params->conversationId;
			ThrowEvent(&data);
		}
		else if(params->target2 == NULL) {
			NpcWantTalkWithData data;
			data.substruct = params;
			data.card = AAPlay::GetSeatFromStruct(user);
			data.action = params->conversationId;
			data.conversationTarget = AAPlay::GetSeatFromStruct(params->target1->m_thisChar);
			ThrowEvent(&data);
		}
		else {
			NpcWantTalkWithAboutData data;
			data.substruct = params;
			data.card = AAPlay::GetSeatFromStruct(user);
			data.action = params->conversationId;
			data.conversationTarget = AAPlay::GetSeatFromStruct(params->target1->m_thisChar);
			data.conversationAbout = AAPlay::GetSeatFromStruct(params->target2->m_thisChar);
			ThrowEvent(&data);
		}
		break; }
	default:
		break;
	}


}

DWORD loc_NpcMovingActionFuncion;
void __declspec(naked) NpcMovingActionRedirect() {
	__asm {
		pushad
		push edi
		push esi
		call NpcMovingActionEvent
		popad
		call [loc_NpcMovingActionFuncion]
		ret
	}
}

void NpcMovingActionInjection() {
	//when an npc decides to do something that causes him to walks
	//esi is pointer to a specific struct from the acting person, the struct at offset F60
	//not sure what esi is, but edi points to the new conversation type (as well as other information)
	//+C is where esi points and is the conversation id
	//+10 seems to be walk type (2=talk, 3=walk etc)
	//+1C is a pointer to a charachterStructs activity data, as is 20 (similarities to NpcAnswer creep up) (these are NULL sometimes, tho)
	/*AA2Play v12 FP v1.4.0a.exe+117D25 - 8D 7C 24 0C           - lea edi,[esp+0C]
	AA2Play v12 FP v1.4.0a.exe+117D29 - 8B F3                 - mov esi,ebx
	AA2Play v12 FP v1.4.0a.exe+117D2B - E8 10000000           - call "AA2Play v12 FP v1.4.0a.exe"+117D40{ ->AA2Play v12 FP v1.4.0a.exe+117D40 }*/

	DWORD address = General::GameBase + 0x117D2B;
	DWORD redirectAddress = (DWORD)(&NpcMovingActionRedirect);
	Hook((BYTE*)address,
		{ 0xE8, 0x10, 0x00, 0x00, 0x00 },							//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },		//redirect to our function
		(DWORD*)(&loc_NpcMovingActionFuncion));						//save old function to call in our redirect function
}

bool __stdcall NpcMovingActionPlanEvent(void* unknownStruct, ExtClass::ActionParamStruct* params, bool success) {
	ExtClass::CharacterStruct* user = NULL;
	for (int i = 0; i < 25; i++) {
		ExtClass::CharacterStruct* it = AAPlay::g_characters[i].m_char;
		if (it != NULL) {
			if (it->m_moreUnknownData != NULL) {
				if(*(void**)((BYTE*)(it->m_moreUnknownData) + 0x1C) == unknownStruct) {
					user = it;
					break;
				}
			}
		}
	}

	LUA_EVENT("plan", success, params, user);

	//where unknownStruct is [m_moreUnknownData + 0x1C]

	// an action has been planned succesfuly
	if (success) return success;

	// no event planned this time round (or cancelled by lua), inject our own forced action

	if (user == NULL) return success;

	//apply a forced action if queued
	auto* inst = AAPlay::GetInstFromStruct(user);
	if(inst->m_forceAction.conversationId != -1) {
		//got a forced struct
		*params = inst->m_forceAction;
		inst->m_forceAction.conversationId = -1;
		return true;
	}
	return false;
}

DWORD loc_NpcMovingActionPlan;
void __declspec(naked) NpcMovingActionPlanRedirect() {
	__asm {
		call [loc_NpcMovingActionPlan]
		push eax
		push edi
		push esi
		call NpcMovingActionPlanEvent
		ret
	}
}

void NpcMovingActionPlanInjection() {
	//no stack param, esi = some struct (unknown data + 1C), edi = param struct, same as above
	//AA2Play v12 FP v1.4.0a.exe+117CFE - E8 ED8B0700           - call "AA2Play v12 FP v1.4.0a.exe"+1908F0 { ->AA2Play v12 FP v1.4.0a.exe+1908F0 }
	//return al == 1 and fills params if an action was performed
	DWORD address = General::GameBase + 0x117CFE;
	DWORD redirectAddress = (DWORD)(&NpcMovingActionPlanRedirect);
	Hook((BYTE*)address,
		{ 0xE8, 0xED, 0x8B, 0x07, 0x00 },							//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },		//redirect to our function
		(DWORD*)(&loc_NpcMovingActionPlan));
}

}
}




//some sort of npc ai tick
//AA2Play v12 FP v1.4.0a.exe+16E210 - A1 CC610001           - mov eax,["AA2Play v12 FP v1.4.0a.exe"+3761CC] { [13D02008] }


//changes npc walking type to "talk to someone"
//AA2Play v12 FP v1.4.0a.exe+172089 - C7 40 3C 03000000     - mov [eax+3C],00000003 { 3 }

//16F0D7 changes clothes for npcs