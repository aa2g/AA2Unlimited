#include "StdAfx.h"

namespace HButtonMove {

	
ExtClass::HGUIButton* loc_controlButton = NULL;
DWORD loc_controlX = 0;
const DWORD MAX_BUTTONS_PER_ROW = 19;
const bool bJMCP = false;	//whether to use hardcoded JMCP button positions

float screenScaleX = 0;
float screenScaleY = 0;

bool IsButtonValid(ExtClass::HGUIButton* btn) {
	if (btn == NULL) return false;
	if (btn->m_bInvalid) return false;
	if (btn->m_posTop == 0 || btn->m_posLeft == 0) return false;
	return true;
}

void NormalizeButton(ExtClass::HGUIButton* btn)
{
	float screenScaleX = g_Config.renderWidth / 1280.0;
	float screenScaleY = g_Config.renderHeight / 720.0;

	btn->m_renderX = floor(btn->m_renderX * screenScaleX);
	btn->m_renderY = floor(btn->m_renderY * screenScaleY);

	btn->m_posLeft = floor(btn->m_posLeft * screenScaleX);
	btn->m_posTop = floor(btn->m_posTop * screenScaleY);
	btn->m_posRight = floor(btn->m_posRight * screenScaleX);
	btn->m_posBottom = floor(btn->m_posBottom * screenScaleY);
}

ExtClass::HGUIButton* MoveButton(int categoryId, int buttonId, ExtClass::HGUIButton* btn)
{
	bool buttonMoved = false;
	LUA_EVENT("move_h_button", buttonMoved, categoryId, buttonId, btn);
	if (!buttonMoved) { // JMCP.lua script failed or is missing
		const DWORD X_SPACING = 5;
		const DWORD Y_SPACING = 5;
		const DWORD X_INTER_SPACING = 15;
		DWORD X_WIDE_SPACING = X_INTER_SPACING * 2 + X_SPACING;
		//first button
		int x = 1060;
		int y = 99;
		int xs = 136;
		int ys = 24;

		switch (categoryId) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		{
			//reposition
			DWORD column = buttonId / MAX_BUTTONS_PER_ROW;
			DWORD row = buttonId % MAX_BUTTONS_PER_ROW;
			DWORD xPos = x - (xs + X_SPACING) * column;
			DWORD yPos = y + (ys + Y_SPACING) * row;
			btn->m_renderX = xPos;
			btn->m_renderY = yPos;

			btn->m_posLeft = btn->m_renderX;
			btn->m_posTop = btn->m_renderY;
			btn->m_posRight = btn->m_renderX + xs;
			btn->m_posBottom = btn->m_renderY + ys;
			NormalizeButton(btn);
			if (column != 0 && loc_controlButton == NULL) {
				loc_controlButton = btn;
				loc_controlX = btn->m_renderX;
			}
			break;
		}
		case 7:
		case 8:
		{
			if (bJMCP)
			{
				switch (buttonId % 3) {
				case 0:
				{
					//reposition
					DWORD column = buttonId / (MAX_BUTTONS_PER_ROW * 3);
					DWORD row = (buttonId % (MAX_BUTTONS_PER_ROW * 3)) / 3;
					DWORD xPos = x - (xs + X_WIDE_SPACING) * column;
					DWORD yPos = y + (ys + Y_SPACING) * row;
					btn->m_posTop = yPos;
					btn->m_posBottom = yPos + ys;
					btn->m_posRight = xPos + xs;
					btn->m_posLeft = btn->m_posRight - X_INTER_SPACING;
					btn->m_renderX = xPos;
					btn->m_renderY = yPos;
					NormalizeButton(btn);
					if (column != 0 && loc_controlButton == NULL) {
						loc_controlButton = btn;
						loc_controlX = btn->m_renderX;
					}
					break;
				}
				case 1:
				{
					//reposition
					DWORD column = buttonId / (MAX_BUTTONS_PER_ROW * 3);
					DWORD row = (buttonId % (MAX_BUTTONS_PER_ROW * 3)) / 3;
					DWORD xPos = x - (xs + X_WIDE_SPACING) * column - 2 * X_INTER_SPACING;
					DWORD yPos = y + (ys + Y_SPACING) * row;
					btn->m_posTop = yPos;
					btn->m_posLeft = xPos;
					btn->m_posBottom = yPos + ys;
					btn->m_posRight = xPos + X_INTER_SPACING;
					btn->m_renderX = xPos;
					btn->m_renderY = yPos;
					NormalizeButton(btn);
					if (column != 0 && loc_controlButton == NULL) {
						loc_controlButton = btn;
						loc_controlX = btn->m_renderX;
					}
					break;
				}
				case 2:
				{
					//reposition
					DWORD column = buttonId / (MAX_BUTTONS_PER_ROW * 3);
					DWORD row = (buttonId % (MAX_BUTTONS_PER_ROW * 3)) / 3;
					DWORD xPos = x - (xs + X_WIDE_SPACING) * column - X_INTER_SPACING;
					DWORD yPos = y + (ys + Y_SPACING) * row;
					btn->m_posTop = yPos;
					btn->m_posLeft = xPos;
					btn->m_posBottom = yPos + ys;
					btn->m_posRight = xPos + xs;
					btn->m_renderX = xPos;
					btn->m_renderY = yPos;
					NormalizeButton(btn);
					if (column != 0 && loc_controlButton == NULL) {
						loc_controlButton = btn;
						loc_controlX = btn->m_renderX;
					}
					break;
				}
				}
			}
			else
			{
				//reposition
				DWORD column = buttonId / MAX_BUTTONS_PER_ROW;
				DWORD row = buttonId % MAX_BUTTONS_PER_ROW;
				DWORD xPos = x - (xs + X_SPACING) * column;
				DWORD yPos = y + (ys + Y_SPACING) * row;
				btn->m_posTop = yPos;
				btn->m_posBottom = yPos + ys;
				btn->m_posLeft = xPos;
				btn->m_posRight = xPos + xs;
				btn->m_renderX = xPos;
				btn->m_renderY = yPos;
				NormalizeButton(btn);
				if (column != 0 && loc_controlButton == NULL) {
					loc_controlButton = btn;
					loc_controlX = btn->m_renderX;
				}
			}
			break;
		}
		}
	}
	else // JMCP.lua or similar executed
	{
		DWORD column = buttonId / MAX_BUTTONS_PER_ROW;
		DWORD row = buttonId % MAX_BUTTONS_PER_ROW;
		// NormalizeButton(btn); // normalization is done on the script's side already
		if (column != 0 && loc_controlButton == NULL) {
			loc_controlButton = btn;
			loc_controlX = btn->m_renderX;
		}
	}

	return btn;
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
		for (int j = 0, n = 0; j < nButtons; j++, n++) {
			ExtClass::HGUIButton* btn = btns.m_arrButtonList[j];
			if (!IsButtonValid(btn)) { n--;  continue; }

			MoveButton(i, n, btn);
		}
	}
}

void PostTick(ExtClass::HInfo* info, bool cont) {
	//the buttons move themselves back at a position change, so we need
	//to keep moving them to the right position if that happened
	if (!g_Config.bEnableHPosButtonReorder) return;
	if (loc_controlButton == NULL || loc_controlButton->m_renderX != loc_controlX) {
		MoveButtons(info);
	}
	if (info->m_bEnd || !cont) {
		loc_controlButton = NULL;
	}
}



}
