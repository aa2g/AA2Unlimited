#pragma once
#include <Windows.h>

namespace ExtClass {

#pragma pack(push, 1)
/*
 * Represents the internal data structure that represents a GUI button on the games overlay
 * (e.g change h position, their group buttons, yes/no, everyone lets do x etc)
 */
class HGUIButton
{
private:
	static const DWORD VirtualTableOffset = 0x327660;

public:
	void* m_virtualTable;
	void* m_someGlobalPointer;
	DWORD m_unknown1;
	BYTE m_bClicked;
	BYTE m_unkown2[3];
	float m_someFloat;		//if 1, visible, if 0, not. However, these are constantly reset, so changing is of no use.
	BYTE m_bActive;			//if 0 (FALSE), this button is greyed out and can not be clicked
	BYTE m_unknown3[3];
	BYTE m_unknown4[0x20];
	void* m_somePointer;
	DWORD m_state;			//treated as bool it seems. categorys use this to save if they are opened. Cloth switches also utilize it.
	BYTE m_unknown5[8];
	DWORD m_posLeft;
	DWORD m_posTop;
	DWORD m_posRight;
	DWORD m_posBottom;
	BYTE m_mouseHover;

	static HGUIButton* PressedButton; //used for Press() function

	//virtual table
	static void* VirtualTable;
	//hooked methods
	static void (HGUIButton::*vmptrRefreshState)();
		static_assert(sizeof(vmptrRefreshState) == 4,"Member function pointer became too big"); //intelisense is just bugged
	void HookedRefreshState();
public:
	HGUIButton() = delete;
	~HGUIButton() = delete;

	void Press();


	static void InitializeHooks();
};
static_assert(sizeof(HGUIButton) == 0x59,"HGUIButton must be 0x59 bytes wide");
#pragma pack(pop)



}