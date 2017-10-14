#include "StdAfx.h"
namespace ExtClass {
bool g_anim_data[25][14];
void CharacterStruct::ApplyAnimData() {
	if (!General::IsAAPlay) return;


	for (int i = 0; i < 25; i++) {
		bool *d = &g_anim_data[i][0];
		if (d[0] && AAPlay::g_characters[i].IsValid()) {
			auto ch = AAPlay::g_characters[i].m_char;
			ch->m_xxSkeleton->Animate(d[1], d[2], d[3], d[4], d[5], d[6]);
			ch->m_xxBody->Animate(d[7], d[8], d[9], d[10], d[11], d[12]);
			if (!d[13])
				ch->AnimateFace();
		}
	}
}

}