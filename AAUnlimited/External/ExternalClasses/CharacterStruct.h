#pragma once
#include <Windows.h>

#include "Bone.h"
#include "CharacterData.h"
#include "XXFile.h"
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
	void* m_somePointer;
	BYTE m_unknown2[12];
	int m_seat; //seat number; from top to bottom, right to left, zero based, teacher is exception and 24
	BYTE m_unknown3[2];
	BYTE m_bClothesOn;
	BYTE m_unknown4;
	BYTE m_currClothes;
	BYTE m_unknown5[3];
	XXFile* m_xxFace; //certain pointers to model files. all of these may be NULL if they are not loaded yet or not used
	XXFile* m_xxGlasses;
	union {
		struct {
			XXFile* m_xxFrontHair;
			XXFile* m_xxSideHair;
			XXFile* m_xxBackHair;
			XXFile* m_xxHairExtension;
		};
		XXFile* m_xxHairs[4];
	};
	XXFile* m_xxTounge;
	XXFile* m_xxSkeleton;
	XXFile* m_xxBody;
	XXFile* m_xxLegs;
	BYTE m_unknown6[0x130];
	Bone** m_bonePtrArray; //first one is neck (focused on q press), second one is spin (focused on w press)
	Bone** m_bonePtrArrayEnd; //(exclusive, not part of array anymore)
	BYTE m_unknown7[0xDB8];
	void* m_somedata;
	BYTE m_unknown8[0x1C];
	XXFile* m_xxSkirt;
	BYTE m_unknown9[0x18];

public:
	CharacterStruct() = delete;
	~CharacterStruct() = delete;
};

static_assert(sizeof(CharacterStruct) == 0xF9C, "CharacterStruct size missmatch; must be 0xF9C bytes (allocation size)");


#pragma pack(pop)
}
