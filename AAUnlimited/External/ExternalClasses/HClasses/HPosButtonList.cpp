#include "HPosButtonList.h"

namespace ExtClass {

int HPosButtonList::GetButtonCount()
{
	DWORD firstEntryAddress = (DWORD)m_arrButtonList;
	DWORD lastEntryAddress = (DWORD)m_pLastButton;
	return (lastEntryAddress - firstEntryAddress)/4;
}

HGUIButton* HPosButtonList::GetRandomActiveButton()
{
	HGUIButton* retVal = NULL;
	HGUIButton** it = m_arrButtonList;
	int nActive = 0;
	while(it != m_pLastButton) {
		//doing an approximate random pick here in only one iteration (as we don't know how any active buttons there are)
		if((*it)->m_bActive) {
			nActive++;
			double pickPropability = nActive / 1.0;
			double randPick = (rand() / (double)RAND_MAX); //not the best resolution, but meh
			if(randPick < pickPropability) {
				retVal = *it;
			}
		}
		it++;
	}
	return retVal;
}

HGUIButton* HPosButtonList::GetRandomButton() {
	int nButtons = GetButtonCount();
	if (nButtons == 0) return NULL;
	int randButton = rand() % nButtons;
	return m_arrButtonList[randButton];
}

void HPosButtonList::InitializeHooks()
{
	//virtTable+4 = void ecx-thiscall RefreshCheckState();
	//virtTable+0x20 = bool ecx-thiscall GetChecked() { return m_checked; }
}

}