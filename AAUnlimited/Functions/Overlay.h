#pragma once


namespace Overlay {
	extern HWND gameHwnd;
	extern int gameWindowWidth;
	extern int gameWindowHeight;
	extern HWND overlayHwnd;
	extern bool needRender;

	extern void CreateOverlay();
	extern void Render();
	extern void CheckPosition();
}
