#pragma once

#include "External\ExternalClasses\CharacterStruct.h"

namespace ExtVars {
namespace AAPlay {

ExtClass::CharacterStruct** PlayerCharacterPtr();

ExtClass::CharacterStruct** ClassMembersArray();
ExtClass::CharacterStruct** ClassMembersArrayEnd(); //first invalid pointer in the array

}
}