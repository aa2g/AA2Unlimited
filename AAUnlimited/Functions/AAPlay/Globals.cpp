#include "StdAfx.h"

using namespace Shared::Triggers;

namespace AAPlay {



CharInstData g_characters[25];
CharInstData g_previewChar;

void InitOnLoad() {
	LUA_EVENT_NORET("load_class", false);

	for(int i = 0; i < 25; i++) {
		g_characters[i].Reset();
	}

	//initialize characters
	ExtClass::CharacterStruct** start = ExtVars::AAPlay::ClassMembersArray();
	ExtClass::CharacterStruct** end = ExtVars::AAPlay::ClassMembersArrayEnd();
	int idxCharacter = 0;
	for(start; start != end; start++, idxCharacter++) {
		ExtClass::CharacterStruct* it = *start;

		int seat = it->m_seat;
		g_characters[seat].m_char = it;
		g_characters[seat].LoadAAUData();		
		g_characters[seat].charOffset = 0x04 * idxCharacter;

		//initialize triggers
		auto& aauData = g_characters[seat].m_cardData;
		for (Trigger& trg : aauData.GetTriggers()) {
			trg.Initialize(&aauData.GetGlobalVariables(), seat);
		}
		//initialize modules
		for (auto& module : aauData.GetModules()) {
			for (Trigger& trg : module.triggers) {
				trg.Initialize(&aauData.GetGlobalVariables(), seat);
			}
		}

		//throw init event
		CardInitializeData data;
		data.card = seat;
		ThrowEvent(&data);

		// ditto for lua
		LUA_EVENT_NORET("load_card", seat, false);
	}
	LOGPRIONC(Logger::Priority::INFO) "Loaded class with " << (idxCharacter) << " cards in it.\r\n";

	LUA_EVENT_NORET("load_class", true);
}

void InitTransferedCharacter(ExtClass::CharacterStruct* character) {
	int seat = character->m_seat;
	g_characters[seat].m_char = character;
	bool modded = g_characters[seat].LoadAAUData();
	if (modded)
		LOGPRIO(Logger::Priority::INFO) << std::dec << "Loaded modcard into seat " << seat << "\n";
	
	//get chrOffset
	ExtClass::CharacterStruct** start = ExtVars::AAPlay::ClassMembersArray();
	ExtClass::CharacterStruct** end = ExtVars::AAPlay::ClassMembersArrayEnd();
	int idxCharacter = 0;
	for (start; start != end; start++, idxCharacter++) {
		ExtClass::CharacterStruct* it = *start;
		if (it->m_seat == seat) {
			g_characters[seat].charOffset = 0x04 * idxCharacter;
		}
	}

	//initialize triggers
	auto& aauData = g_characters[seat].m_cardData;
	for (auto& trg : aauData.GetTriggers()) {
		trg.Initialize(&aauData.GetGlobalVariables(),seat);
	}
	//initialize modules
	for (auto& module : aauData.GetModules()) {
		for (auto& trg : module.triggers) {
			trg.Initialize(&aauData.GetGlobalVariables(), seat);
		}
	}
	//throw init event
	CardInitializeData data;
	data.card = seat;
	ThrowEvent(&data);

	LUA_EVENT("load_card", seat, true);

	//throw added to class event
	CardAddedData cdata;
	cdata.card = seat;
	ThrowEvent(&cdata);
}
void RemoveTransferedCharacter(ExtClass::CharacterStruct* character) {
	int seat = character->m_seat;
	//throw destroy event
	CardDestroyData data;
	data.card = seat;
	ThrowEvent(&data);
	LUA_EVENT_NORET("unload_card", seat);

	//destroy
	g_characters[seat].Reset();
}


void ApplyRelationshipPoints(ExtClass::CharacterStruct* characterFrom,ExtClass::CharacterRelation* relation) {
	//this sequence of actions seems to apply relationship points. they get cut of in steps of 30 by the first function,
	//then magically get applied by the second function. dont know what the third one does.
	//esi is the relationship struct (param relation),
	//ebx is the unkown struct that contains the relationship array (currently called moreData)
	//all stdcall, nothing to worry about
	/*AA2Play v12 FP v1.4.0a.exe+142923 - 56                    - push esi
	AA2Play v12 FP v1.4.0a.exe+142924 - 53                    - push ebx
	AA2Play v12 FP v1.4.0a.exe+142925 - E8 26000000           - call "AA2Play v12 FP v1.4.0a.exe"+142950 { ->AA2Play v12 FP v1.4.0a.exe+142950 }
	AA2Play v12 FP v1.4.0a.exe+14292A - 80 7B 2C 01           - cmp byte ptr [ebx+2C],01 { 1 }
	AA2Play v12 FP v1.4.0a.exe+14292E - 75 11                 - jne "AA2Play v12 FP v1.4.0a.exe"+142941 { ->AA2Play v12 FP v1.4.0a.exe+142941 }
	AA2Play v12 FP v1.4.0a.exe+142930 - 56                    - push esi
	AA2Play v12 FP v1.4.0a.exe+142931 - E8 8A020000           - call "AA2Play v12 FP v1.4.0a.exe"+142BC0 { ->AA2Play v12 FP v1.4.0a.exe+142BC0 }
	AA2Play v12 FP v1.4.0a.exe+142936 - 8B C3                 - mov eax,ebx
	AA2Play v12 FP v1.4.0a.exe+142938 - E8 132F0000           - call "AA2Play v12 FP v1.4.0a.exe"+145850 { ->AA2Play v12 FP v1.4.0a.exe+145850 }
	AA2Play v12 FP v1.4.0a.exe+14293D - C6 43 2C 00           - mov byte ptr [ebx+2C],00 { 0 }
	*/

	void* unknownStruct = (void*)characterFrom->m_moreData;
	
	void  (__stdcall *firstFunc)(void* structAbove,ExtClass::CharacterRelation* relation);
	void (__stdcall *secondFunc)(ExtClass::CharacterRelation* relation);
	void (__stdcall *thirdFunc)(void* structAbove);
	firstFunc = (decltype(firstFunc))(General::GameBase + 0x142950);
	secondFunc = (decltype(secondFunc))(General::GameBase + 0x142BC0);
	thirdFunc = (decltype(thirdFunc))(General::GameBase + 0x145850);

	firstFunc(unknownStruct,relation);
	BYTE* flag = (BYTE*)(unknownStruct)+0x2C;
	if(*flag == 1) {
		secondFunc(relation);
		//third one is called with eax as param
		__asm {
			mov eax, [unknownStruct]
			call [thirdFunc]
		}
		*flag = 0;
	}
}


}
