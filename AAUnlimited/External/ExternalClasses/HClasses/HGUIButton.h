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
	BYTE m_bVisible;		//if 0, this button is currently not visible.
	BYTE m_unkown2[2];
	float m_opacity;		//if 1, visible, if 0, not. However, these are constantly reset, so changing is of no use.
	BYTE m_bActive;			//if 0 (FALSE), this button is greyed out and can not be clicked
	BYTE m_unknown3[3];
	HGUIButton* m_categoryButton; //category this button is in, or NULL if its not an h pos button
	BYTE m_unknown4[0x18];
	BYTE m_unknown5;
	BYTE m_bInvalid;		//if 1, this button will not be rendered and can not be clicked.
	BYTE m_unknown6[2];
	void* m_somePointer;
	DWORD m_state;			//treated as bool it seems. categorys use this to save if they are opened. Cloth switches also utilize it.
	float m_renderX;		//where this button is rendered.
	float m_renderY;		//these coordinates define the top left corner
	DWORD m_posLeft;		//the actual position of the button,
	DWORD m_posTop;			//that is,
	DWORD m_posRight;		//where you have to click
	DWORD m_posBottom;		//to activate it
	BYTE m_mouseHover;

protected:
	static HGUIButton* PressedButton[5]; //used for Press() function

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