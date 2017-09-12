#include "StdAfx.h"
namespace ExtClass {
DWORD g_anim_data[25][10];
void CharacterStruct::ApplyAnimData() {
	if (!General::IsAAPlay) return;

	for (int i = 0; i < 25; i++) {
		DWORD *d = &g_anim_data[i][0];
		if (d[0] && AAPlay::g_characters[i].IsValid()) {
			auto ch = AAPlay::g_characters[i].m_char;
			ch->Animate2(d[1], d[2], d[3], d[4], d[5], d[6]);
			ch->Animate1(d[7], d[8], d[9]);
		}
	}
}

}