#include "StdAfx.h"
namespace ExtClass {
// b entry
// b anim skeleton
// b b anim body 1
// anim1(b,b) // 1 1
// anim2(b,b,b,b) // 1 1 1 1
BYTE g_anim_data[25][6];
D3DMATRIX mtx[25];
void CharacterStruct::ApplyAnimData() {
	if (!General::IsAAPlay) return;


	for (int i = 0; i < 25; i++) {
		BYTE *e = &g_anim_data[i][0];
		if (e[0] && AAPlay::g_characters[i].IsValid()) {
			auto ch = AAPlay::g_characters[i].m_char;				
			if (e[1]) ch->m_xxSkeleton->Animate(0, 0, 0, 0, 0, 0);
			ch->Animate2(0, NULL, 1, 1, e[2], e[3]);
			ch->Animate1(0, e[4], e[5]);
		}
	}
}

}