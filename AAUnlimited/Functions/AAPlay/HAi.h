#pragma once

#include "External/ExternalClasses.h"
#include "Files/Config.h"

/*
 * Controls H if forced by npc on pc.
 */

namespace HAi {

/*
 * Is Called periodically while in h mode. 
 */
void PreTick(ExtClass::HInfo* hinfo);
/*
 * Is Called after each tick in h mode. end contains the return value
 * of illusions original tick function and denotes one possible end.
 */
void PostTick(ExtClass::HInfo* hinfo, bool end);

/*
 * Called periodically in an npc->pc conversation. used to determined
 * if pc is getting forced.
 */
void ConversationTickPost(ExtClass::NpcPcInteractiveConversationStruct* param);
void ConversationPcResponse(ExtClass::BaseConversationStruct* param);
/*
 * Called periodically for every conversation. used to determine circumstances
 */
void ConversationTickPost(ExtClass::NpcPcNonInteractiveConversationStruct* param);

};