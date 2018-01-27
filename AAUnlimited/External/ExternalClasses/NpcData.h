#pragma once

#include <Windows.h>
#include "Script/ScriptLua.h"

namespace ExtClass {

#pragma pack(push, 1)

class NpcData
{
public:
	DWORD m_unknown[82];
	DWORD* routeStart;
	DWORD* routeEnd;
	DWORD* routeUnknown;
	DWORD m_unknown1[70];
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
