#include "WindowData.h"

#include "General\ModuleInfo.h"

namespace ExtVars {
namespace AAEdit {

	HWND* MainWnd = NULL;
	HWND* Dialogs = NULL;
	
	void InitializeExternals() {
		MainWnd = (HWND*)(General::GameBase + 0x381A6C);
		Dialogs = (HWND*)(General::GameBase + 0x353180);
	}

}
}