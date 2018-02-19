#pragma once

#include <Windows.h>
#include "Script/ScriptLua.h"

namespace ExtClass {

class CharacterStruct;

#pragma pack(push, 1)

// Same as CharacterActivity
// TODO: Merge with

class NpcStatus
{
public:
	/*
	// From TAA2
	BYTE m_unknown[0x38];
	DWORD m_reffTo;
	DWORD m_status;
	DWORD m_action;
	DWORD m_unknown1;
	DWORD m_location;
	DWORD m_unknown2;
	DWORD m_moodAction; //50
	DWORD m_unknown3;
	DWORD m_unknown4;
	DWORD m_response; //5C
	DWORD m_unknown5[6];
	DWORD m_instructionWait; //74
	DWORD m_instructionTime;
	DWORD m_fireAction;
	DWORD m_randomAction;
	*/
	DWORD m_virtualTable; //0
	BYTE m_unknown1[0x10]; //4
	BYTE m_animLocked;	//14
	BYTE m_loading; //statue or conversation loading?
	BYTE m_unknown2[3];
	BYTE m_isInConversation;
	BYTE m_conversationLoading; //?
	BYTE m_currentlyTalking; //?
	BYTE m_unknown3[28];
	CharacterActivity* m_refto;
	DWORD m_status; //3C //0=still, 1=settle in location, 2=move to location, 3=walk to character, 4=follow, 7=talk, 8=minna
	DWORD m_actionId;
	DWORD m_currConversationId; //if this is an npc that plans a conversation, this is set once the character starts walking towards its target
	DWORD m_locationTarget;
	DWORD m_unknown6;
	DWORD m_lastConversationSuccess; //(3=trumpet, 2 = nice, 1 = normal)
	BYTE m_unknown7[4];
	DWORD m_lastConversationAnswerPercent;
	DWORD m_lastConversationAnswer;
	BYTE m_unknown8[0x3C];
	DWORD m_idleAnimationProgress;
	BYTE m_unknown9[0x40];
	CharacterStruct* m_thisChar;
	BYTE m_unknown10[0x20];

	static inline void bindLua() {
#define LUA_CLASS ExtClass::NpcStatus
		LUA_NAME;
		LUA_BIND(m_animLocked)
		LUA_BIND(m_loading)
		LUA_BIND(m_thisChar)
		LUA_BIND(m_isInConversation)
		LUA_BIND(m_conversationLoading)
		LUA_BIND(m_currentlyTalking)
		LUA_BIND(m_status)
		LUA_BIND(m_actionId)
		LUA_BIND(m_currConversationId)
		LUA_BIND(m_locationTarget)

		LUA_BIND(m_lastConversationSuccess)
		LUA_BIND(m_lastConversationAnswerPercent)
		LUA_BIND(m_lastConversationAnswer)

		LUA_BIND(m_idleAnimationProgress)

		LUA_BIND(m_thisChar)
#undef LUA_CLASS
	}
};

#pragma pack(pop)
}
