#include "CardData.h"

#include "../../AddressRule.h"

namespace ExtVars {
namespace AAEdit {


ExtClass::CharacterStruct* GetCurrentCharacter() {
	static const DWORD rule[] { 0x353254, 0x2C, 0 };
	return (ExtClass::CharacterStruct*)ApplyRule(rule);
}


}
}
