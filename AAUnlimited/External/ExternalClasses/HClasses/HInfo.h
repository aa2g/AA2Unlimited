#pragma once
#include <Windows.h>
#include "HParticipant.h"
#include "HPosButtonList.h"
#include "HPosData.h"
#include "HCamera.h"
#include "../IllusionList.h"
#include "../../AddressRule.h"

namespace ExtClass {

#pragma pack(push, 1)
/*
* Contains information about ongoing h
*/
class HInfo
{
public:
	BYTE m_unknown1[0x24];
	DWORD m_somePtr; //points to somthing that includes something that includes the camera
	BYTE m_unknown___[0x14];
	void* m_positionInfo; //some position info it seems. *(*m_positionInfo + 4) points to an array with 0x8C per struct
						  //describing h positions
	BYTE m_unknown__[0x28];
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
	HGUIButton* m_btnExit;			//button to exit the scene
	HGUIButton* m_btnSwap;			//the two circle arrows that swap positions between the two participants
	HGUIButton* m_btnExpandClothes;	//hides or unhides the cloth select buttons
	union {
		struct {
			HGUIButton* m_btnCategory1;	//category buttons that hide the h-actions until clicked on. 1: Hand
			HGUIButton* m_btnCategory2;	//2: tounge
			HGUIButton* m_btnCategory3;	//3: m|f
			HGUIButton* m_btnCategory4;	//4: m in f
			HGUIButton* m_btnCategory5;	//5: flower
			HGUIButton* m_btnCategory6;	//6: finish hand
			HGUIButton* m_btnCategory7;	//7: finish m|f/tounge
			HGUIButton* m_btnCategory8;	//8: finish m in f
			HGUIButton* m_btnCategory9;	//9: finish flower
		};
		HGUIButton* m_btnCategories[9];
	};
	BYTE m_unknown8[0x4];
	HPosButtonList m_hPosButtons[9];
	struct {
		DWORD* m_arrPositions; //positions belonging to the buttons in the corresponding button arrays
		DWORD* m_arrPositionsEnd; //first invalid element behind arrPositoins
		DWORD m_unknown1;	//usually same as arrPositionsEnd, sometimes pointing past it, with 0s inbetween
		DWORD m_padding;	//pretty much always 0
	} m_hPosButtonPositions[8];
	struct {
		DWORD* m_arrPositions; //positions belonging to the buttons in the corresponding button arrays
		DWORD* m_arrPositionsEnd; //first invalid element behind arrPositoins
		DWORD m_unknown1;	//usually same as arrPositionsEnd, sometimes pointing past it, with 0s inbetween
	} m_hPosButtonPositions_; //the last one doesnt have padding.
	HGUIButton* m_btnMale; //the button that switches male between blue / off / ugly
	HGUIButton* m_btnSkirt;	//toggles skirt state
	HGUIButton* m_btnShoe;	//toggles shoe visibility on and off
	HGUIButton* m_btnUnderwear; //toggles underwear state
	HGUIButton* m_btnOutfit; //toggles outfit state
public:
	inline DWORD GetHPosition(int category, int buttonNumber) {
		if (category < 0 || category > 8) return -1;
		auto* list = &(m_hPosButtonPositions[category]);
		DWORD* ptrHPos = list->m_arrPositions + buttonNumber;
		if (ptrHPos >= list->m_arrPositionsEnd) return 0;
		return *ptrHPos;
	}

	inline HPosData* GetHPosData(DWORD position) {
		HPosData* arr = *(HPosData**)((BYTE*)(m_positionInfo)+4);
		return arr + position;
	}

	inline HCamera* GetCamera() { 
		const static DWORD rule[] { 0x24, 0x4, 0x198, 0 };
		return (HCamera*)ExtVars::ApplyRule(this,rule); 
	}

	HInfo() = delete;
	~HInfo() = delete;
};

static_assert(sizeof(HInfo) == 0x774,"HInfo must be 0x6D4 bytes wide");
#pragma pack(pop)

}