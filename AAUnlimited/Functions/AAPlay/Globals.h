#pragma once

#include "Functions\CharInstData.h"
#include "External\ExternalClasses\CharacterStruct.h"

namespace AAPlay {

	extern CharInstData g_characters[25]; //characters by their SEAT number, NOT by their internal order
	extern CharInstData g_previewChar;

	void InitOnLoad();
	void InitTransferedCharacter(ExtClass::CharacterStruct* character);
	void RemoveTransferedCharacter(ExtClass::CharacterStruct* character);
	void SetPreviewChar(ExtClass::CharacterStruct* previewChar);

	inline CharInstData* GetInstFromStruct(ExtClass::CharacterStruct* character) { return &g_characters[character->m_seat]; }
	inline int GetSeatFromStruct(ExtClass::CharacterStruct* character) { return character->m_seat; }


	void ApplyRelationshipPoints(ExtClass::CharacterStruct* characterFrom,ExtClass::CharacterRelation* relation);
}