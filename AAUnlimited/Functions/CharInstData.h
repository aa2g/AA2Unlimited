#pragma once

#include <vector>

#include "External\ExternalClasses\CharacterStruct.h"
#include "Functions\AAUCardData.h"

/*
 * Contains data about a character that currently exists in a game.
 * Apart from the pointer to the original struct, this includes the AAU card data,
 * currently loaded hair pointers etc
 */
class CharInstData
{
public:
	struct ActionParamStruct {
		DWORD conversationId = -1;
		DWORD movementType;
		DWORD roomTarget;
		DWORD unknown; //always -1
		ExtClass::CharacterActivity* target1;
		ExtClass::CharacterActivity* target2;
		DWORD unknown2; //always 1, though initialized to 2
	};

public:
	CharInstData();
	~CharInstData();

	ExtClass::CharacterStruct* m_char;
	AAUCardData m_cardData;

	std::vector<std::pair<AAUCardData::HairPart,ExtClass::XXFile*>> m_hairs[4];

	ActionParamStruct m_forceAction;

	void Reset();
	inline bool IsValid() { return m_char != NULL; }
};

