#pragma once
#include <vector>

#include "Files/PNGData.h"
namespace ExtClass {

	struct ActionParamStruct {
		DWORD conversationId;// = -1;
		DWORD movementType;
		DWORD roomTarget;
		DWORD unknown; //always -1
		CharacterActivity* target1;
		CharacterActivity* target2;
		DWORD unknown2; //always 1, though initialized to 2
		static inline void bindLua() {
#define LUA_CLASS ExtClass::ActionParamStruct
			LUA_NAME;
			LUA_BIND(conversationId)
				LUA_BIND(movementType)
				LUA_BIND(roomTarget)
				LUA_BIND(target1)
				LUA_BIND(target2)
				LUA_BIND(unknown2)
		}
#undef LUA_CLASS
	};
}