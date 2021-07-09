#include "StdAfx.h"

namespace AAEdit {
	
CharInstData g_currChar;
HINSTANCE AAFACEDLL;
void SetAAFACEDLL(HINSTANCE ptr) {
	AAFACEDLL = ptr;
}
}
