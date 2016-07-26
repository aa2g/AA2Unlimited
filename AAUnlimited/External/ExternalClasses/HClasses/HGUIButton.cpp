#include "HGUIButton.h"
#include "General/ModuleInfo.h"
#include "MemMods/MemRightsLock.h"
 
namespace ExtClass {

	void (HGUIButton::*HGUIButton::vmptrRefreshState)() = NULL;
	void* HGUIButton::VirtualTable = NULL;
	HGUIButton* HGUIButton::PressedButton; //used for Press() function

/*
 * Marks Button as Pressed. The next time this buttons state is polled, it will be set to pressed afterwards
 */
void HGUIButton::Press()
{
	HGUIButton::PressedButton = this;
}

void HGUIButton::HookedRefreshState()
{
	//call original method first
	(this->*vmptrRefreshState)();
	//if we were supposed to be pressed, do that now
	if(this == HGUIButton::PressedButton) {
		HGUIButton::PressedButton = NULL;
		m_bClicked = TRUE;
	}
}

void HGUIButton::InitializeHooks()
{
	//find virtual table
	VirtualTable = (void*)(General::GameBase + VirtualTableOffset);

	//set method pointers for later use
	//virt+4 is ecx-thiscall that refreshes state
	DWORD* refreshStateEntry = (DWORD*)((BYTE*)VirtualTable + 4);	//find entry in virtual table
	Memrights  memlock(refreshStateEntry,4);
	*(DWORD*)(&vmptrRefreshState) = *refreshStateEntry;				//set member function pointer
	auto newfunc = &HookedRefreshState;
	
	*refreshStateEntry = *(DWORD*)(&newfunc);						//replace entry with our method
}

}