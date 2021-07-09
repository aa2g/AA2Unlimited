#pragma once
#include <Windows.h>

#include "HClasses\HPosData.h"
#include <Script/ScriptLua.h>

namespace ExtClass {

	enum Gender {
		GENDER_MALE = 0, GENDER_FEMALE = 1
	};

	enum PregnancyRisk {
		PREGRISK_DANGEROUS = 2, PREGRISK_NORMAL = 0, PREGRISK_SAFE = 1
	};

	enum Trait {
		TRAIT_EASYGOING, TRAIT_AFFABLE, TRAIT_BAD_WITH_GUYS, TRAIT_BAD_WITH_GIRLS, TRAIT_CHARMING, TRAIT_TSUNDERE,
		TRAIT_CHIVALROUS, TRAIT_TRENDY, TRAIT_OBEDIENT, TRAIT_POSITIVE, TRAIT_SHY, TRAIT_JEALOUS, TRAIT_MELANCHOLIC,
		TRAIT_PERVERTED, TRAIT_SERIOUS, TRAIT_CALM, TRAIT_IMPULSIVE, TRAIT_ABSENDMINDED, TRAIT_VIOLENT, TRAIT_PASSIVE,
		TRAIT_MEDDLESOME, TRAIT_CLASS_PREZ, TRAIT_CHATTY, TRAIT_ALWAYS_HUNGRY, TRAIT_ROMNATIC, TRAIT_SINGLEMINDED,
		TRAIT_INDECISIVE, TRAIT_COMPETIVIE, TRAIT_SCHEMING, TRAIT_DILIGENT, TRAIT_WILD, TRAIT_MASOCHIST, TRAIT_SWEATY,
		TRAIT_EVIL, TRAIT_DEAF, TRAIT_EXPLOITABLE, TRAIT_ASEXUAL, TRAIT_LUCKY, TRAIT_RAINBOW
	};

#pragma pack(push, 1)

	/*
	*Declares all the complex structs used by CharacterData and CardStyleData
	*/
	class CharacterDataTypes
	{
	public:
		struct Figure {
			BYTE height;
			BYTE figure;
			BYTE headSize;
			BYTE headLength;
			BYTE waist;
#define LUA_CLASS CharacterDataTypes::Figure
			static inline void bindLua() {
				LUA_NAME;
				LUA_BIND(height);
				LUA_BIND(figure);
				LUA_BIND(headSize);
				LUA_BIND(headLength);
				LUA_BIND(waist);
			}
#undef LUA_CLASS
		};
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
#define LUA_CLASS CharacterDataTypes::Chest
			static inline void bindLua() {
				LUA_NAME;
				LUA_BIND(size);
				LUA_BIND(aerolaSize);
				LUA_BIND(shape);
				LUA_BIND(roundness);
				LUA_BIND(direction);
				LUA_BIND(height);
				LUA_BIND(spacing);
				LUA_BIND(depth);
				LUA_BIND(softness);
			}
#undef LUA_CLASS
		};
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
#define LUA_CLASS CharacterDataTypes::BodyColor
			static inline void bindLua() {
				LUA_NAME;
				LUA_BIND(skinColor);
				LUA_BIND(pubShape);
				LUA_BIND(pubOpacity);
				LUA_BIND(unknown7);
				LUA_BIND(unknown8);
				LUA_BIND(pubicColor);
				LUA_BIND(nipType);
				LUA_BIND(nipColor);
				LUA_BIND(nipOpacity);
				LUA_BIND(tan);
				LUA_BIND(tanOpacity);
				LUA_BIND(mosaic);
			}
#undef LUA_CLASS
		};
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
#define LUA_CLASS CharacterDataTypes::Eyes::Iris
				static inline void bindLua() {
					LUA_BIND(shape);
					LUA_BIND(width);
					LUA_BIND(height);
					LUA_BIND(position);
				}
#undef LUA_CLASS
			} iris;
			BYTE textureSlot;
			BYTE hightlightSlot;
			BYTE bExtTextureUsed;
			BYTE bExtHighlightUsed;
			char texture[260];
			char highlight[260];
			DWORD leftColor;
			DWORD rightColor;
#define LUA_CLASS CharacterDataTypes::Eyes
			static inline void bindLua() {
				Iris::bindLua();
				LUA_NAME;
				LUA_BIND(textureSlot);
				LUA_BIND(hightlightSlot);
				LUA_BIND(bExtTextureUsed);
				LUA_BIND(bExtHighlightUsed);
				LUA_BINDSTR(texture);
				LUA_BINDSTR(highlight);
				LUA_BIND(leftColor);
				LUA_BIND(rightColor);
			}
#undef LUA_CLASS
		};
		struct Eyebrows {
			DWORD color;
			BYTE shapeSlot;
			BYTE angle;
			BYTE bMole1;
			BYTE bMole2;
			BYTE bMole3;
			BYTE bMole4;
#define LUA_CLASS CharacterDataTypes::Eyebrows
			static inline void bindLua() {
				LUA_NAME;
				LUA_BIND(color);
				LUA_BIND(shapeSlot);
				LUA_BIND(angle);
				LUA_BIND(bMole1);
				LUA_BIND(bMole2);
				LUA_BIND(bMole3);
				LUA_BIND(bMole4);
			}
#undef LUA_CLASS

		};
		struct FaceDetails {
			BYTE eyelidSlot;
			BYTE upperEyelidSlot;
			BYTE lowerEyelidSlot;
			BYTE glassesSlot;
			BYTE lipColorSlot;
			BYTE lipOpacity;
			DWORD glassesColor;
#define LUA_CLASS CharacterDataTypes::FaceDetails
			static inline void bindLua() {
				LUA_NAME;
				LUA_BIND(eyelidSlot);
				LUA_BIND(upperEyelidSlot);
				LUA_BIND(lowerEyelidSlot);
				LUA_BIND(glassesSlot);
				LUA_BIND(lipColorSlot);
				LUA_BIND(lipOpacity);
				LUA_BIND(glassesColor);
			}
#undef LUA_CLASS
		};
		struct Hair {
			union {
				struct {
					BYTE frontHair;
					BYTE sideHair;
					BYTE backhair;
					BYTE hairExtension;
				};
				BYTE hairs[4];
			};
			union {
				struct {
					BYTE frontHairAdjustment;
					BYTE sideHairAdjustment;
					BYTE backhairAdjustment;
					BYTE hairExtensionAdjustment;
				};
				BYTE hairAdjustment[4];
			};
			union {
				struct {
					BYTE frontHairFlip;
					BYTE sideHairFlip;
					BYTE backhairFlip;
					BYTE hairExtensionFlip;
				};
				BYTE hairFlips[4];
			};
			DWORD hairColor;
#define LUA_CLASS CharacterDataTypes::Hair
			static inline void bindLua() {
				LUA_NAME;
				LUA_BINDARR(hairs);
				LUA_BINDARR(hairAdjustment);
				LUA_BINDARR(hairFlips);
				LUA_BIND(hairColor);
			}
#undef LUA_CLASS
		};
		struct Clothes {
			static inline void bindLua() {
#define LUA_CLASS CharacterDataTypes::Clothes
			LUA_NAME
			LUA_BIND(slot)
			LUA_BIND(skirtLength)
			LUA_BIND(socks)
			LUA_BIND(indoorShoes)
			LUA_BIND(outdoorShoes)
			LUA_BIND(isOnePiece)
			LUA_BIND(hasUnderwear)
			LUA_BIND(hasSkirt)
			LUA_BIND(unknown13)
			LUA_BIND(colorTop1)
			LUA_BIND(colorTop2)
			LUA_BIND(colorTop3)
			LUA_BIND(colorTop4)
			LUA_BIND(colorBottom1)
			LUA_BIND(colorBottom2)
			LUA_BIND(colorUnderwear)
			LUA_BIND(colorSocks)
			LUA_BIND(colorIndoorShoes)
			LUA_BIND(colorOutdoorShoes)
			LUA_BIND(textureBottom1)
			LUA_BIND(textureUnderwear)
			LUA_BIND(textureBottom1Hue)
			LUA_BIND(textureBottom1Lightness)
			LUA_BIND(shadowBottom1Hue)
			LUA_BIND(shadowBottom1Lightness)
			LUA_BIND(textureUnderwearHue)
			LUA_BIND(textureUnderwearLightness)
			LUA_BIND(shadowUnderwearHue)
			LUA_BIND(shadowUnderwearLightness)
#undef LUA_CLASS
			}
			DWORD slot;
			BYTE skirtLength;
			BYTE socks;
			BYTE indoorShoes;
			BYTE outdoorShoes;
			BYTE isOnePiece;
			BYTE hasUnderwear;
			BYTE hasSkirt;
			BYTE unknown13;
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
		};
	public:

		//CharacterDataTypes() = delete;
		//~CharacterDataTypes() = delete;
	};
	class CardStyleData;
	class CharacterData;


	/*
	 * Used to Represent mostly fixed Data about a character, such as appearance,
	 * Personality and Clothes,
	 * but also some of  their volatile data, such as intelligence value.
	 * In essence, this seems to be a slightly misaligned version of the "advanced" tab
	 * from reiedit. i filled what members i could observe from the editor and filled in
	 * some interisting holes using reiedit, so im not sure if they are all correct.
	 *
	 * CJS_ChaEditInfo
	 */
	class CharacterData : public CharacterDataTypes
	{
	public:
		void* m_virtualTable; // 4
		BYTE m_unknown1[0x2C-8]; 
		DWORD m_pngRosterBufferSize;
		void* m_pngRosterBuffer; // roster image
		DWORD m_pngBufferSize; // 0x30
		void* m_pngBuffer; // 0x34 card image, exluding the aa2 data (just the image part)
		BYTE m_unknown2[4]; // 0x38
		BYTE m_gender; //0x3c, 0=male, 1=female. no tumbler, thats all.
		char m_forename[260]; // 0x3d	//Family Name
		char m_surname[260];			//Given Name
		char m_description[260];
		BYTE m_unknown3[0xFC];
		BYTE m_bPersonality;
		BYTE m_club; //0 based index - Running, swimming, outdoorball, indoorball, fighting, music, jap, random
		BYTE m_padding1;
		BYTE m_unknown5;
		Figure m_figure;
		Chest m_chest;
		BYTE m_unknown6;
		BodyColor m_bodyColor;
		BYTE m_faceSlot;
		Eyes m_eyes;
		Eyebrows m_eyebrows;
		FaceDetails m_faceDetails;
		Hair m_hair;
		struct Character {
#define LUA_CLASS ExtClass::CharacterData::Character
			static inline void bindLua()
			{
			LUA_BIND(intelligence)
			LUA_BIND(intelligenceClassRank)
			LUA_BIND(intelligenceValue)
			LUA_BIND(strength)
			LUA_BIND(strengthClassRank)
			LUA_BIND(strengthValue)
			LUA_BIND(clubClassRanking)
			LUA_BIND(clubValue)
			LUA_BIND(sociability)
			LUA_BIND(fightingStyle)
			LUA_BIND(virtue)
			LUA_BIND(orientation)
			LUA_BIND(h_experience)
			LUA_BIND(a_h_experience)
#undef LUA_CLASS
			}

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
		BYTE m_reservedPreferenceBools[3];
		BYTE m_pregnancyRisks[14]; //2 = dangerous, 1 = safe, 0 = normal
		DWORD m_hCompatibility[25]; //index is seat order, which counts from top -> bottom, then right -> left
		char m_item1[260];
		char m_item2[260];
		char m_item3[260];
		Clothes m_clothes[4];
	public:
#define LUA_CLASS ExtClass::CharacterData
		static inline void bindLua()
		{
			Character::bindLua();
			Clothes::bindLua();
			Hair::bindLua();
			FaceDetails::bindLua();
			Eyebrows::bindLua();
			Eyes::bindLua();
			BodyColor::bindLua();
			Chest::bindLua();
			Figure::bindLua();

			LUA_NAME;

			LUA_BINDP(m_figure);
			LUA_BINDP(m_chest);
			LUA_BINDP(m_bodyColor);
			LUA_BINDP(m_eyes);
			LUA_BINDP(m_eyebrows);
			LUA_BINDP(m_faceDetails);
			LUA_BINDP(m_hair);

			LUA_BINDP(m_character);
			LUA_BIND(m_voicePitch);
			LUA_BINDARR(m_traitBools);
			LUA_BIND(m_faceSlot);
			LUA_BIND(m_club);
			LUA_BIND(m_bPersonality);
			LUA_BIND(m_gender);
			LUA_BINDSTR(m_forename);
			LUA_BINDSTR(m_surname);
			LUA_BINDSTR(m_description);;
			LUA_METHOD(GetPreferenceFlags, { return _gl.push(_self->GetPreferenceFlags()).one; });
			LUA_BINDARR(m_preferenceBools);
			LUA_BINDARR(m_pregnancyRisks);
			LUA_BINDARR(m_hCompatibility);
			LUA_BINDSTR(m_item1);
			LUA_BINDSTR(m_item2);
			LUA_BINDSTR(m_item3);
			LUA_BINDARRP(m_clothes);

			LUA_BIND(m_pngBuffer);
			LUA_BIND(m_pngBufferSize);
			LUA_BIND(m_pngRosterBuffer);
			LUA_BIND(m_pngRosterBufferSize);
		}
#undef LUA_CLASS

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

		void CopyCharacterSetData(CardStyleData* data);
	};

	class CardStyleData : public CharacterDataTypes
	{
	public:
		BYTE m_gender; //0=male, 1=female. no tumbler, thats all.
		Figure m_figure;
		Chest m_chest;
		BodyColor m_bodyColor;
		BYTE m_faceSlot;
		Eyes m_eyes;
		Eyebrows m_eyebrows;
		FaceDetails m_faceDetails;
		Hair m_hair;
		Clothes m_clothes[4];
		BYTE m_unknown12[0x3]; //more padding i guess
	public:

		//CardStyleData() = delete;
		//~CardStyleData() = delete;

		bool empty() { return false; };
		void CopyCharacterData(CharacterData* data);
		void CopyCharacterFigureData(CharacterData* data);
		void CopyCharacterFaceData(CharacterData* data);
		void CopyCharacterEyesData(CharacterData* data);
		void CopyCharacterHairData(CharacterData* data);
	};

	class CardStyleDataUnpadded : public CharacterDataTypes
	{
	public:
		BYTE m_gender; //0=male, 1=female. no tumbler, thats all.
		Figure m_figure;
		Chest m_chest;
		BodyColor m_bodyColor;
		BYTE m_faceSlot;
		Eyes m_eyes;
		Eyebrows m_eyebrows;
		FaceDetails m_faceDetails;
		Hair m_hair;
		Clothes m_clothes[4];
	public:

		//CardStyleData() = delete;
		//~CardStyleData() = delete;

		bool empty() { return false; };

		CardStyleData Pad() {
			CardStyleData padded;
			padded.m_gender = this->m_gender;
			padded.m_figure = this->m_figure;
			padded.m_chest = this->m_chest;
			padded.m_bodyColor = this->m_bodyColor;
			padded.m_faceSlot = this->m_faceSlot;
			padded.m_eyes = this->m_eyes;
			padded.m_eyebrows = this->m_eyebrows;
			padded.m_faceDetails = this->m_faceDetails;
			padded.m_hair = this->m_hair;	//should be deep copy
			for (int i = 0; i< 4; i++) padded.m_clothes[i] = this->m_clothes[i];
			return padded;
		}
	};

	// maybe 0xbf8
	static_assert(sizeof(CharacterData) == 0xBF0, "CharacterData size mismatch");

#pragma pack(pop)


}