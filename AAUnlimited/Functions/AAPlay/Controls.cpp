#include "StdAfx.h"


namespace Controls {

	// Scancodes from http://www.flint.jp/misc/?q=dik&lang=en
	static const WORD ESCAPE_SCANCODE = 0x01;
	// ...

	bool keyPressed = false;
	bool needShortDelay = false;
	std::list<WORD> pressedKeys;

	void sendKey(WORD scan_code, bool key_down, bool extended = false) {
		INPUT input = { 0 };
		input.type = INPUT_KEYBOARD;
		input.ki.time = 0;
		input.ki.wScan = scan_code;
		input.ki.dwFlags = KEYEVENTF_SCANCODE;
		if (!key_down) input.ki.dwFlags |= KEYEVENTF_KEYUP;
		if (extended)    input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
		SendInput(1, &input, sizeof(input));
	}

	void keyDown(WORD scan_code, bool extended) {
		sendKey(scan_code, true, extended);
	}

	void keyUp(WORD scan_code, bool extended) {
		sendKey(scan_code, false, extended);
	}

	// Correct Full Press process: KeyDown and KeyUp on the next Rendered frame
	void keyPress(WORD scan_code, bool extended, bool needDelay) {
		keyDown(scan_code, extended);
		pressedKeys.push_back(scan_code);
		keyPressed = true;
		needShortDelay = needDelay; // if need 1 more frame to release the key
	}

	void keysRelease() { // (execute in RenderWrap.cpp)
		if (needShortDelay) { // If need 1 more frame to release the key
			needShortDelay = false;
			return;
		}
		if (keyPressed) { // Send keyup for each pressed key
			for each (const auto key_scancode in pressedKeys)
				keyUp(key_scancode);
			pressedKeys.clear();
			keyPressed = false;
		}
	}


	// Some Additional game Actions
	void EnterMenuF1() {
		keyPress(0x3B); // F1 scancode
	}

	void ExitGameESC() {
		keyPress(0x01); // ESC scancode
	}

	void PressKeySpace() {
		keyPress(0x39); // Space scancode
	}

	void screenshotF9() {
		keyPress(0x43); // F9 scancode
	}

	void screenshotF11() {
		keyPress(0x57); // F11 scancode
	}
}
