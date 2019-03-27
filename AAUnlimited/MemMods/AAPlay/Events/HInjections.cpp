#include "StdAfx.h"

namespace PlayInjections {
namespace HPlayInjections {

ExtClass::HInfo* loc_currentHInfo;
ExtClass::CharacterStruct* loc_activeChar = NULL;
ExtClass::CharacterStruct* loc_passiveChar = NULL;
ExtClass::XXFile* loc_passiveFaceXX = NULL;
ExtClass::XXFile* loc_activeFaceXX = NULL;
DWORD loc_currPos = -1;

bool (__stdcall *loc_OriginalTickFunction)(ExtClass::HInfo* info);


//take note that these ticks might be called multiple times even after returning contScene = false
bool __stdcall TickRedirect(ExtClass::HInfo* hInfo) {
	Shared::GameState::setHInfo(hInfo);
	HAi::PreTick(hInfo);
	bool contScene = loc_OriginalTickFunction(hInfo);
	if (contScene) {
		if (!loc_currentHInfo) {
			LOGPRIO(Logger::Priority::INFO) << "H started\n";
			LUA_EVENT_NORET("start_h", hInfo);
			Shared::Triggers::HStartData data;
			data.card = Shared::GameState::getPlayerCharacter()->m_char->m_seat;
			data.dominantParticipant = hInfo->m_activeParticipant->m_charPtr->m_seat;
			data.submissiveParticipant = hInfo->m_passiveParticipant->m_charPtr->m_seat;
			Shared::Triggers::ThrowEvent(&data);
		}
		loc_currentHInfo = hInfo;
	}
	else {
		if (loc_currentHInfo) {
			LOGPRIO(Logger::Priority::INFO) << "H ended\n";
			LUA_EVENT_NORET("end_h", hInfo);
			Shared::Triggers::HEndData data;
			data.card = Shared::GameState::getPlayerCharacter()->m_char->m_seat;
			Shared::Triggers::ThrowEvent(&data);
			
		}
		loc_currentHInfo = NULL;

		loc_currPos = -1;
		loc_activeChar = NULL;
		loc_passiveChar = NULL;
		loc_activeFaceXX = NULL;
		loc_passiveFaceXX = NULL;
	}
	HAi::PostTick(hInfo, contScene);
	HButtonMove::PostTick(hInfo, contScene);
	if (loc_currentHInfo) {
		auto active = hInfo->m_activeParticipant->m_charPtr;
		auto passive = hInfo->m_passiveParticipant->m_charPtr;
		if (
			(loc_activeChar != active) ||
			(loc_passiveChar != passive) ||
			(active->m_xxFace != loc_activeFaceXX) ||
			(passive->m_xxFace != loc_passiveFaceXX) ||
			(loc_currPos != hInfo->m_currPosition)) {

			LUA_EVENT_NORET("change_h", hInfo, loc_currPos, loc_activeChar, loc_passiveChar, loc_activeFaceXX, loc_passiveFaceXX);
			//LUA_EVENT_NORET("change_h", hInfo, (loc_activeChar != active), (loc_passiveChar != passive), (active->m_xxFace != loc_activeFaceXX), (passive->m_xxFace != loc_passiveFaceXX), (loc_currPos != hInfo->m_currPosition))
			loc_currPos = hInfo->m_currPosition;
			loc_activeChar = active;
			loc_passiveChar = passive;
			loc_activeFaceXX = active->m_xxFace;
			loc_passiveFaceXX = passive->m_xxFace;
		}
	}

	ExtClass::Camera::PostTick(hInfo,contScene);
	return contScene;
}

void TickInjection() {
	//			bool HTick(HInfo*);
	// Returns false if this tick ends h (end button pressed). stdcall.
	//AA2Play v12 FP v1.4.0a.exe+80DB0 - 53                    - push ebx
	//AA2Play v12 FP v1.4.0a.exe+80DB1 - E8 9ADDFFFF           - call "AA2Play v12 FP v1.4.0a.exe"+7EB50{ ->AA2Play v12 FP v1.4.0a.exe+7EB50 }
	//this function returns false if h actually ends (not just if the button was pressed), but might be called multiple times
	//despite h ending
	//AA2Play v12 FP v1.4.0a.exe+3B5C7 - 50                    - push eax
	//AA2Play v12 FP v1.4.0a.exe+3B5C8 - E8 03510400           - call "AA2Play v12 FP v1.4.0a.exe"+806D0 { ->AA2Play v12 FP v1.4.0a.exe+806D0 }
	DWORD address = General::GameBase + 0x3B5C8;
	DWORD redirectAddress = (DWORD)(&TickRedirect);
	Hook((BYTE*)address,
		{ 0xE8, 0x03, 0x51, 0x04, 0x00 },						//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		(DWORD*)(&loc_OriginalTickFunction));						//save old function to call in our redirect function
}

}
}


//AA2Play v12 FP v1.4.0a.exe+326FC0 is virtual table [+4] is function that changes h position (thiscall on temp struct, unknown properties) (part of the tick)
/*
jump is usually taken if h scene is NOT ended due to high virtue first time
AA2Play v12 FP v1.4.0a.exe+95167 - E8 E4F8FFFF           - call "AA2Play v12 FP v1.4.0a.exe"+94A50 { ->AA2Play v12 FP v1.4.0a.exe+94A50 }
AA2Play v12 FP v1.4.0a.exe+9516C - 84 C0                 - test al,al
AA2Play v12 FP v1.4.0a.exe+9516E - 74 48                 - je "AA2Play v12 FP v1.4.0a.exe"+951B8 { ->AA2Play v12 FP v1.4.0a.exe+951B8 }
*/
//ends h scene as a result of the scene switch
//AA2Play v12 FP v1.4.0a.exe+9519E - E8 2DE8FEFF           - call "AA2Play v12 FP v1.4.0a.exe"+839D0 { ->AA2Play v12 FP v1.4.0a.exe+839D0 }

/*
jump is taken if h scene is ended because npc was seen. eax is hstruct (part of the tick)
AA2Play v12 FP v1.4.0a.exe+80F83 - 8B C3                 - mov eax,ebx
AA2Play v12 FP v1.4.0a.exe+80F85 - E8 36220000           - call "AA2Play v12 FP v1.4.0a.exe"+831C0 { ->AA2Play v12 FP v1.4.0a.exe+831C0 }
AA2Play v12 FP v1.4.0a.exe+80F8A - 84 C0                 - test al,al
AA2Play v12 FP v1.4.0a.exe+80F8C - 75 28                 - jne "AA2Play v12 FP v1.4.0a.exe"+80FB6 { ->AA2Play v12 FP v1.4.0a.exe+80FB6 }
 */
/*
jump is taken if was seen long enough to end h scene
esi is npc action data (charstruct f60->1C)
AA2Play v12 FP v1.4.0a.exe+190DB5 - 8B 56 08              - mov edx,[esi+08]
AA2Play v12 FP v1.4.0a.exe+190DB8 - 8B 82 B40B0000        - mov eax,[edx+00000BB4]
AA2Play v12 FP v1.4.0a.exe+190DBE - 01 46 28              - add [esi+28],eax
AA2Play v12 FP v1.4.0a.exe+190DC1 - 8B 46 28              - mov eax,[esi+28]
AA2Play v12 FP v1.4.0a.exe+190DC4 - 3D 60EA0000           - cmp eax,0000EA60 { 60000 }
AA2Play v12 FP v1.4.0a.exe+190DC9 - 73 1E                 - jae "AA2Play v12 FP v1.4.0a.exe"+190DE9 { ->AA2Play v12 FP v1.4.0a.exe+190DE9 }
*/
/*
if above jump is taken, this function returns true and takes the jump, ending the scene
AA2Play v12 FP v1.4.0a.exe+190AB1 - E8 7A020000           - call "AA2Play v12 FP v1.4.0a.exe"+190D30 { ->AA2Play v12 FP v1.4.0a.exe+190D30 }
AA2Play v12 FP v1.4.0a.exe+190AB6 - 84 C0                 - test al,al
AA2Play v12 FP v1.4.0a.exe+190AB8 - 75 18                 - jne "AA2Play v12 FP v1.4.0a.exe"+190AD2 { ->AA2Play v12 FP v1.4.0a.exe+190AD2 }
*/
