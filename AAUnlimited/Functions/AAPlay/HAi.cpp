#include "HAi.h"

#include <vector>
#include <string>
#include <array>

#include "General\Util.h"
#include "Files\Config.h"
#include "Files\Logger.h"
#include "HAis\ForceAi.h"

using namespace ExtClass;

namespace HAi {

/*
 * local variables used for the ai
 */
bool loc_initialized = false;
bool loc_isForcedH = false;
bool loc_isTalkedTo = false;

ForceAi loc_forceAi;

void Initialize(HInfo* info) {
	loc_forceAi.Initialize(info);
	loc_initialized = true;
}

void PreTick(HInfo* hinfo)
{
	if (!g_Config.GetKeyValue(Config::USE_H_AI).bVal) return;
	if (!loc_isForcedH) return; //only do something if forced
	if (!loc_initialized) return;
	loc_forceAi.Tick(hinfo);

}

void PostTick(HInfo* hinfo, bool notEnd)
{
	if (!g_Config.GetKeyValue(Config::USE_H_AI).bVal) return;
	if (!loc_isForcedH) return; //only do something if forced
	if (!loc_initialized) {
		//initialize
		LOGPRIO(Logger::Priority::SPAM) << "initializing H-Ai\r\n";
		Initialize(hinfo);
	}
	if (!notEnd || hinfo->m_bEnd) {
		LOGPRIO(Logger::Priority::SPAM) << "ending H-Ai\r\n";
		loc_initialized = false;
		loc_isForcedH = false;
		loc_isTalkedTo = false;
	}
}

void ConversationTickPost(ExtClass::NpcPcInteractiveConversationStruct* param) {
	if (!g_Config.GetKeyValue(Config::USE_H_AI).bVal) return;

	ConversationSubStruct* convData = param->GetSubStruct();
	if (convData->m_conversationId == ConversationId::FORCE_H 	//npc wants to force
		||  (g_Config.GetKeyValue(Config::HAI_ON_NO_PROMPT).bVal
			&& convData->m_conversationId == ConversationId::NO_PROMPT_H)) 
	{
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

void ConversationTickPost(ExtClass::NpcPcNonInteractiveConversationStruct* param) {
	if (!g_Config.GetKeyValue(Config::USE_H_AI).bVal) return;
	ConversationSubStruct* convData = param->GetSubStruct();
	/*if (g_Config.GetKeyValue(Config::NO_PROMPT_IS_FORCE).bVal) {
		if (convData->m_conversationId == ConversationId::NO_PROMPT_H) {
			convData->m_conversationId = ConversationId::FORCE_H;
		}
	}*/
	if (g_Config.GetKeyValue(Config::HAI_ON_NO_PROMPT).bVal 
	&& convData->m_conversationId == ConversationId::NO_PROMPT_H) {
		loc_isForcedH = true;
	}
	else {
		loc_isForcedH = false;
	}
}

void ConversationPcResponse(ExtClass::BaseConversationStruct* param) {
	if (loc_isTalkedTo) {
		ConversationSubStruct* convData = param->GetSubStruct();
		convData->m_playerAnswer = 0; //positive player answer
		convData->m_response = 1;
		loc_isForcedH = true;
	}
}

};