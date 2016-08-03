#pragma once
#include <Windows.h>

#include "HClasses\HPosData.h"

namespace ExtClass {

enum Gender {
	GENDER_MALE = 0, GENDER_FEMALE = 1
};

enum PregnancyRisk {
	PREGRISK_DANGEROUS = 2, PREGRISK_NORMAL = 0, PREGRISK_SAFE = 1
};

#pragma pack(push, 1)
/*
 * Used to Represent mostly fixed Data about a character, such as appearance,
 * Personality and Clothes,
 * but also some of  their volatile data, such as intelligence value.
 * In essence, this seems to be a slightly misaligned version of the "advanced" tab
 * from reiedit. i filled what members i could observe from the editor and filled in
 * some interisting holes using reiedit, so im not sure if they are all correct.
 */
class CharacterData
{
public:
	void* m_virtualTable;
	BYTE m_unknown1[0x2C];
	DWORD m_pngBufferSize;
	void* m_pngBuffer; //card image, exluding the aa2 data (just the image part)
	BYTE m_unknown2[4];
	BYTE m_gender; //0=male, 1=female. no tumbler, thats all.
	char m_forename[260];
	char m_surname[260];
	char m_description[260];
	BYTE m_unknown3[0xFC];
	BYTE m_bPersonality;
	BYTE m_unknown4;
	BYTE m_padding1;
	BYTE m_unknown5;
	struct Figure {
		BYTE height;
		BYTE figure;
		BYTE headSize;
		BYTE headLength;
		BYTE waist;
	} m_figure;
	struct Chest {
		BYTE size;
		BYTE aerolaSize;
		BYTE shape;
		BYTE roundness;
		BYTE direction;
		BYTE height;
		BYTE spacing;
		BYTE depth;
		BYTE softness;
	} m_chest;
	BYTE m_unknown6;
	struct BodyColor {
		DWORD skinColor;
		BYTE pubShape;
		BYTE pubOpacity;
		BYTE unknown7;
		BYTE unknown8;
		DWORD pubicColor;
		BYTE nipType;
		BYTE nipColor;
		BYTE nipOpacity;
		BYTE tan;
		BYTE tanOpacity;
		BYTE mosaic;
	} m_bodyColor;
	BYTE m_faceSlot;
	struct Eyes {
		BYTE width;
		BYTE height;
		BYTE position;
		BYTE spacing;
		BYTE angle;
		struct Iris {
			BYTE shape;
			BYTE width;
			BYTE height;
			BYTE position;
		} iris;
		BYTE textureSlot;
		BYTE hightlightSlot;
		BYTE bExtTextureUsed;
		BYTE bExtHighlightUsed;
		char texture[260];
		char highlight[260];
		DWORD leftColor;
		DWORD rightColor;
	} m_eyes;
	struct Eyebrows {
		DWORD color;
		BYTE shapeSlot;
		BYTE angle;
		BYTE bMole1;
		BYTE bMole2;
		BYTE bMole3;
		BYTE bMole4;
	} m_eyebrows;
	struct FaceDetails {
		BYTE eyelidSlot;
		BYTE upperEyelidSlot;
		BYTE lowerEyelidSlot;
		BYTE glassesSlot;
		BYTE lipColorSlot;
		BYTE lipOpacity;
		DWORD glassesColor;
	} m_faceDetails;
	struct Hair {
		BYTE frontHair;
		BYTE sideHair;
		BYTE backhair;
		BYTE hairExtension;
		BYTE frontHairAdjustment;
		BYTE sideHairAdjustment;
		BYTE backhairAdjustment;
		BYTE hairExtensionAdjustment;
		BYTE frontHairFlip;
		BYTE sideHairFlip;
		BYTE backhairFlip;
		BYTE hairExtensionFlip;
		DWORD hairColor;
	} m_hair;
	struct Character {
		BYTE intelligence;
		BYTE intelligenceClassRank; //higher is better. assumed, taken from reiedit
		BYTE unknown9[2]; //i assume these are padding
		DWORD intelligenceValue;
		BYTE strength;
		BYTE strengthClassRank; //higher is better.
		BYTE unknown10[2];
		DWORD strengthValue;
		BYTE clubClassRanking; //higher is better
		BYTE unknown11[3];
		DWORD clubValue;
		BYTE sociability;
		BYTE fightingStyle;
		BYTE virtue;
		BYTE orientation;
		BYTE h_experience;
		BYTE a_h_experience;
	} m_character;
	BYTE m_voicePitch;
	BYTE m_traitBools[39]; //reiedit says last bool is rainbow card
	BYTE m_preferenceBools[13];
	BYTE m_pregnancyRisks[14]; //2 = dangerous, 1 = safe, 0 = normal
	BYTE m_unknown12[0x3]; //more padding i guess
	DWORD m_hCompatibility[25]; //index is seat order, which counts from top -> bottom, then right -> left
	char m_item1[260];
	char m_item2[260];
	char m_item3[260];
	struct Clothes {
		DWORD slot;
		BYTE skirtLength;
		BYTE socks;
		BYTE unknown13[6];
		DWORD colorTop1;
		DWORD colorTop2;
		DWORD colorTop3;
		DWORD colorTop4;
		DWORD colorBottom1;
		DWORD colorBottom2;
		DWORD colorUnderwear;
		DWORD colorSocks;
		DWORD colorIndoorShoes;
		DWORD colorOutdoorShoes;
		DWORD textureBottom1;
		DWORD textureUnderwear;
		DWORD textureBottom1Hue;
		DWORD textureBottom1Lightness;
		DWORD shadowBottom1Hue;
		DWORD shadowBottom1Lightness;
		DWORD textureUnderwearHue;
		DWORD textureUnderwearLightness;
		DWORD shadowUnderwearHue;
		DWORD shadowUnderwearLightness;
	} m_clothes[4];
public:
	
	inline DWORD GetPreferenceFlags() {
		DWORD retVal = 0;
		DWORD currFlag = 1;
		for (int i = 0; i < 13; i++) {
			if (m_preferenceBools[i]) retVal |= currFlag;
			currFlag <<= 1;
		}
		return retVal;
	}

	CharacterData() = delete;
	~CharacterData() = delete;
};

static_assert(sizeof(CharacterData) == 0xBF0,"CharacterData size mismatch");

#pragma pack(pop)


}