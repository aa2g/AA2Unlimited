#pragma once

#include "External\ExternalClasses\CharacterStruct.h"
#include "External\ExternalClasses\ConversationStruct.h"
#include "External\ExternalClasses\TimeData.h"

namespace ExtVars {
namespace AAPlay {

ExtClass::CharacterStruct** PlayerCharacterPtr(); //pointer to the character struct of the currently controlled player.
ExtClass::PcConversationStruct* PlayerConversationPtr(); //pointer to the global struct that handles conversations involving the pc


ExtClass::CharacterStruct** ClassMembersArray();
ExtClass::CharacterStruct** ClassMembersArrayEnd(); //first invalid pointer in the array

ExtClass::TimeData* GameTimeData();

}
}