#pragma once
#include <Windows.h>
#include "HParticipant.h"
#include "HPosButtonList.h"
#include "../IllusionList.h"

namespace ExtClass {

#pragma pack(push, 1)
/*
* Contains information about ongoing h
*/
class HInfo
{
public:
	BYTE m_unknown1[0x68];
	DWORD m_nPosChanges;
	BYTE m_unknown2[8];
	float m_speed; //maximum value accessable through buttons ingame is 3
	BYTE m_unknown3[0x30];
	struct {
		DWORD unknown1;
		DWORD unknown2;
		DWORD unknown3;
		IllusionList* buttonList; //note that the first element is NOT a button
		DWORD unknown;
		HGUIButton* currentlyHovered;
	} *m_ptrAllButtons;		//a list containing EVERY button, including the category buttons below.
	BYTE m_unknown4[0x31];
	BYTE m_bEnd;
	WORD m_unknown5;
	HParticipant* m_activeParticipant;
	HParticipant* m_passiveParticipant;
	BYTE m_unknown6[0x50C];
	DWORD m_currPosition;
	BYTE m_unknown7[0x18];
	HGUIButton* m_exitButton;			//button to exit the scene
	HGUIButton* m_swapButton;			//the two circle arrows that swap positions between the two participants
	HGUIButton* m_expandClothesButton;	//hides or unhides the cloth select buttons
	HGUIButton* m_category1;	//category buttons that hide the h-actions until clicked on. 1: Hand
	HGUIButton* m_category2;	//2: tounge
	HGUIButton* m_category3;	//3: m|f
	HGUIButton* m_category4;	//4: m in f
	HGUIButton* m_category5;	//5: flower
	HGUIButton* m_category6;	//6: finish hand
	HGUIButton* m_category7;	//7: finish m|f/tounge
	HGUIButton* m_category8;	//8: finish m in f
	HGUIButton* m_category9;	//9: finish flower
	BYTE m_unknown8[0x4];
	HPosButtonList m_hPosButtons[9];
	BYTE m_unknown9[0x8C];
	HGUIButton* m_maleButton; //the button that switches male between blue / off / ugly
	HGUIButton* m_skirtButton;	//toggles skirt state
	HGUIButton* m_shoeButton;	//toggles shoe visibility on and off
	HGUIButton* m_underwearButton; //toggles underwear state
	HGUIButton* m_outfitButton; //toggles outfit state
public:
	HInfo() = delete;
	~HInfo() = delete;
};

static_assert(sizeof(HInfo) == 0x774,"HInfo must be 0x6D4 bytes wide");
#pragma pack(pop)

}