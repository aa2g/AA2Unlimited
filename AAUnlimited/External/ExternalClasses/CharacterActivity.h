#pragma once

#include <Windows.h>

namespace ExtClass {

class CharacterStruct;

#pragma pack(push, 1)
class CharacterActivity {
public:
	DWORD m_virtualTable;
	BYTE m_unknown1[0x10];
	BYTE m_animLocked;	//?
	BYTE m_loading; //statue or conversation loading?
	BYTE m_unknown2[3];
	BYTE m_isInConversation;
	BYTE m_conversationLoading; //?
	BYTE m_currentlyTalking; //?
	BYTE m_unknown3[0x20];
	BYTE m_currentMovementType; //0=stand, 1=move, 2=roam, 3=walk to character, 4 = follow, 7 = talk, 8=mina
	BYTE m_unknown4[3];
	DWORD m_currConversationId; //if this is an npc that plans a conversation, this is set once the character starts walking towards its target
	DWORD m_unknown5;
	DWORD m_currRoomTarget; //similarily, set when getting the command to walk to a room
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
};

static_assert(sizeof(CharacterActivity) == 0x104,"size mismatch");

#pragma pack(pop)
}
