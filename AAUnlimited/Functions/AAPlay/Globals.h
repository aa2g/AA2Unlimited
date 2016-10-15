#pragma once

#include "Functions\CharInstData.h"
#include "External\ExternalClasses\CharacterStruct.h"

namespace AAPlay {

	extern CharInstData g_characters[25]; //characters by their SEAT number, NOT by their internal order

	void InitOnLoad();
	void InitTransferedCharacter(ExtClass::CharacterStruct* character);
	void RemoveTransferedCharacter(ExtClass::CharacterStruct* character);

	inline CharInstData* GetInstFromStruct(ExtClass::CharacterStruct* character) { return &g_characters[character->m_seat]; }

}