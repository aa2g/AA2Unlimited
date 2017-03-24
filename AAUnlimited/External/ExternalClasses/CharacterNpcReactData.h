#pragma once

#include <Windows.h>

#include "IllusionArray.h"

namespace ExtClass {


#pragma pack(push, 1)
/*
 * contains some data used for npc actions, most notably the ai files for reactions (06_0x)
 */
class CharacterNpcReactData {
	BYTE m_unknown[0x28];
	DWORD m_seenCount; //constantly counted up when seen during H, stopped at 0xEA60 (60k) to perform an action (end h scene)
	BYTE m_unknown2[0x20];
	//the following arrays store the floats of an ai file (jg2p00_00_00.pp. jg2_01_xx_xx.lst) the specified files on a column-basis.
	//that means that every array element represents a column, and every IllusionArray element represents the value in a row.
	//note that these arrays do not seem to have these strict column sizes in there definition but are instead linearily dumped
	//into the struct; however, it seems that the code makes assumption about this structure and by extension the amount of columns
	//in the ai files
	IllusionArray<float> m_ai06_01[12];
	IllusionArray<float> m_ai06_02[3];
	IllusionArray<float> m_ai06_03[6];
	IllusionArray<float> m_ai06_04[7];
	IllusionArray<float> m_ai06_05[4];
	IllusionArray<float> m_ai06_00[3];
	IllusionArray<float> m_ai06_06[4];
	IllusionArray<float> m_ai06_07[2];
	IllusionArray<float> m_ai06_08[4];

	CharacterNpcReactData() = delete;
	~CharacterNpcReactData() = delete;
};

static_assert(sizeof(CharacterNpcReactData) == 0x31C,"size missmatch");

#pragma pack(pop)
}
