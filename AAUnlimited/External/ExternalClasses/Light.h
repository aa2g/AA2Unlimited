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

		DWORD m_origMaterialCount; // material count?
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
		DWORD m_materialCount; // again material count?
		LightMaterial m_material[3]; // material used by the game

		float m_posX;
		float m_posY;
		float m_posZ;

		float m_float1;
		float m_float2;
		float m_float3;

		// ...
		BYTE m_unknown1[0x20];
		union {
			float m_origLightArray[16];
			float m_origLightMatrix[4][4];
		};
		union {
			float m_lightArray[16];
			float m_lightMatrix[4][4];
		};
		BYTE m_unknown2[0x2c];

#define LUA_CLASS ExtClass::Light
		static inline void bindLua() {
			LUA_BINDSTRP(m_name);
			LUA_BIND(m_materialCount);
			LUA_BINDARRP(m_material);
			LUA_BIND(m_posX);
			LUA_BIND(m_posY);
			LUA_BIND(m_posZ);
			LUA_BINDARR(m_origLightArray);
			LUA_BINDARR(m_lightArray);
			LUA_METHOD(SetLightDirection, {
				float x = _gl.get(2);
				float y = _gl.get(3);
				float z = _gl.get(4);
				float w = _gl.get(5);
				_self->m_lightMatrix[2][0] = x;
				_self->m_lightMatrix[2][1] = y;
				_self->m_lightMatrix[2][2] = z;
				_self->m_lightMatrix[2][3] = w;
			});
			LUA_METHOD(SetLightMaterialColor, {
				int material = _gl.get(2);
				float r = _gl.get(3);
				float g = _gl.get(4);
				float b = _gl.get(5);
				float a = _gl.get(6);
				_self->m_material[material].m_materialRed = r;
				_self->m_material[material].m_materialGreen = g;
				_self->m_material[material].m_materialBlue = b;
				_self->m_material[material].m_materialAlpha = a;
			});
			LUA_METHOD(GetLightDirection, {
				float x = _self->m_lightMatrix[2][0];
				float y = _self->m_lightMatrix[2][1];
				float z = _self->m_lightMatrix[2][2];
				float w = _self->m_lightMatrix[2][3];
				_gl.push(x).push(y).push(z).push(w);
				return 4;
			});
			LUA_METHOD(GetLightMaterialColor, {
				int material = _gl.get(2);
				float r = _self->m_material[material].m_materialRed;
				float g = _self->m_material[material].m_materialGreen;
				float b = _self->m_material[material].m_materialBlue;
				float a = _self->m_material[material].m_materialAlpha;
				_gl.push(r).push(g).push(b).push(a);
				return 4;
			});
		}
#undef LUA_CLASS

	};
#pragma pack(pop)

	static_assert(sizeof(Light) == 0x18c, "Material size mismatch; must be 0x134 bytes");

};
