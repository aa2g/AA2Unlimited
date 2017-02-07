#include "RefreshTable.h"

#include "WindowData.h"
#include "General\ModuleInfo.h"

namespace ExtVars {
namespace AAEdit {



namespace {
//offsets of the classes
	enum RedrawClassOffset {
		BAR = 0,
		SYSTEM = 0, FIGURE = 0xD8, CHEST = 0, BODYCOLOR = 0x240, FACE = 0, EYES = 0x114, EYECOLOR = 0,
		EYEBROWS = 0, FACEDETAILS = 0x19C, HAIR = 0x5C4, HAIRCOLOR = 0, CHARACTER = 0, PERSONALITY = 0, TRAITS = 0
	};
	RedrawClassOffset loc_offsetTable[16] = { 
		BAR,
		SYSTEM, FIGURE, CHEST, BODYCOLOR, FACE, EYES, EYECOLOR, 
		EYEBROWS, FACEDETAILS, HAIR, HAIRCOLOR, CHARACTER, PERSONALITY, TRAITS
	};
}

void RedrawBodyPart(Category cat, RedrawId redraw) {
	int windowIndex = cat;
	RedrawClassOffset offset = loc_offsetTable[windowIndex];
	if (offset == 0) return;
	
	HWND dialog = Dialogs[cat];
	WCHAR* propName = (WCHAR*)(General::GameBase + 0x3100A4); //name of the prop that is used to save the class name, "__WND_BASE_CLASS__"
	BYTE* internclass = (BYTE*)GetPropW(dialog,propName);
	if (internclass == NULL) return;
	//set both flags
	*(internclass+offset) = 1;
	BYTE* redrawTable = (BYTE*)(General::GameBase + 0x353160);
	*(redrawTable + redraw) = 1;
}



}
}
