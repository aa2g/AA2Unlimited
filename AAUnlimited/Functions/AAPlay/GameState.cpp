#include "StdAfx.h"

using namespace Shared::GameState;

struct GameStateStruct {
	GameStateStruct()
	{
		reset();
	}

	inline void reset() {
		m_isClothesScreen = false;
		m_isPcConversation = false;
		m_isOverriding = false;
		m_isMenuMode = false;
		m_isHighPolyLoaded = false;
		m_PCConversationState = -1;
		m_classSaveName = L"";
		m_char[0] = nullptr;
		m_char[1] = nullptr;
		for (int i = 0; i < 25; i++)
		{
			roomNumber[i] = -1;
		}
	}

	//Game state indicators
	bool m_isClothesScreen;				//true if in clothes edit screen
	bool m_isPcConversation;			//true if PC is in conversation mode
	bool m_isOverriding;				//true if overrides need to be applied
	bool m_isMenuMode;					//true if in menu mode(settings, roster, save/load, etc)
	
	bool m_isPeeping;					//true if the current H scene does not involve the PC
	ExtClass::CharacterStruct* m_voyeur;//the observer of the H scene. Virtually always the PC regardless of what the game thinks about it
	ExtClass::NpcData* m_voyeurTarget;//the NPC voyeur is peeping at. Usually the first actor in the H scene

	std::wstring m_classSaveName;
	DWORD m_PCConversationState;		//0 = still speaking, 1 = waiting for answer, 2/3 = answering/end?
	DWORD m_NPCLineState;				//increments from 0 to whatever
	int roomNumber[25];					//Current room ID
	DWORD interrupt;					//Disabled interruptions
#define CONVERSATION_CHARACTERS_N 2
	ExtClass::CharacterStruct* m_char[CONVERSATION_CHARACTERS_N];

	//TODO: it's a band-aid solution for managing isOverriding. Either remove the flag if a better one is implemented or remove the comment if there are other uses found for it.
	bool m_isHighPolyLoaded;	//true if there's a high poly model currently on screen
} loc_gameState;

void Shared::GameState::reset() {
	loc_gameState.reset();
}

void Shared::GameState::setIsClothesScreen(bool value)
{
	loc_gameState.m_isClothesScreen = value;
}

bool Shared::GameState::getIsClothesScreen()
{
	return loc_gameState.m_isClothesScreen;
}

void Shared::GameState::setIsPcConversation(bool value)
{
	loc_gameState.m_isPcConversation = value;
}

bool Shared::GameState::getIsPcConversation()
{
	return loc_gameState.m_isPcConversation;
}

void Shared::GameState::setIsOverriding(bool value)
{
	loc_gameState.m_isOverriding = value;
}

bool Shared::GameState::getIsOverriding()
{
	return loc_gameState.m_isOverriding;
}

void Shared::GameState::updateIsOverriding()
{
	Shared::GameState::setIsOverriding(
		Shared::GameState::getIsClothesScreen() ||
		Shared::GameState::getIsPcConversation()
	);
}

void Shared::GameState::setIsHighPolyLoaded(bool value)
{
	loc_gameState.m_isHighPolyLoaded = value;
}

bool Shared::GameState::getIsHighPolyLoaded()
{
	return loc_gameState.m_isHighPolyLoaded;
}

void Shared::GameState::setIsPeeping(bool value)
{
	loc_gameState.m_isPeeping = value;
}

bool Shared::GameState::getIsPeeping()
{
	return loc_gameState.m_isPeeping;
}

void Shared::GameState::setVoyeur(ExtClass::CharacterStruct* voyeur)
{
	loc_gameState.m_voyeur = voyeur;
}

ExtClass::CharacterStruct* Shared::GameState::getVoyeur()
{
	return loc_gameState.m_voyeur;
}

void Shared::GameState::setVoyeurTarget(ExtClass::NpcData* target)
{
	loc_gameState.m_voyeurTarget = target;
}

void Shared::GameState::setInterrupt(DWORD value)
{
	loc_gameState.interrupt = value;
}

DWORD Shared::GameState::getInterrupt()
{
	return loc_gameState.interrupt;
}

ExtClass::NpcData* Shared::GameState::getVoyeurTarget()
{
	return loc_gameState.m_voyeurTarget;
}

void Shared::GameState::setPCConversationState(DWORD value)
{
	loc_gameState.m_PCConversationState = value;
}

DWORD Shared::GameState::getPCConversationState()
{
	return loc_gameState.m_PCConversationState;
}

void Shared::GameState::setNPCLineState(DWORD value)
{
	loc_gameState.m_NPCLineState = value;
}

DWORD Shared::GameState::getNPCLineState()
{
	return loc_gameState.m_NPCLineState;
}

void Shared::GameState::SetRoomNumber(int seat, int room) {
	if (seat < 0 || seat >= 25) return;
	loc_gameState.roomNumber[seat] = room;
}

int Shared::GameState::GetRoomNumber(int seat) {
	if (seat < 0 || seat >= 25) return -1;
	return loc_gameState.roomNumber[seat];
}


void Shared::GameState::addConversationCharacter(ExtClass::CharacterStruct* chara) {
	for (int i = 0; i < CONVERSATION_CHARACTERS_N; i++) {
		if (!loc_gameState.m_char[i]) {
			setConversationCharacter(chara, i);
			return;
		}
	}
}
ExtClass::CharacterStruct* Shared::GameState::getConversationCharacter(int idx) {
	return loc_gameState.m_char[idx % CONVERSATION_CHARACTERS_N];
}
void Shared::GameState::setConversationCharacter(ExtClass::CharacterStruct* chara, int idx) {
	loc_gameState.m_char[idx] = chara;
}
void Shared::GameState::clearConversationCharacter(int idx) {
	if (idx < 0) {	//if idx < 0 clear all the characters
		for (int i = 0; i < CONVERSATION_CHARACTERS_N; i++) {
			loc_gameState.m_char[i] = nullptr;
		}
	} else {
		loc_gameState.m_char[idx] = nullptr;
	}
}

void Shared::GameState::setPlayerCharacter(int seat) {
	if (AAPlay::g_characters[seat].IsValid()) {
		const DWORD offstPC[] = { 0x376164, 0x88 };
		auto pc = (ExtClass::CharacterStruct**)ExtVars::ApplyRule(offstPC);
		*pc = AAPlay::g_characters[seat].m_char;
	}
}

std::wstring Shared::GameState::getCurrentClassSaveName()
{
	static const DWORD saveNameOffset[]{ 0x376164, 0x28, 0x64 };
	auto strp = (const wchar_t*)ExtVars::ApplyRule(saveNameOffset);
	auto className = std::wstring(strp?strp:L"");
	loc_gameState.m_classSaveName = className.substr(0, className.size() - 4);

	return loc_gameState.m_classSaveName;
}
