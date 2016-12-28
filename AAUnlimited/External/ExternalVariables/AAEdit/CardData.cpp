#include "CardData.h"

#include "../../AddressRule.h"

namespace ExtVars {
namespace AAEdit {


ExtClass::CharacterStruct* GetCurrentCharacter() {
	//2C is female character, 30 is male character
	static const DWORD femaleRule[] { 0x353254, 0x2C, 0 };
	static const DWORD maleRule[]{ 0x353254, 0x30, 0 };
	//try to get female struct
	ExtClass::CharacterStruct* retVal = (ExtClass::CharacterStruct*)ApplyRule(femaleRule);
	//if not there, try to get male struct instead
	if(retVal == NULL) retVal = (ExtClass::CharacterStruct*)ApplyRule(maleRule);

	return retVal;
}


}
}
