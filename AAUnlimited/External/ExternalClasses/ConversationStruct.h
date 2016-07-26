#pragma once
#include <Windows.h>

namespace ExtClass {

#pragma pack(push, 1)
class ConversationSubStruct;
/*
 * Saves data about the currently ongoing conversation of the PC with a NPC
 */
class ConversationStruct
{
public:
	void* m_virtualTable;
	DWORD m_askWaitTime; //how long the npc has been waiting for you to make a question if you talked to it. Stops at 10000
	BYTE m_unknown1[0x10];
	DWORD m_answerWaitTime; //how long the npc has been waiting for an answer. if a certain time has been waited, an automatic decision will be made.
public:
	ConversationStruct() = delete;
	~ConversationStruct() = delete;

	inline ConversationSubStruct* getSubStruct() {
		return ((ConversationSubStruct**)m_virtualTable)[1];
	}
};

/*
 * Referenced from within the struct; most likely polymorphic or something, as it seems to be
 * at a current offset of 0x5B60, yet the offset is taken out of the virtual table at [[ConversationStruct]+4]
 */
class ConversationSubStruct {
public:
	BYTE m_unknown2[0x2C];
	BYTE m_unknown3;
	BYTE m_unknown4;
	BYTE m_bCurrentlyAnswering;
	BYTE m_unknown5;
	DWORD m_response; //0 = negative, 1 = positive response
	DWORD m_npcTalkState; //0 = still speaking, 1 = waiting for answer, 2/3 = answering/end?
	BYTE m_unknown6[4];
	DWORD m_conversationState; //some are a series of animations and text; this one indicates where in the conversation the npc is
	DWORD m_unknown7;
	DWORD m_conversationId; //unique identifier of the kind of conversation going on
public:
	ConversationSubStruct() = delete;
	~ConversationSubStruct() = delete;
};

static_assert(sizeof(ConversationSubStruct) == 0x48,"Invalid size");

enum ConversationId {
	ENCOURAGE = 0,
	TALK_HOBBIES = 10,
	LETS_GO_CLASS = 24,
	KARAOKE = 30,
	INSULT = 33,
	GROPE = 47,
	NEVERMIND = 52,
	FORCE = 56,
	NORMAL_H = 72,
	TRADE = 78,
	EXPLOITABLE_LINE = 90,
	AFTER_H = 101
};



#pragma pack(pop)

}