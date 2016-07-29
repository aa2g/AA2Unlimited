#pragma once
#include <Windows.h>

namespace ExtClass {

/*
 * These Classes seem to be more complex.
 * First and foremost, the class is huge. Some members are beeing addressed as far as 0x839C from the sub-struct.
 * It contains several subclasses, some maybe overlapping, im not sure, that start at certain offsets.
 * 
 * Each of these subclasses start with a virtual table, whose second entry is the offset from this class
 * to the sub-struct. there are 13 of these subclasses,
 * at 0, C, 20, 286C, 2874, 287C, 2890, 2894, 289C, 28A4, 28AC, 28B8, 28C4, respectively.
 * ConversationSubStruct + 0x839C holds an integer that determines which of these substructs is to be used
 * for the last call (by the position in the above list).
 * 0: player talks to someone
 * 1: npc talks to player
 * 3: after h
 * 9: fight
 */

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
		return (ConversationSubStruct*)((BYTE*)(this) + ((DWORD*)m_virtualTable)[1]); //cast-madness
	}
};

class ConversationStructPcNpc {
public:
	void* m_virtualTable;
	DWORD m_unknown1;
	DWORD m_unknown2;
	ConversationStruct m_rest;

public:
	ConversationStructPcNpc() = delete;
};

/*
 * Referenced from within the struct; most likely polymorphic or something, as it seems to be
 * at a current offset of 0x5B60, yet the offset is taken out of the virtual table at [[ConversationStruct]+4]
 */
class ConversationSubStruct {
public:
	BYTE m_unknown[0x25];
	BYTE m_bStartH;
	BYTE m_unknown2[0x6];
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
	DWORD m_conversationAnswerId; //after the npc made a positive or negative answer, this id will be what he answers with.
	DWORD m_playerAnswer; //0 = positive, 1 = negative
public:
	inline int GetConversationKindInt() { return *(int*)((BYTE*)(this) + 0x839C); }
	ConversationSubStruct() = delete;
	~ConversationSubStruct() = delete;
};

static_assert(sizeof(ConversationSubStruct) == 0x50,"Invalid size");

//ids can be seen at https://docs.google.com/spreadsheets/d/1gwmoVpKuSuF0PtEPLEB17eK_dexPaKU106ShZEpBLhg/edit#gid=1592342892
enum ConversationId {
	ENCOURAGE = 0, CALM = 1, PRAISE = 2, GRUMBLE = 3, APOLOGIZE = 4,
	MAKE_STUDY = 5, MAKE_EXERCISE = 6, MAKE_CLUB = 7, MAKE_GET_ALONG = 8, MAKE_NO_PUBLIC_LEWD = 9,
	GOOD_RUMOR = 10,
	GET_ALONG_WITH = 11, I_WANNA_GET_ALONG_WITH = 12,
	BAD_RUMOR = 13,
	DO_YOU_LIKE = 14,
	TALK_LIFE = 15, TALK_HOBBIES = 16, TALK_FOOD = 17, TALK_LOVE = 18, TALK_LEWD = 19,
	STUDY_TOGETHER = 20, EXERCISE_TOGETHER = 21, CLUB_TOGETHER = 22,
	MASSAGE = 23, GOTO_CLASS = 24, LUNCH_TOGETHER = 25, TEA_TOGETHER = 26,
	GO_HOME_TOGETHER = 27, GO_PLAY_TOGETHER = 28, GO_EAT_TOGETHER = 29, GO_KARAOKE_TOGETHER = 30,
	STUDY_HOME = 31, STUDT_HOME_H = 32, INSULT = 33, FIGHT = 34, 
	MAKE_IGNORE = 35, FORCE_SHOW_THAT = 36, FORCE_PUT_THIS_ON = 37, FORCE_H = 38,
	MAKE_JOIN_CLUB = 39, ASK_DATE = 40, CONFESS = 41, ASK_COUPLE = 42, ASK_BREAKUP = 43,
	HEADPAT = 44, HUG = 45, KISS = 46, TOUCH = 47, NORMAL_H = 48,
	FOLLOW_ME = 49, 
	GO_AWAY = 50, COME_TO = 51, 
	
	COMPETE = 63,

	NEVERMIND = 66,

	REQUEST_MASSAGE = 69,
	REQEUST_KISS = 70,
	REQUEST_HUG = 71,
	SKIP_CLASS = 72, SKIP_CLASS_H = 73, SKIP_CLASS_SURPRISE_H = 74,
	DID_YOU_HAVE_H = 75,
	SHOW_UNDERWEAR = 76,
	HAVE_YOU_HAD_H = 77,
	EXCHANGE_ITEMS = 78,
	LEWD_PROMISE = 79,
	AWARD_LEWD_PROMISE = 80,

	EXPLOITABLE_LINE = 90,
	AFTER_H = 101
};



#pragma pack(pop)

}