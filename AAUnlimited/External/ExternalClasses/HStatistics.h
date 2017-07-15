#pragma once

#include <Windows.h>


namespace ExtClass {


#pragma pack(push, 1)

class HStatistics {
public:
	BYTE m_unknown[0x1DC];
	DWORD m_victoryCount;
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


	HStatistics() = delete;
	~HStatistics() = delete;
};

#pragma pack(pop)
}
