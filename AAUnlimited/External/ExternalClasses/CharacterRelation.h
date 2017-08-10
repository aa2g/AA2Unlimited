#pragma once

#include <Windows.h>

#include "IllusionArray.h"
#include "Script/ScriptLua.h"

namespace ExtClass {

#pragma pack(push, 1)
/*
 * Describes the relationship of a given (unspecified) character towards a specific character
 */
class CharacterRelation {
public:
	int m_targetSeat;			//seat of the character who this data describes
	int m_lovePoints;
	int m_likePoints;
	int m_dislikePoints;
	int m_hatePoints;
	IllusionArray<DWORD> m_actionBacklog;		//between 0 and 4, 0 = love, 1 = like, 2 = dislike, 3 = hate
	int m_loveCount;
	int m_likeCount;
	int m_dislikeCount;
	int m_hateCount;
	int m_love;			//this are between (0,1,2) as (max,med,low)
	int m_like;
	int m_dislike;
	int m_hate;
	BYTE m_unknown3[0x10];
	int m_poin;			//i have no idea what this is. aa2trainer calls it poin.

public:
	CharacterRelation() = delete;
	~CharacterRelation() = delete;
	static inline void bindLua() {
#define LUA_CLASS ExtClass::CharacterRelation
			LUA_BIND(m_poin)
			LUA_BIND(m_hate)
			LUA_BIND(m_dislike)
			LUA_BIND(m_like)
			LUA_BIND(m_love)
			LUA_BIND(m_hateCount)
			LUA_BIND(m_dislikeCount)
			LUA_BIND(m_likeCount)
			LUA_BIND(m_loveCount)
			LUA_BIND(m_hatePoints)
			LUA_BIND(m_dislikePoints)
			LUA_BIND(m_likePoints)
			LUA_BIND(m_lovePoints)
			LUA_BIND(m_targetSeat)
			LUA_BINDARRE(m_actionBacklog,,_self->m_actionBacklog.GetSize())
	}
#undef LUA_CLASS
};

static_assert(sizeof(CharacterRelation) == 0x58,"Character Relation should be 0x58 bytes in size");


#pragma pack(pop)

}
