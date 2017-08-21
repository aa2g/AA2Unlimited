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
#include "XXFileFace.h"
#include "Script/ScriptLua.h"

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
//	void* m_virtualTable;

	// Known caveats:
	// - Despawn must be called in reverse-spawn order
	// - Materials will be all over place, the clones must be same "family"

	// original game vtable
	/* #0 */ virtual DWORD Destroy(int free); // destroys the character, 1 frees it too i think
	/* #1 */ virtual DWORD Spawn(BYTE clothstate, BYTE a3, BYTE light, BYTE partial); // Loads character on scene
	/* #2 */ virtual DWORD Update(BYTE clothstate, BYTE playormake); // second argument indicates if maker/play pp
	/* #3 */ virtual DWORD fn3();
	/* #4 */ virtual DWORD Despawn(); // removes the character from scene
	/* #5 */ virtual DWORD fn5();
	/* #6 */ virtual DWORD Despawn2(); // called together with Despawn, frees memory?
	/* #7 */ virtual DWORD fn7();
	/* #8 */ virtual DWORD fn8();
	/* #9 */ virtual DWORD Skeleton(const wchar_t *pp, const wchar_t *xa, int pose, int z0, int z1);

	BYTE m_unknown1[0x24];
	CharacterData* m_charData; // 0x28
	void* m_somePointer; // 0x2c
	void* m_somePointer2;
	void* m_somePointer3;
	void* m_somePointer4;
	int m_seat; //seat number; from top to bottom, right to left, zero based, teacher is exception and 24 //3c
	BYTE m_boobs; // weird female boobs state. girls generally have some, boys dont. // 40
	BYTE m_clothState;
	BYTE m_bClothesOn;
	BYTE m_currClothSlot;
	BYTE m_currClothes;
	BYTE m_unknown5[3];
	XXFile* m_xxFace; //0x48 certain pointers to model files. all of these may be NULL if they are not loaded yet or not used

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

#define LUA_CLASS ExtClass::CharacterStruct
	static inline void bindLua() {
	LUA_NAME;
	LUA_METHOD(Destroy, {
		return _gl.push(_self->Destroy(_gl.get(1))).one;
	});
	LUA_METHOD(Spawn, {
		//__debugbreak();
		return _gl.push(_self->Spawn(_gl.get(2), _gl.get(3), _gl.get(4), _gl.get(5))).one;
	});
	LUA_METHOD(Update, {
		//__debugbreak();
		return _gl.push(_self->Update(_gl.get(2), _gl.get(3))).one;
	});
	LUA_METHOD(Skeleton, {
		//__debugbreak();
		const char *a = _gl.get(2);
		const char *b = _gl.get(3);
		std::wstring aw = General::utf8.from_bytes(a);
		std::wstring bw = General::utf8.from_bytes(b);
		return _gl.push(_self->Skeleton(aw.c_str(),bw.c_str(),_gl.get(4),0,0)).one;
	});
	LUA_METHOD(Despawn, {
		_gl.push(_self->Despawn());
		_gl.push(_self->Despawn2());
	});
	LUA_BINDSTR(m_unknown1)
	LUA_BINDSTR(m_unknown5)
	LUA_BINDSTR(m_unknown6)
	LUA_BIND(m_somePointer)
	LUA_BIND(m_somePointer2)
	LUA_BIND(m_somePointer3)
	LUA_BIND(m_somePointer4)
	LUA_BIND(m_charData)
	LUA_BIND(m_seat)
	LUA_BIND(m_boobs)
	LUA_BIND(m_clothState)
	LUA_BIND(m_bClothesOn)
	LUA_BIND(m_currClothSlot)
	LUA_BIND(m_currClothes)
	LUA_BIND(m_xxFace)
	LUA_BIND(m_xxGlasses)
	LUA_BINDARR(m_xxHairs)
	LUA_BIND(m_xxTounge)
	LUA_BIND(m_xxSkeleton)
	LUA_BIND(m_xxBody)
	LUA_BIND(m_xxLegs)
	LUA_BINDARRE(m_bonePtrArray,,_self->m_bonePtrArrayEnd-_self->m_bonePtrArray)
	LUA_BIND(m_hStats)

	LUA_MGETTER0(GetActivity)
	LUA_MGETTER1(GetXXFile)
	LUA_MGETTER1(GetRelation)
	LUA_MGETTER1(GetLover)
	LUA_MGETTER0(GetNpcReactData)
	LUA_MGETTER0(GetNpcAiData)
	LUA_MGETTER0(GetXXFileFace)

	}
#undef LUA_CLASS
	inline XXFileFace *GetXXFileFace() {
		return (XXFileFace*)m_xxFace;
	}

	inline CharacterRelation *GetRelation(int idx) {
		auto &rel = *GetRelations();
		if (idx >= rel.GetSize())
			return NULL;
		return &rel[idx];
	}
	inline LoverData *GetLover(int idx) {
		auto &rel = *GetLovers();
		if (idx >= rel.GetSize())
			return NULL;
		return &rel[idx];
	}

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

	inline XXFile* GetXXFile(int target) {
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
		case TONGUE:
			return m_xxTounge;
			break;

		default:
			return NULL;
		}
	}
};

static_assert(sizeof(CharacterStruct) == 0xF9C, "CharacterStruct size missmatch; must be 0xF9C bytes (allocation size)");


#pragma pack(pop)
}
