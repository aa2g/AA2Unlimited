#include "HButtonMove.h"

#include "Files\Config.h"

namespace HButtonMove {

	
ExtClass::HGUIButton* loc_controlButton = NULL;
DWORD loc_controlX = 0;
const DWORD MAX_BUTTONS_PER_ROW = 20;
const DWORD X_SPACING = 10;

bool IsButtonValid(ExtClass::HGUIButton* btn) {
	if (btn == NULL) return false;
	if (btn->m_bInvalid) return false;
	if (btn->m_posTop == 0 || btn->m_posLeft == 0) return false;
	return true;
}

void MoveButtons(ExtClass::HInfo* info) {
	loc_controlButton = NULL;
	//go through every category
	for (int i = 0; i < 9; i++) {
		if (!IsButtonValid(info->m_btnCategories[i])) continue;
		auto& btns = info->m_hPosButtons[i];
		int nButtons = btns.GetButtonCount();
		DWORD x = 0, y = 0; //start x/y
		DWORD xs = 0, ys = 0; //x size / y size
		DWORD yspace = 0; //space between the y-start of each button
		int n;
		//go through every button of this category; i is button index, n is valid button index
		for (int i = 0, n = 0; i < nButtons; i++, n++) {
			ExtClass::HGUIButton* btn = btns.m_arrButtonList[i];
			if (!IsButtonValid(btn)) { n--;  continue; }
			if (x == 0) {
				//first button, save button dimensions
				x = btn->m_posLeft;
				y = btn->m_posTop;
				xs = btn->m_posRight - btn->m_posLeft;
				ys = btn->m_posBottom - btn->m_posTop;
			}
			else if (yspace == 0) {
				//second button, save space between buttons
				yspace = btn->m_posTop - (y + ys);
			}
			if (n >= MAX_BUTTONS_PER_ROW) {
				//reposition
				DWORD column = n / MAX_BUTTONS_PER_ROW;
				DWORD row = n % MAX_BUTTONS_PER_ROW;
				DWORD xPos = x - (xs + X_SPACING)*column;
				DWORD yPos = y + (ys + yspace)*row;
				btn->m_posTop = yPos;
				btn->m_posLeft = xPos;
				btn->m_posBottom = yPos + ys;
				btn->m_posRight = xPos + xs;
				btn->m_renderX = xPos;
				btn->m_renderY = yPos;
				if (loc_controlButton == NULL) {
					loc_controlButton = btn;
					loc_controlX = btn->m_renderX;
				}
			}
		}
	}
}

void PostTick(ExtClass::HInfo* info, bool cont) {
	//the buttons move themselves back at a position change, so we need
	//to keep moving them to the right position if that happened
	if (!g_Config.GetKeyValue(Config::USE_H_POS_BUTTON_MOVE).bVal) return;
	if (loc_controlButton == NULL || loc_controlButton->m_renderX != loc_controlX) {
		MoveButtons(info);
	}
	if (info->m_bEnd || !cont) {
		loc_controlButton = NULL;
	}
}



}