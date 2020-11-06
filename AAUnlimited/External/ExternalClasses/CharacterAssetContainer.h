#pragma once
#pragma once

#include <Windows.h>
#include "Script/ScriptLua.h"

namespace ExtClass {

#pragma pack(push, 1)

	class CharacterAssetContainer
	{
	public:
		BYTE m_unknown[0x34c];
		static inline void bindLua() {
#define LUA_CLASS ExtClass::CharacterAssetContainer
			LUA_NAME;
#undef LUA_CLASS
		}
	};
	static_assert(sizeof(CharacterAssetContainer) == 0x34c, "CharacterAssetContainer size missmatch; must be 0x34c bytes (allocation size)");

#pragma pack(pop)
}
