#include "HAi.h"

#include "General\Util.h"
#include "Files\Config.h"

using namespace ExtClass;

namespace HAi {

/*
 * local variables used for the ai
 */
bool loc_initialized = false;
bool loc_isForcedH = false;

DWORD loc_tickCount = 0;
DWORD loc_aiState = 0;

const unsigned int N_TIMERS = 10;
General::PassiveTimer loc_timers[N_TIMERS];

double loc_speedStart;
double loc_speedTarget;
double loc_speedChangeDuration;
double loc_continueChance = 1;
double loc_continueChanceDecay = 0.75;

void PressRandomCategoryButton(HInfo* info, int category, bool onlyActive) {
	HGUIButton* btn = NULL;
	if (onlyActive) {
		btn = info->m_hPosButtons[category].GetRandomActiveButton();
	}
	else {
		btn = info->m_hPosButtons[category].GetRandomButton();
	}
	if (btn != NULL) {
		btn->Press();
	}
}

void DisableAllButtons(HInfo* info) {
	info->m_category1->m_bActive = FALSE;
	info->m_category2->m_bActive = FALSE;
	info->m_category3->m_bActive = FALSE;
	info->m_category4->m_bActive = FALSE;
	info->m_category5->m_bActive = FALSE;
	info->m_category6->m_bActive = FALSE;
	info->m_category7->m_bActive = FALSE;
	info->m_category8->m_bActive = FALSE;
	info->m_category9->m_bActive = FALSE;

	info->m_swapButton->m_bActive = FALSE;
	info->m_expandClothesButton->m_bActive = FALSE;

	info->m_shoeButton->m_bActive = FALSE;
	info->m_underwearButton->m_bActive = FALSE;
	info->m_outfitButton->m_bActive = FALSE;
	info->m_maleButton->m_bActive = FALSE;
	info->m_skirtButton->m_bActive = FALSE;

	info->m_exitButton->m_bActive = FALSE; //NO ESCAPE

	//note that the outside finish button is dynamically created and added to the end of this
	//list only when an appropriate position is taken.
	//there is nothing we want to enable though, so its never wrong to just disable
	//whatever the last button is if the button does not currently exist
	HGUIButton* outside = (HGUIButton*)info->m_ptrAllButtons->buttonList->prev->data;
	if (outside != NULL) outside->m_bActive = FALSE;
}

void Initialize(HInfo* info) {
	//we disable all buttons, so the player cant click anything anymore
	DisableAllButtons(info);

	loc_initialized = true;
	loc_tickCount = 0;
	loc_aiState = 0;
}

void PreTick(HInfo* hinfo)
{
	if (!g_Config.GetKeyValue(Config::USE_H_AI).bVal) return;
	if (!loc_isForcedH) return; //only do something if forced
	if(!loc_initialized) {
		//initialize
		Initialize(hinfo);
	}
	DisableAllButtons(hinfo);
	loc_tickCount++;

	/* this ai will follow a really bad choreography for now. */
	switch (loc_aiState) {
	case 0:
		loc_timers[0].Start();
		loc_timers[1].Start();
		loc_aiState = 1;
		break;
	case 1:
		if (loc_timers[1].GetTime() > 5) {
			loc_timers[1].Start();
			PressRandomCategoryButton(hinfo, 0, false);
			hinfo->m_speed = 0.5;
			loc_aiState = 2;
		}
		break;
	case 2:
		if (loc_timers[1].GetTime() > 10) {
			loc_timers[1].Start();
			loc_speedStart = hinfo->m_speed;
			loc_speedTarget = 1.5;
			loc_speedChangeDuration = 10;
			loc_aiState = 3;
		}
		break;
	case 3: {
		double percent = loc_timers[1].GetTime() / loc_speedChangeDuration;
		hinfo->m_speed = (float)(loc_speedStart + percent * (loc_speedTarget - loc_speedStart));
		if (percent >= 1) {
			hinfo->m_speed = (float)loc_speedTarget;
			if (loc_continueChance >= (rand() / (double)RAND_MAX)) {
				loc_continueChance *= loc_continueChanceDecay;
				loc_timers[1].Start();
				loc_speedStart = loc_speedTarget;
				loc_speedTarget = 2 + (rand() / (double)RAND_MAX);
				loc_speedChangeDuration = 10;
				PressRandomCategoryButton(hinfo, 0, false);
				loc_aiState = 3;
			}
			else {
				loc_timers[1].Start();
				PressRandomCategoryButton(hinfo, 0, false);
				hinfo->m_speed = 4;
				loc_aiState = 4;
			}
		}
		break; }
	case 4:
		if (loc_timers[1].GetTime() > 6) {
			loc_timers[1].Start();
			PressRandomCategoryButton(hinfo, 5, false);
			hinfo->m_speed = 2;
			loc_aiState = 5;
		}
		break;
	case 5:
		if (loc_timers[1].GetTime() > 25) {
			hinfo->m_exitButton->Press();
		}
		break;
	default:
		hinfo->m_exitButton->Press();
		break;
	}
}

void PostTick(HInfo* hinfo, bool end)
{

}

namespace {
	bool loc_isTalkedTo = false;
}

void ConversationTickPre(ConversationStruct* param)
{
	if (!g_Config.GetKeyValue(Config::USE_H_AI).bVal) return;

	ConversationSubStruct* convData = param->getSubStruct();

}

void ConversationTickPost(ExtClass::ConversationStruct* param) {
	if (!g_Config.GetKeyValue(Config::USE_H_AI).bVal) return;
	loc_initialized = false;
	ConversationSubStruct* convData = param->getSubStruct();
	if (convData->m_conversationId == ConversationId::FORCE_H) {	//npc wants to force
		loc_isTalkedTo = true;
		if (convData->m_npcTalkState == 1) {					//we just answered
			if (convData->m_response == 1) {					//and our answer was positive
				loc_isForcedH = true;
				return;
			}
		}
	}
	else {
		loc_isTalkedTo = false;
	}
}

void ConversationPcResponse(ExtClass::ConversationStruct* param) {
	if (loc_isTalkedTo) {
		ConversationSubStruct* convData = param->getSubStruct();
		convData->m_playerAnswer = 0; //positive player answer
		convData->m_response = 1;
		loc_isForcedH = true;
	}
}

};