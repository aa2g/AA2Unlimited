#include "GameGlobals.h"

#include "External\ExternalClasses\CharacterStruct.h"
#include "..\..\AddressRule.h"
#include "General\ModuleInfo.h"

namespace ExtVars {
namespace AAPlay {

ExtClass::CharacterStruct** PlayerCharacterPtr() {
	//"AA2Play v12 FP v1.4.0a.exe"+376164
	//+88
	static const DWORD offsets[] { 0x376164, 0x88 };
	return (ExtClass::CharacterStruct**)
		ApplyRule(offsets);
}

}
}