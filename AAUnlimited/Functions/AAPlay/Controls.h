#pragma once


namespace Controls {

	extern void keyDown(WORD scan_code, bool extended = false);
	extern void keyUp(WORD scan_code, bool extended = false);
	extern void keyPress(WORD scan_code, bool extended = false, bool needDelay = false);
	extern void keysRelease();

	extern void EnterMenuF1();
	extern void ExitGameESC();
	extern void PressKeySpace();
	extern void screenshotF9();
	extern void screenshotF11();
}
