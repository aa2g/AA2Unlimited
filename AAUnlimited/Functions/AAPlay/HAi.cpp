#include "HAi.h"

#include <vector>
#include <string>
#include <array>

#include "General\Util.h"
#include "Files\Config.h"
#include "HAis/ForceAi.h"

using namespace ExtClass;

namespace HAi {

/*
 * local variables used for the ai
 */
bool loc_initialized = false;
bool loc_isForcedH = false;

ForceAi loc_forceAi;

void Initialize(HInfo* info) {
	loc_forceAi.Initialize(info);
	loc_initialized = true;
}

void PreTick(HInfo* hinfo)
{
	
}

void PostTick(HInfo* hinfo, bool end)
{
	if (!g_Config.GetKeyValue(Config::USE_H_AI).bVal) return;
	if (!loc_isForcedH) return; //only do something if forced
	if (!loc_initialized) {
		//initialize
		Initialize(hinfo);
	}
	loc_forceAi.Tick(hinfo);
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