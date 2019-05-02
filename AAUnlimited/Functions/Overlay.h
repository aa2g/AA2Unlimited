#pragma once


namespace Overlay {
	extern HWND gameHwnd;
	extern int gameWindowWidth;
	extern int gameWindowHeight;
	extern int gameWindowMarginX;
	extern int gameWindowMarginY;
	extern HWND overlayHwnd;

	extern void CreateOverlay();
	extern void Render();
	extern void CheckPosition();
}
