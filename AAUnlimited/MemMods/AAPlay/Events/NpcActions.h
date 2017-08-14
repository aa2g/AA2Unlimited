#pragma once

#include "External/ExternalClasses/CharacterActivity.h"
#include "External/ExternalClasses/CharacterStruct.h"
#include "Script/ScriptLua.h"


namespace PlayInjections {
namespace NpcActions {


	void NpcAnswerInjection();
	void NpcMovingActionInjection();
	void NpcMovingActionPlanInjection();

#pragma pack(push, 1)
	struct AnswerStruct {
		BYTE unk0[28];

		DWORD answer; // + 28
		DWORD unk32; // +32
		DWORD conversationId; // +36, on entry says what is going on, it is later set to -1

							  // Mystery
		BYTE unk3[400 - 12];

		// These dwords seem to be used for something
		DWORD unk428;
		DWORD unk432;
		DWORD unk436;
		ExtClass::CharacterActivity *answerChar; // + 440
		ExtClass::CharacterActivity *askingChar; // + 444
		DWORD unk448;

		// Rest is a mystery too
		static inline void bindLua() {
#define LUA_CLASS AnswerStruct
			LUA_NAME;
			LUA_BIND(answer);
			LUA_BIND(conversationId);
			LUA_BIND(answerChar);
			LUA_BIND(askingChar);
#undef LUA_CLASS
		}
	};
#pragma pack(pop)
}
}