#pragma once
#include <Windows.h>

#include "CharacterNpcReactData.h"
#include "CharacterNpcAiData.h"
#include "CharacterRelation.h"
#include "LoverData.h"
#include "External\AddressRule.h"
#include "Frame.h"
#include "CharacterData.h"
#include "CharacterActivity.h"
#include "HStatistics.h"
#include "XXFile.h"

namespace ExtClass {


#pragma pack(push, 1)
/*
 * Represents a character in the game and its state.
 */
class CharacterStruct
{
public:
	enum Models {
		FACE,SKELETON,BODY,HAIR_FRONT,HAIR_SIDE,HAIR_BACK,HAIR_EXT,
		FACE_SLIDERS,SKIRT,
		N_MODELS,
		TONGUE,
		GLASSES,
		H3DROOM,
		INVALID
	};

public:
	void* m_virtualTable;
	BYTE m_unknown1[0x24];
	CharacterData* m_charData;
	void* m_somePointer;
	BYTE m_unknown2[12];
	int m_seat; //seat number; from top to bottom, right to left, zero based, teacher is exception and 24
	BYTE m_unknown3[2];
	BYTE m_bClothesOn;
	BYTE m_currClothSlot;
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
	Frame** m_bonePtrArray; //note that this is an array of only certain frequently used frames with a fixed position; the bone might be NULL thought.
							//first one is neck (focused on q press), second one is spin (focused on w press), 10th (0x24 offset) is tears
	Frame** m_bonePtrArrayEnd; //(exclusive, not part of array anymore)
	BYTE m_unknown7[0xDB4];
	void* m_somedata;
	void* m_moreUnknownData;
	void* m_moreData;		//where m_moreData+0x16A18 is pointer to array of CharacterRelation, m_moreData+0x16A1C is end (typical array structure)
	BYTE m_unknown8_2[0x4];
	HStatistics* m_hStats;
	BYTE m_unknown9[0x4];
	void* m_moreData2;		//m_moreData2+0x18 is pointer to CharacterActivity struct
	BYTE m_unknown10[0x4];
	XXFile* m_xxSkirt;
	BYTE m_unknown11[0x1C];


public:
	CharacterStruct() = delete;
	~CharacterStruct() = delete;

	inline IllusionArray<CharacterRelation>* GetRelations() {
		BYTE* ptr = (BYTE*)m_moreData;
		return (IllusionArray<CharacterRelation>*)(ptr + 0x16A14);
	}
	
	inline CharacterActivity* GetActivity() {
		if (m_moreData2 == NULL) return NULL;
		BYTE* ptr = (BYTE*)(m_moreData2)+0x18;
		return *(CharacterActivity**)(ptr);
	}

	inline IllusionArray<LoverData>* GetLovers() {
		if (m_moreData2 == NULL) return NULL;
		BYTE* ptr = (BYTE*)(m_moreData2)+0x20;
		return (IllusionArray<LoverData>*)ptr;
	}

	inline CharacterNpcReactData* GetNpcReactData() {
		if (m_somedata == NULL) return NULL;
		BYTE* ptr = (BYTE*)(m_somedata)+0x1C;
		return *(CharacterNpcReactData**)(ptr);
	}

	inline CharacterNpcAiData* GetNpcAiData() {
		if (m_somedata == NULL) return NULL;
		BYTE* ptr = (BYTE*)(m_somedata)+0x18;
		return *(CharacterNpcAiData**)(ptr);
	}

	inline XXFile* GetXXFile(Models target) {
		switch (target) {
		case FACE:
			return m_xxFace;
		case SKELETON:
			return m_xxSkeleton;
		case BODY:
			return m_xxBody;
		case HAIR_FRONT:
			return m_xxFrontHair;
		case HAIR_SIDE:
			return m_xxSideHair;
		case HAIR_BACK:
			return m_xxBackHair;
		case HAIR_EXT:
			return m_xxHairExtension;
		case FACE_SLIDERS: {
			DWORD rule[] {0x70, 4, 0};
			return (XXFile*)ExtVars::ApplyRule(this, rule);
			break; }
		default:
			return NULL;
		}
	}
};

static_assert(sizeof(CharacterStruct) == 0xF9C, "CharacterStruct size missmatch; must be 0xF9C bytes (allocation size)");


#pragma pack(pop)
}
