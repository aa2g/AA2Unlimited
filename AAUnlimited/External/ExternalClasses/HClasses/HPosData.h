#pragma once

#include <Windows.h>

enum HPreferenceFlags {
	HPREF_KISSING = 1, HPREF_CHEST_CARRESSING = 2, HPREF_FEMALE_PRIMARY_CARESSING = 4,
	HPREF_MALE_PRIMARY_CARESSING = 8, HPREF_CUNNILINGUS = 0x10, HPREF_FELLATIO = 0x20,
	HPREF_DOGGY_STYLE = 0x40, HPREF_FEMDOM = 0x80, HPREF_ANAL_PLAY = 0x100, HPREF_NO_CONDOM = 0x200
};

enum HGenderAllowance {
	GENDERALLOW_HOMO_ONLY = -1, GENDERALLOW_HETERO_ONLY = 0, GENDERALLOW_BOTH = 1,
};

enum HPoseCategory {
	HCAT_FOREPLAY_FEMALE = 0, HCAT_FOREPLAY_MALE = 1, HCAT_FOREPLAY_MUTUAL = 2, HCAT_FRONTAL = 3, HCAT_BACK = 4,
	HCAT_CLIMAX_FOREPLAY_FEMALE = 5, HCAT_CLIMAX_FOREPLAY_MALE = 6, HCAT_CLIMAX_FRONTAL = 7, HCAT_CLIMAX_BACK = 8
};

#pragma pack(push, 1)
class HPosData {
public:
	BYTE m_unknown1[0x1C];
	DWORD m_type; //0 to 8, which category it belongs to (f|m|mutual|v|a||f-finish|m-f|v-f|a-f)
	DWORD m_climaxOptions; //1 = giver, 2 = reciever, 4 = together, 8 = inside, 0x10 = outside
	BYTE m_unknown2[0x4];
	WORD m_priority; //1 = male, 2 = female
	BYTE m_unknown3[0xE];
	DWORD m_preferenceFlags;
	DWORD m_creamPosition; //0 = none, 2 = outside & no button, 3 = inside & button
	BYTE m_unknown4[0x8];
	WORD m_yaoiAllowance; //-1 = yaoi only, 0 = both, 1 = normal only
	WORD m_yuriAllowance; //-1 = yuri only, 0 = both, 1 = normal only
	BYTE m_unknown5[0x14];
	wchar_t m_fileName[8]; //h_xx_x\0: seems to be unique to each scene. Could be used to identify semantics of this position
	BYTE m_unknown6[0x1C];

	HPosData() = delete;
	~HPosData() = delete;
};
static_assert(sizeof(HPosData) == 0x8C, "HInfo must be 0x8C bytes wide");
#pragma pack(pop)