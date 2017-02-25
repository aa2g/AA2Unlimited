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
	CharInstData();
	~CharInstData();

	ExtClass::CharacterStruct* m_char;
	AAUCardData m_cardData;

	std::vector<std::pair<AAUCardData::HairPart,ExtClass::XXFile*>> m_hairs[4];

	void Reset();
	inline bool IsValid() { return m_char != NULL; }
};

