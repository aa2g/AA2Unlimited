#include "GameState.h"

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
	}

	//Game state indicators
	bool m_isClothesScreen;		//true if in clothes edit screen
	bool m_isPcConversation;	//true if PC is in conversation mode
	bool m_isOverriding;		//true if overrides need to be applied
	bool m_isMenuMode;			//true if in menu mode(settings, roster, save/load, etc)

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
