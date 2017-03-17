#include "NpcActions.h"

#include "MemMods/Hook.h"
#include "General/ModuleInfo.h"

#include "External\ExternalClasses\CharacterStruct.h"
#include "Functions\Shared\TriggerEventDistributor.h"
#include "Functions\AAPlay\Globals.h"

namespace PlayInjections {
namespace NpcActions {


using namespace ExtClass;

BYTE __stdcall ClothesChangedEvent(ExtClass::CharacterStruct* npc, BYTE newClothes) {
	return newClothes;
}

void __declspec(naked) ClothesChangedRedirect() {
	__asm {
		mov ecx, [edx+0xE0]
		push ecx
		push edx

		push eax
		push ecx
		call ClothesChangedRedirect

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

void __stdcall NpcMovingActionEvent(void* moreUnknownData, CharInstData::ActionParamStruct* params) {

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

	using namespace Shared::Triggers;

	switch(params->movementType) {
	case 2: {
		//walk somewhere
		NpcWalkToRoomData data;
		data.card = AAPlay::GetSeatFromStruct(user);
		data.targetRoom = params->roomTarget;
		ThrowEvent(&data);
		break; }
	case 3: {
		//talk to someone
		if(params->target1 == NULL) {
			NpcWantActionNoTargetData data;
			data.card = AAPlay::GetSeatFromStruct(user);
			data.action = params->conversationId;
			ThrowEvent(&data);
		}
		else if(params->target2 == NULL) {
			NpcWantTalkWithData data;
			data.card = AAPlay::GetSeatFromStruct(user);
			data.action = params->conversationId;
			data.conversationTarget = AAPlay::GetSeatFromStruct(params->target1->m_thisChar);
			ThrowEvent(&data);
		}
		else {
			NpcWantTalkWithAboutData data;
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

bool __stdcall NpcMovingActionPlanEvent(void* unknownStruct,CharInstData::ActionParamStruct* params, bool success) {
	//where unknownStruct is [m_moreUnknownData + 0x1C]
	if (success) return success;

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