#pragma once

#include <Windows.h>

namespace ExtClass {


#pragma pack(push, 1)

/*
 * I know basically nothing about this struct and what it contains. Feel free to do research
 */
class LoverData {
public:
	int m_targetSeat;
	BYTE m_unknown[0x58];

};

static_assert(sizeof(LoverData) == 0x5C,"LoverData size missmatch");

#pragma pack(pop)

}