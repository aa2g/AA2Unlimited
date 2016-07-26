#pragma once
#include "HGUIButton.h"

namespace ExtClass {

#pragma pack(push, 1)
/*
 * Represents a single list of h pos buttons under a certain category
 */
class HPosButtonList
{
public:
	HGUIButton** m_arrButtonList; //array of HGUIButton pointers
	HGUIButton** m_pLastButton; //pointer pointing inside the array above, past the last valid member
	void* m_unknownPointer; //usually same as lastButtonList
	DWORD m_unknown;
public:
	HPosButtonList() = delete;
	~HPosButtonList() = delete;

	int GetButtonCount();
	HGUIButton* GetRandomActiveButton();
	HGUIButton* GetRandomButton();

	static void InitializeHooks();
};
static_assert(sizeof(HPosButtonList) == 0x10,"HPosButtonList must be 0x10 bytes wide");
#pragma pack(pop)

}