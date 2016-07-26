#include "NpcPcConversation.h"
#include "MemMods/Hook.h"
#include "General/ModuleInfo.h"
#include "External/ExternalClasses.h"
#include "Functions/AAPlay/HAi.h"

namespace PlayInjections {
/*
 * Events that are thrown when a npc talks to a pc.
 */
namespace NpcPcConversation {

/*
	* Tick that is called while a NPC->PC conversation is taking place
	*/
void __cdecl Tick(ExtClass::ConversationStruct* param) {
	HAi::ConversationTick(param);
}

DWORD loc_OriginalTickFunction;

void __declspec(naked) TickRedirect() {
	_asm {
		//remember, its an eax thiscall
		pushad
		push eax
		call Tick
		add esp, 4
		popad
		jmp [loc_OriginalTickFunction] //redirect to original function (it will return for us)
	}
}

void TickInjection() {
	//eax-thiscall with ConversationStruct
	//AA2Play v12 FP v1.4.0a.exe+50FCE - E8 4D750000           - call "AA2Play v12 FP v1.4.0a.exe"+58520 {->AA2Play v12 FP v1.4.0a.exe+58520 }
	DWORD address = General::GameBase + 0x50FCE;
	DWORD redirectAddress = (DWORD)(&TickRedirect);
	Hook((BYTE*)address,
	{ 0xE8, 0x4D, 0x75, 0x00, 0x00 },						//expected values
	{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		(DWORD*)(&loc_OriginalTickFunction));
}


}
}
