#pragma once

#include <Windows.h>
#include <stdint.h>

#include "Script/ScriptLua.h"

namespace ExtClass {

#pragma pack(push, 1)
	/*
	* Represents a light source
	*/
	class Light
	{
	public:
		struct LightMaterial {
			float m_materialRed;
			float m_materialGreen;
			float m_materialBlue;
			float m_materialAlpha;
#define LUA_CLASS ExtClass::Light::LightMaterial
			static inline void bindLua() {
				LUA_BIND(m_materialRed)
				LUA_BIND(m_materialGreen)
				LUA_BIND(m_materialBlue)
				LUA_BIND(m_materialAlpha)
			}
#undef LUA_CLASS
		};

		size_t m_nameBufferSize;
		char* m_name; // xltool light name

		DWORD m_someFlag0; // first xltool data row

		DWORD m_materialCount1; // material count?
		LightMaterial m_origMaterial[3]; // material data as is in .xl file

		float m_origPosX; // fifth xltool data row
		float m_origPosY;
		float m_origPosZ;

		float m_origNormalX;
		float m_origNormalY;
		float m_origNormalZ;

		// xltool 7th row
		float m_unknownFloat1;
		float m_unknownFloat2;
		float m_unknownFloat3;
		float m_unknownFloat4;
		float m_unknownFloat5;
		float m_unknownFloat6;
		float m_unknownFloat7;

		// it seems to repeat the previous structure above including the previous 7 float group
		DWORD m_materialCount2; // again material count?
		LightMaterial m_material[3]; // material used by the game

		float m_posX;
		float m_posY;
		float m_posZ;

		float m_float1;
		float m_float2;
		float m_float3;

		// ...

		BYTE m_unknown[0xCC];

#define LUA_CLASS ExtClass::Light
		static inline void bindLua() {
			LUA_BINDSTRP(m_name)
			LUA_BINDARRP(m_material)
			LUA_BIND(m_posX)
			LUA_BIND(m_posY)
			LUA_BIND(m_posZ)
		}
#undef LUA_CLASS
	};
#pragma pack(pop)

static_assert(sizeof(Light) == 0x18c, "Material size mismatch; must be 0x134 bytes");

};
