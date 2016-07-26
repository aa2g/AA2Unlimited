#pragma once

#include <Windows.h>

namespace ExtVars {
namespace AAEdit {

extern HWND* MainWnd; //pointer to handle of the left window (or 
extern HWND* Dialogs; //pointer to array of dialog handles (system/figure etc).
					  //note that [0] is the upper bar that selects the dialogs, [1] is system etc

void InitializeExternals();

}
}
