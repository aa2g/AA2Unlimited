#pragma once

#include <Windows.h>
#include "Script/ScriptLua.h"

namespace ExtClass {

class CharacterStruct;

#pragma pack(push, 1)
class CharacterActivity {
public:
	DWORD m_virtualTable;
	BYTE m_unknown1[0x10]; // +4
	BYTE m_animLocked;	//? // +20
	BYTE m_loading; //statue or conversation loading?
	BYTE m_unknown2[3];
	BYTE m_isInConversation;
	BYTE m_conversationLoading; //?
	BYTE m_currentlyTalking; //?
	BYTE m_unknown3[0x3];
	BYTE m_interactionLock;
	BYTE m_unknown4[0x1C]; //37
	BYTE m_currentMovementType; //0=stand, 1=move, 2=roam, 3=walk to character, 4 = follow, 7 = talk, 8=mina
	BYTE m_unknown5[3];
	DWORD m_currConversationId; //if this is an npc that plans a conversation, this is set once the character starts walking towards its target
	DWORD m_actionAboutRoom; //This is room the NPC talks about in a come here event. Has seemingly no effect in follow_me_h and follow_me
	DWORD m_currRoomTarget; //similarily, set when getting the command to walk to a room
	DWORD m_unknown6;
	DWORD m_lastConversationSuccess; //(3=trumpet, 2 = nice, 1 = normal)
	BYTE m_unknown7[4];
	DWORD m_lastConversationAnswerPercent;
	DWORD m_lastConversationAnswer; //52
	BYTE m_unknown11[0xc];
	DWORD m_isMasturbating;
	BYTE m_unknown8[0x2c];
	DWORD m_idleAnimationProgress;
	BYTE m_unknown9[0x40];
	CharacterStruct* m_thisChar;
	BYTE m_unknown10[0x20];
	static inline void bindLua() {
#define LUA_CLASS ExtClass::CharacterActivity
		LUA_BIND(m_animLocked)
		LUA_BIND(m_loading)
		LUA_BIND(m_thisChar)
		LUA_BIND(m_isInConversation)
		LUA_BIND(m_conversationLoading)
		LUA_BIND(m_currentlyTalking)
		LUA_BIND(m_currentMovementType)
		LUA_BIND(m_currConversationId)
		LUA_BIND(m_currRoomTarget)

		LUA_BIND(m_lastConversationSuccess)
		LUA_BIND(m_lastConversationAnswerPercent)
		LUA_BIND(m_lastConversationAnswer)

		LUA_BIND(m_idleAnimationProgress)

		LUA_BIND(m_isMasturbating)

		LUA_BIND(m_thisChar)
#undef LUA_CLASS
	};
};

static_assert(sizeof(CharacterActivity) == 0x104,"size mismatch");

#pragma pack(pop)
}
