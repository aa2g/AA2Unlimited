#pragma once

#include <Windows.h>

#include "IllusionArray.h"

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
	BYTE m_unknown[4];
	IllusionArray<DWORD> m_actionBacklog;		//between 0 and 4, 0 = love, 1 = like, 2 = dislike, 3 = hate
	BYTE m_unknown2[4];
	int m_loveCount;
	int m_likeCount;
	int m_dislikeCount;
	int m_hateCount;
	int m_love;			//this are between (0,1,2) as (max,med,low)
	int m_like;
	int m_dislike;
	int m_hate;
	BYTE m_unknown3[10];
	int m_poin;			//i have no idea what this is. aa2trainer calls it poin.

public:
	CharacterRelation() = delete;
	~CharacterRelation() = delete;

};


#pragma pack(pop)

}
