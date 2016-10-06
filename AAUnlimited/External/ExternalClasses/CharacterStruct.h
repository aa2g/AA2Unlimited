#pragma once
#include <Windows.h>

#include "Bone.h"
#include "CharacterData.h"
namespace ExtClass {

#pragma pack(push, 1)
/*
 * Represents a character in the game and its state.
 */
class CharacterStruct
{
public:
	void* m_virtualTable;
	BYTE m_unknown1[0x24];
	CharacterData* m_charData;
	BYTE m_unknown2[0x16];
	BYTE m_bClothesOn;
	BYTE m_unknown3;
	BYTE m_currClothes;
	BYTE m_unknown_[0x15B];
	Bone** m_bonePtrArray; //first one is neck (focused on q press), second one is spin (focused on w press)
	Bone** m_bonePtrArrayEnd; //(exclusive, not part of array anymore)
	BYTE m_unknown4[0xDB4];
	void* m_somedata;
	BYTE m_unknown5[0x3C];

public:
	CharacterStruct() = delete;
	~CharacterStruct() = delete;
};

static_assert(sizeof(CharacterStruct) == 0xF9C, "CharacterStruct size missmatch; must be 0xF9C bytes (allocation size)");


#pragma pack(pop)
}
