#pragma once

#include <Windows.h>
#include "Script/ScriptLua.h"

namespace ExtClass {

#pragma pack(push, 1)

class NpcData
{
public:
	BYTE m_unknown[0x26C];
	NpcData* m_target;

	static inline void bindLua() {
#define LUA_CLASS ExtClass::NpcData
		LUA_NAME;
		LUA_BIND(m_target)
#undef LUA_CLASS
	}
};

#pragma pack(pop)
}
