#pragma once

#include <vector>

#include "External\ExternalClasses\CharacterStruct.h"
#include "Functions\AAUCardData.h"
#include "Files/PNGData.h"
#include "External\ExternalClasses\ActionParamStruct.h"
#include "External\ExternalClasses\Light.h"

/*
 * Contains data about a character that currently exists in a game.
 * Apart from the pointer to the original struct, this includes the AAU card data,
 * currently loaded hair pointers etc
 */

class CharInstData
{
public:
//	struct ActionParamStruct {
//		DWORD conversationId;// = -1;
//		DWORD movementType;
//		DWORD roomTarget;
//		DWORD unknown; //always -1
//		ExtClass::CharacterActivity* target1;
//		ExtClass::CharacterActivity* target2;
//		DWORD unknown2; //always 1, though initialized to 2
//		static inline void bindLua() {
//#define LUA_CLASS CharInstData::ActionParamStruct
//			LUA_NAME;
//			LUA_BIND(conversationId)
//			LUA_BIND(movementType)
//			LUA_BIND(roomTarget)
//			LUA_BIND(target1)
//			LUA_BIND(target2)
//			LUA_BIND(unknown2)
//		}
//#undef LUA_CLASS
//	};

public:

	CharInstData();
	~CharInstData();


	ExtClass::CharacterStruct* m_char;
	AAUCardData m_cardData;

	std::vector<std::pair<AAUCardData::HairPart,ExtClass::XXFile*>> m_hairs[4];

	ExtClass::ActionParamStruct m_forceAction;

	int idxSave; //Illusion's load order

	void SetCurrentStyle(int index);
	int GetStyleCount();
	int GetCurrentStyle();
	const char* GetStyleName(int index);

	ExtClass::Light* GetLightArray();
	int GetLightIndex(const char* name);
	void SetLightMaterialColor(int light, int materialSlot, float red, float green, float blue, float alpha);
	void SetLightDirection(int light, float x, float y, float z, float w);
	void SetAmbientColor(BYTE red, BYTE green, BYTE blue);
	std::vector<float> GetLightMaterialColor(int light, int materialSlot);
	std::vector<float> GetLightDirection(int light);
	std::vector<BYTE> GetAmbientColor();

	int GetLoveTowards(CharInstData* towards);
	int GetLikeTowards(CharInstData* towards);
	int GetDislikeTowards(CharInstData* towards);
	int GetHateTowards(CharInstData* towards);

	void SetPointsTowards(CharInstData* towards, float love, float like, float dislike, float hate, float spare);
	bool IsPC();

	int GetCurrentRoom();
	CharInstData* GetTargetInst();

	void Reset();
	void StoreInitialStats();
	bool IsValid();

	inline bool Editable() {
		const DWORD femaleRule[]{ 0x353254, 0x2C, 0 };
		const DWORD maleRule[]{ 0x353254, 0x30, 0 };
		m_char = (ExtClass::CharacterStruct*) ExtVars::ApplyRule(femaleRule);
		if (m_char == NULL) m_char = (ExtClass::CharacterStruct*) ExtVars::ApplyRule(maleRule);
		return m_char != NULL;
	}

	inline bool LoadAAUData() {
		bool ok = m_cardData.FromPNGBuffer((char*)m_char->m_charData->m_pngBuffer, m_char->m_charData->m_pngBufferSize);
		if (ok && m_cardData.m_version < m_cardData.CurrentVersion)
			Shared::PNG::SavePNGChunk(m_char, (BYTE**)&m_char->m_charData->m_pngBuffer, &m_char->m_charData->m_pngBufferSize);
		return ok;
	}

	void ApplyDecals(int bodyPart, int decalStrength);

	DWORD* lastDialogue = NULL;

	static inline void bindLua() {
#define LUA_CLASS CharInstData
		LUA_NAME;
		LUA_BIND(m_char);
		LUA_METHOD(IsValid, {
			return _gl.push(_self->IsValid()).one;
		});
		LUA_METHOD(IsPC, {
			return _gl.push(_self->IsPC()).one;
		});
		LUA_METHOD(GetCurrentRoom, {
			return _gl.push(_self->GetCurrentRoom()).one;
		});
		LUA_METHOD(SetCurrentStyle, {
			_self->SetCurrentStyle(_gl.get(2));
		});
		LUA_METHOD(GetStyleCount, {
			return _gl.push(_self->GetStyleCount()).one;
		});
		LUA_METHOD(GetCurrentStyle, {
			return _gl.push(_self->GetCurrentStyle()).one;
		});
		LUA_METHOD(GetStyleName, {
			return _gl.push(_self->GetStyleName(_gl.get(2))).one;
		});
		LUA_METHOD(GetLightIndex, {
			const char * name = _gl.get(2);
			return _gl.push(_self->GetLightIndex(name)).one;
		});
		LUA_METHOD(GetLightMaterialColor, {
			int light = _gl.get(2);
			int material = _gl.get(3);
			std::vector<float> color = _self->GetLightMaterialColor(light, material);
			float r = color[0];
			float g = color[1];
			float b = color[2];
			float a = color[3];
			_gl.push(r).push(g).push(b).push(a);
			return 4;
		});
		LUA_METHOD(GetLightDirection, {
			int light = _gl.get(2);
			std::vector<float> direction = _self->GetLightDirection(light);
			float x = direction[0];
			float y = direction[1];
			float z = direction[2];
			float w = direction[3];
			_gl.push(x).push(y).push(z).push(w);
			return 4;
		});
		LUA_METHOD(GetAmbientColor, {
			std::vector<BYTE> color = _self->GetAmbientColor();
			float r = color[0];
			float g = color[1];
			float b = color[2];
			_gl.push(r).push(g).push(b);
			return 3;
		});
		LUA_METHOD(SetLightMaterialColor, {
			int light = _gl.get(2);
			int material = _gl.get(3);
			float r = _gl.get(4);
			float g = _gl.get(5);
			float b = _gl.get(6);
			float a = _gl.get(7);
			_self->SetLightMaterialColor(light, material, r, g, b, a);
		});
		LUA_METHOD(SetLightDirection, {
			int light = _gl.get(2);
			float x = _gl.get(3);
			float y = _gl.get(4);
			float z = _gl.get(5);
			float w = _gl.get(6);
			_self->SetLightDirection(light, x, y, z, w);
		});
		LUA_METHOD(SetAmbientColor, {
			BYTE r = _gl.get(2);
			BYTE g = _gl.get(3);
			BYTE b = _gl.get(4);
			_self->SetAmbientColor(r, g, b);
		});
		LUA_METHOD(GetLikeTowards, {
			return _gl.push(_self->GetLikeTowards(_gl.get(2))).one;
		});
		LUA_METHOD(GetLoveTowards, {
			return _gl.push(_self->GetLoveTowards(_gl.get(2))).one;
		});
		LUA_METHOD(GetDislikeTowards, {
			return _gl.push(_self->GetDislikeTowards(_gl.get(2))).one;
		});
		LUA_METHOD(GetHateTowards, {
			return _gl.push(_self->GetHateTowards(_gl.get(2))).one;
		});
		LUA_METHOD(SetPointsTowards, {
			_self->SetPointsTowards(_gl.get(2), _gl.get(3), _gl.get(4), _gl.get(5), _gl.get(6), _gl.get(7));
		});
		LUA_METHOD(ApplyDecal, {
			int bodyPart = _gl.get(2);
			int strength = _gl.get(3);
			if (bodyPart >= 0 && bodyPart < 5 && strength >= 0 && strength < 4)
				_self->ApplyDecals(bodyPart, strength);
		});
#undef LUA_CLASS
	}

};

