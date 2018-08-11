#include "StdAfx.h"

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
	if (!g_Config.bUseHAi) return;
	if (!loc_isForcedH && !Shared::GameState::getH_AI()) return; //only do something if forced
	if (!loc_initialized) return;
	loc_forceAi.Tick(hinfo);

}

void PostTick(HInfo* hinfo, bool notEnd)
{
	if (!g_Config.bUseHAi) return;
	if (!loc_isForcedH && !Shared::GameState::getH_AI()) return; //only do something if forced
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
		Shared::GameState::setH_AI(false);
		Shared::GameState::setLockedInH(true);
	}
}

void ConversationTickPost(ExtClass::NpcPcInteractiveConversationStruct* param) { //sets whether PC was talked to or not
	if (!g_Config.bUseHAi) return;

	ConversationSubStruct* convData = param->GetSubStruct();
	if (convData->m_conversationId == ConversationId::FORCE_H 	//npc wants to force
		||  (g_Config.bHAiOnNoPromptH
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

void ConversationTickPost(ExtClass::NpcPcNonInteractiveConversationStruct* param) { //sets whether PC was talked to or not
	if (!g_Config.bUseHAi) return;
	ConversationSubStruct* convData = param->GetSubStruct();
	/*if (g_Config.GetKeyValue(Config::NO_PROMPT_IS_FORCE).bVal) {
		if (convData->m_conversationId == ConversationId::NO_PROMPT_H) {
			convData->m_conversationId = ConversationId::FORCE_H;
		}
	}*/
	if (g_Config.bHAiOnNoPromptH 
	&& convData->m_conversationId == ConversationId::NO_PROMPT_H) {
		loc_isForcedH = true;
	}
	else {
		loc_isForcedH = false;
	}
}

void ConversationPcResponse(ExtClass::BaseConversationStruct* param) { //sets PC response to yes and sets the bool that starts h-ai
	if (loc_isTalkedTo) {
		auto pc = Shared::GameState::getPlayerCharacter();
		int card = pc->m_seat;
		CharInstData* cardInst = &AAPlay::g_characters[card];
		if (cardInst->m_char->m_charData->m_traitBools[Trait::TRAIT_EXPLOITABLE]) {
			ConversationSubStruct* convData = param->GetSubStruct();
			convData->m_playerAnswer = 0; //positive player answer
			convData->m_response = 1;
			loc_isForcedH = true;
		}
		else {
			loc_isTalkedTo = false;
		}
	}
}

};
