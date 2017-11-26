#pragma once

#include <Windows.h>


namespace ExtClass {

class NpcStatus;

#pragma pack(push, 1)

class CharacterStatus {
public:
	BYTE m_unknown0[0x28];
	NpcStatus* m_npcStatus; //28
	BYTE m_unknown1[0x1B0]; //from TAA2: pointers to meet, date and lewd promises
	DWORD m_victoryCount; //1DC
	DWORD m_classesSkipped;
	DWORD m_winningOverSomeoneCount;
	DWORD m_partnerCount;
	DWORD m_rejectCount;
	DWORD m_academicGrade; //5 = A, 0 = F
	DWORD m_sportGrade;
	DWORD m_clubGrade;
	BYTE m_unknown2[0x10];
	char m_firstHPartner[32];
	char m_firstAnalPartner[32];
	char m_latestHPartner[32];
	union {
		struct {
			DWORD m_unknownArray1[25]; //i do not know what these represent, but they have one element per character
			DWORD m_unknownArray2[25]; //(addressed by seat)
			DWORD m_totalH[25];
			DWORD m_vaginalH[25];
			DWORD m_analH[25];
			DWORD m_condomsUsed[25];
			DWORD m_climaxCount[25];
			DWORD m_simultaneousClimax[25];
			DWORD m_totalCum[25]; //displayed as (value*3)cc (so it stores displayed cc value /3)
			DWORD m_cumInVagina[25];
			DWORD m_cumInAnal[25];
			DWORD m_cumSwallowed[25];
		};
		DWORD m_statArrays[17][25];
		DWORD m_stats[17*25];
	};

#define LUA_CLASS ExtClass::CharacterStatus
	static inline void bindLua() {
		LUA_NAME;
		LUA_BIND(m_victoryCount)
		LUA_BIND(m_classesSkipped)
		LUA_BIND(m_winningOverSomeoneCount)
		LUA_BIND(m_partnerCount)
		LUA_BIND(m_rejectCount)
		LUA_BIND(m_academicGrade)
		LUA_BIND(m_sportGrade)
		LUA_BIND(m_clubGrade)
		LUA_BIND(m_npcStatus)
		// TODO check these are actually strings
		LUA_BINDSTR(m_firstHPartner)
		LUA_BINDSTR(m_firstAnalPartner)
		LUA_BINDSTR(m_latestHPartner)

		LUA_BINDARR(m_totalH)
		LUA_BINDARR(m_vaginalH)
		LUA_BINDARR(m_analH)
		LUA_BINDARR(m_condomsUsed)
		LUA_BINDARR(m_climaxCount)
		LUA_BINDARR(m_simultaneousClimax)
		LUA_BINDARR(m_totalCum)
		LUA_BINDARR(m_cumInVagina)
		LUA_BINDARR(m_cumInAnal)
		LUA_BINDARR(m_stats)
	}
#undef LUA_CLASS

	CharacterStatus() = delete;
	~CharacterStatus() = delete;
};

#pragma pack(pop)
}
