#pragma once
#include <Windows.h>

#include "../CharacterStruct.h"

namespace ExtClass {

#pragma pack(push, 1)
/*
 * Represents the data of one of the two participants in h
 */
class HParticipant
{
public:
	CharacterStruct* charPtr;
	BYTE unknown1[0x1C];
	DWORD nClimedInFront;
	BYTE unknown2[4];
	DWORD nClimedInRear;
	BYTE unknown3[0xC];
	DWORD nClim;
	DWORD nClimTogether;
	BYTE unknown4[0x14];
	DWORD nPosChanges;
	BYTE unknown5[8];
	DWORD nClimedOutside;
	BYTE unknown6[0xC];
	void* someSoundPointer;
	BYTE unknown7[0x14];
	DWORD nCremeOnFront;
	DWORD nCremeOn1;
	DWORD nCremeOn2;
	DWORD nCremeOnBack;
	DWORD nCremeOn3;
	WORD unknown8;
	WORD clothesState;
	BYTE bClothesSlipped;
	BYTE unknown9;
	BYTE bShoesOff;
public:
	HParticipant() = delete;
	~HParticipant() = delete;
};

static_assert(sizeof(HParticipant) == 0xA3,"HParticipant struct must have size of A3");

#pragma pack(pop)
}