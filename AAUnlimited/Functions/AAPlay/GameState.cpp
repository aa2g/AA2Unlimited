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
	}

	//Game state indicators
	bool m_isClothesScreen;				//true if in clothes edit screen
	bool m_isPcConversation;			//true if PC is in conversation mode
	bool m_isOverriding;				//true if overrides need to be applied
	bool m_isMenuMode;					//true if in menu mode(settings, roster, save/load, etc)
	std::wstring m_classSaveName;
	DWORD m_PCConversationState;		//0 = still speaking, 1 = waiting for answer, 2/3 = answering/end?
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

void Shared::GameState::setPCConversationState(DWORD value)
{
	loc_gameState.m_PCConversationState = value;
}

DWORD Shared::GameState::getPCConversationState()
{
	return loc_gameState.m_PCConversationState;
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
		auto currentChar = getPlayerCharacter();
		*currentChar = AAPlay::g_characters[seat].m_char;
	}
}

std::wstring Shared::GameState::getCurrentClassSaveName()
{
	static const DWORD saveNameOffset[]{ 0x376164, 0x28, 0x64 };
	auto className = std::wstring((wchar_t*)ExtVars::ApplyRule(saveNameOffset));
	loc_gameState.m_classSaveName = className.substr(0, className.size() - 4);

	return loc_gameState.m_classSaveName;
}
