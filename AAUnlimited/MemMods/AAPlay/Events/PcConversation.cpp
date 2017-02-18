#include "PcConversation.h"
#include "MemMods/Hook.h"
#include "General/ModuleInfo.h"
#include "External/ExternalClasses.h"
#include "Functions/AAPlay/HAi.h"
#include "Functions/AAPlay/Poser.h"
#include "Functions/AAPlay/GameState.h"

namespace PlayInjections {
/*
 * Events that are thrown when a npc talks to a pc.
 */
namespace PcConversation {

/********************
 * Start / End Event
 ********************/

void __stdcall StartEvent() {
	Shared::GameState::setIsPcConversation(true);
	Poser::StartEvent(Poser::NpcInteraction);
}

void __stdcall EndEvent() {
	Shared::GameState::setIsPcConversation(false);
	Poser::EndEvent();
}


/********************
 * General tick Event
 ********************/

void __stdcall GeneralPreTick(ExtClass::MainConversationStruct* param) {

}

void __stdcall GeneralPostTick(ExtClass::MainConversationStruct* param) {

}

/********************
 * NPC -> PC interactive conversation tick event
 *******************/
void __stdcall NpcPcInteractivePreTick(ExtClass::NpcPcInteractiveConversationStruct* param) {
}

void __stdcall NpcPcInteractivePostTick(ExtClass::NpcPcInteractiveConversationStruct* param) {
	HAi::ConversationTickPost(param);
}

/********************
* NPC -> PC non interactive conversation tick event
*******************/

void __stdcall NpcPcNonInteractivePreTick(ExtClass::NpcPcNonInteractiveConversationStruct* param) {

}

void __stdcall NpcPcNonInteractivePostTick(ExtClass::NpcPcNonInteractiveConversationStruct* param) {
	HAi::ConversationTickPost(param);
}

/********************
 * Called after the NPC sets the response-member of the param struct to answer something.
 * Parameter type is whatever it currently is
 ********************/
void __stdcall NpcAnswer(ExtClass::BaseConversationStruct* param) {
	
}

/*******************
 * Called before the response of the PC in playerAnswer is copied into the response.
 * Parameter type is whatever it currently is
 *******************/
void __stdcall PcAnswer(ExtClass::BaseConversationStruct* param) {
	HAi::ConversationPcResponse(param);
}


/***
* Called for every pc conversation tick. do distributes to other callbacks. do not touch.
*/
void __stdcall PreTick(ExtClass::MainConversationStruct* param) {
	GeneralPreTick(param);
	DWORD substruct = param->GetSubstructType();
	switch (substruct) {
	case ExtClass::PCCONTYPE_NPCPC_GIVEANSWER:
		NpcPcInteractivePreTick((ExtClass::NpcPcInteractiveConversationStruct*) (((DWORD)(param)+param->SubstructOffsets[substruct])));
		break;
	case ExtClass::PCCONTYPE_NPCPC_NOASNWER:
		NpcPcNonInteractivePreTick((ExtClass::NpcPcNonInteractiveConversationStruct*)(((DWORD)(param)+param->SubstructOffsets[substruct])));
		break;
	default:
		break;
	}
}

void __stdcall PostTick(ExtClass::MainConversationStruct* param) {
	GeneralPostTick(param);
	DWORD substruct = param->GetSubstructType();
	switch (substruct) {
	case ExtClass::PCCONTYPE_NPCPC_GIVEANSWER:
		NpcPcInteractivePostTick((ExtClass::NpcPcInteractiveConversationStruct*) (((DWORD)(param)+param->SubstructOffsets[substruct])));
		break;
	case ExtClass::PCCONTYPE_NPCPC_NOASNWER:
		NpcPcNonInteractivePostTick((ExtClass::NpcPcNonInteractiveConversationStruct*)(((DWORD)(param)+param->SubstructOffsets[substruct])));
		break;
	default:
		break;
	}
}


DWORD OriginalTickFunction;
void __declspec(naked) TickRedirect() {
	__asm {
		mov eax, [edi+0x2C]
		push eax
		call PreTick
		mov eax, [edi+ 0x2C]
		call [OriginalTickFunction]
		push eax //rescue return value
		mov eax, [edi+ 0x2C]
		push eax
		call PostTick
		pop eax //restore return value
		ret
	}
}

void TickInjection() {
	//a general callback for conversations with the pc. eax thiscall, no stack parameters.
	//eax is a pointer to a struct that transmorphs as described in ConversationStruct.h
	//AA2Play v12 FP v1.4.0a.exe+4A237 - 8B 47 2C              - mov eax,[edi+2C]
	//AA2Play v12 FP v1.4.0a.exe + 4A23A - E8 416C0000 - call "AA2Play v12 FP v1.4.0a.exe" + 50E80 {->AA2Play v12 FP v1.4.0a.exe + 50E80 }
	DWORD address = General::GameBase + 0x4A23A;
	DWORD redirectAddress = (DWORD)(&TickRedirect);
	Hook((BYTE*)address,
	{ 0xE8, 0x41, 0x6C, 0x00, 0x00 },						//expected values
	{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		(DWORD*)(&OriginalTickFunction));
}

void __declspec(naked) NpcAnswerRedirect() {
	__asm {
		//orignal code
		mov ecx, [edx + 04]
		mov[ecx + ebp + 0x30], eax
		//give a chance to change it
		pushad
		push ebp
		call NpcAnswer
		popad
		ret
	}
}

void NpcAnswerInjection() {
	//this function returns the reaction (positive = 1, negative = 0). ebp is the struct.
	/*AA2Play v12 FP v1.4.0a.exe + 5B3D0 - 8B 48 20 - mov ecx, [eax + 20]
	AA2Play v12 FP v1.4.0a.exe + 5B3D3 - 52 - push edx
	AA2Play v12 FP v1.4.0a.exe + 5B3D4 - 51 - push ecx
	AA2Play v12 FP v1.4.0a.exe + 5B3D5 - 8B 48 50 - mov ecx, [eax + 50]
	AA2Play v12 FP v1.4.0a.exe + 5B3D8 - 8B 40 54 - mov eax, [eax + 54]
	AA2Play v12 FP v1.4.0a.exe + 5B3DB - E8 20C81200 - call "AA2Play v12 FP v1.4.0a.exe" + 187C00{ ->AA2Play v12 FP v1.4.0a.exe + 187C00 }
	AA2Play v12 FP v1.4.0a.exe + 5B3E0 - 8B 55 00 - mov edx, [ebp + 00]
	AA2Play v12 FP v1.4.0a.exe+5B3E0 - 8B 55 00              - mov edx,[ebp+00]
	AA2Play v12 FP v1.4.0a.exe+5B3E3 - 8B 4A 04              - mov ecx,[edx+04]
	AA2Play v12 FP v1.4.0a.exe+5B3E6 - 89 44 29 30           - mov [ecx+ebp+30],eax
*/

	DWORD address = General::GameBase + 0x5B3E3;
	DWORD redirectAddress = (DWORD)(&NpcAnswerRedirect);
	Hook((BYTE*)address,
		{ 0x8B, 0x4A, 0x04, 
		0x89, 0x44, 0x29, 0x30 },						//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress, 0x90, 0x90 },	//redirect to our function
		NULL);
}

void __declspec(naked) PcAnswerRedirect() {
	__asm {
		pushad
		push ebp
		call PcAnswer
		popad
		//original code
		mov eax, [ebp+0]
		mov ecx, [eax+4]
		ret
	}
}

void PcAnswerInjection() {
	//ebp is the conversation struct. eax+4C is taken out and after the cmp translated to the regular response.
	/*AA2Play v12 FP v1.4.0a.exe + 58DF8 - 8B 45 00 - mov eax, [ebp + 00]
	AA2Play v12 FP v1.4.0a.exe + 58DFB - 8B 48 04 - mov ecx, [eax + 04]
	AA2Play v12 FP v1.4.0a.exe + 58DFE - 8D 04 29 - lea eax, [ecx + ebp]
	AA2Play v12 FP v1.4.0a.exe + 58E01 - 8B 48 4C - mov ecx, [eax + 4C]
	AA2Play v12 FP v1.4.0a.exe + 58E04 - 83 F9 01 - cmp ecx, 01 { 1 }*/
	DWORD address = General::GameBase + 0x58DF8;
	DWORD redirectAddress = (DWORD)(&PcAnswerRedirect);
	Hook((BYTE*)address,
		{ 0x8B, 0x45, 0x00, 
		0x8B, 0x48, 0x04 },						//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress, 0x90 },	//redirect to our function
		NULL);
}

DWORD OriginalStart;
void __declspec(naked) StartRedirect() {
	__asm {
		pushad
		call StartEvent
		popad
		push [esp+4]
		call [OriginalStart]
		ret 4
	}
}

void StartInjection() {
	//this call seems to load a lot about a pc-conversation, including the global main struct. stdcall, 1 stack param
	/*AA2Play v12 FP v1.4.0a.exe+EB8B1 - 52                    - push edx
	AA2Play v12 FP v1.4.0a.exe+EB8B2 - E8 59DFF5FF           - call "AA2Play v12 FP v1.4.0a.exe"+49810 { ->AA2Play v12 FP v1.4.0a.exe+49810 }
	AA2Play v12 FP v1.4.0a.exe+EB8B7 - 84 C0                 - test al,al
	AA2Play v12 FP v1.4.0a.exe+EB8B9 - 74 8D                 - je "AA2Play v12 FP v1.4.0a.exe"+EB848 { ->AA2Play v12 FP v1.4.0a.exe+EB848 }
	*/
	DWORD address = General::GameBase + 0xEB8B2;
	DWORD redirectAddress = (DWORD)(&StartRedirect);
	Hook((BYTE*)address,
		{ 0xE8, 0x59, 0xDF, 0xF5, 0xFF },						//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress},	//redirect to our function
		&OriginalStart);
}

void __declspec(naked) EndRedirect() {
	__asm {
		pushad
		call EndEvent
		popad
		mov eax, [edi+0x2C]
		test eax,eax
		ret
	}
}

void EndInjection() {
	//this function is called from 3 different places and seems to be involved in ending a conversation.
	/*AA2Play v12 FP v1.4.0a.exe+4AFD0 - 8B 47 2C              - mov eax,[edi+2C]
	AA2Play v12 FP v1.4.0a.exe+4AFD3 - 85 C0                 - test eax,eax
	AA2Play v12 FP v1.4.0a.exe+4AFD5 - 74 17                 - je "AA2Play v12 FP v1.4.0a.exe"+4AFEE { ->AA2Play v12 FP v1.4.0a.exe+4AFEE }
	...
	AA2Play v12 FP v1.4.0a.exe+4B02E - 75 D0                 - jne "AA2Play v12 FP v1.4.0a.exe"+4B000 { ->AA2Play v12 FP v1.4.0a.exe+4B000 }
	AA2Play v12 FP v1.4.0a.exe+4B030 - 5E                    - pop esi
	AA2Play v12 FP v1.4.0a.exe+4B031 - 5D                    - pop ebp
	AA2Play v12 FP v1.4.0a.exe+4B032 - 5B                    - pop ebx
	AA2Play v12 FP v1.4.0a.exe+4B033 - C3                    - ret
	AA2Play v12 FP v1.4.0a.exe+4B034 - CC                    - int 3
	AA2Play v12 FP v1.4.0a.exe+4B035 - CC                    - int 3
	*/
	DWORD address = General::GameBase + 0x4AFD0;
	DWORD redirectAddress = (DWORD)(&EndRedirect);
	Hook((BYTE*)address,
	{ 0x8B, 0x47, 0x2C,
		0x85, 0xC0 },						//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		NULL);
}





}
}
