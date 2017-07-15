#pragma once

#include <Windows.h>

#include "IllusionArray.h"
#include "CharacterActivity.h"

namespace ExtClass {


#pragma pack(push, 1)
/*
* contains a lot of ai data
*/
class CharacterNpcAiData {
	BYTE m_unknown[0x18];
	CharacterActivity* m_activity;
	struct {
		BYTE unknown[0x1C];
		IllusionArray<float> ai04_00[4];
		IllusionArray<float> ai04_01[3];
		IllusionArray<float> ai04_02[3];
	}* m_passiveAiData; //this is a pointer somewhere globally, yet every character has its own copy
	IllusionArray<float> ai01_03[2];
	IllusionArray<float> ai05_00[13];
	IllusionArray<int> ai05_01[13];	//these are adjusted in some way using the 05_08 data
	IllusionArray<float> ai05_02[96];
	IllusionArray<float> ai05_03[109];
	struct unknownstruct {
		BYTE unknown[0x16];
	};
	IllusionArray<unknownstruct> ai05_08;

	CharacterNpcAiData() = delete;
	~CharacterNpcAiData() = delete;
};

static_assert(sizeof(CharacterNpcAiData) == 0xEC0,"size missmatch");

#pragma pack(pop)
}
