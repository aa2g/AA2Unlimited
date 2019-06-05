#include "StdAfx.h"
#include <Dwmapi.h>

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "Dwmapi.lib")

namespace Overlay {

	HWND gameHwnd = NULL;
	int gameWindowWidth = 800;
	int gameWindowHeight = 600;
	int gameWindowMarginX = 0;
	int gameWindowMarginY = 0;
	RECT gameWindowRECT;
	const MARGINS margin = { 0,0,gameWindowWidth, gameWindowHeight };

	HWND overlayHwnd = NULL;
	int overlayPositionX = 9999;
	int overlayPositionY = 9999;
	LPDIRECT3D9 d3d;    // the pointer to our Direct3D interface
	LPDIRECT3DDEVICE9 d3ddev;
	IUnknown *pFont;
	void *(WINAPI *DrawText)(IUnknown *, void*, LPCTSTR, int, LPRECT, DWORD, D3DCOLOR);


	void CheckPosition() {
		GetWindowRect(gameHwnd, &gameWindowRECT);
		//ClientToScreen(gameHwnd, reinterpret_cast<POINT*>(&gameWindowRECT.left)); // convert top-left corner to screen coords
		if (overlayPositionX == gameWindowRECT.left + gameWindowMarginX 
			&& overlayPositionY == gameWindowRECT.top + gameWindowMarginY)
			return;
		// Get the offsets X and Y to Inner Area (without game window borders)
		if (gameWindowRECT.bottom - gameWindowRECT.top - gameWindowHeight - gameWindowMarginX != gameWindowMarginY) {
			gameWindowMarginX = round((gameWindowRECT.right - gameWindowRECT.left - gameWindowWidth) / 2);
			gameWindowMarginY = gameWindowRECT.bottom - gameWindowRECT.top - gameWindowHeight - gameWindowMarginX;
			LOGPRIONC(Logger::Priority::INFO) "Calculated offsets: " << gameWindowMarginX << "_" << gameWindowMarginY << "!\r\n";
		}

		LOGPRIONC(Logger::Priority::INFO) "Wind moving (func)" << gameWindowRECT.left << "_" << gameWindowMarginX 
			<< "_" << gameWindowRECT.top << "_" << gameWindowMarginY << "!\r\n";
		// Set Overlay position, if game window moved
		overlayPositionX = gameWindowRECT.left + gameWindowMarginX;
		overlayPositionY = gameWindowRECT.top + gameWindowMarginY;
		::SetWindowPos(overlayHwnd, HWND_NOTOPMOST, overlayPositionX, overlayPositionY, gameWindowWidth, gameWindowHeight, SWP_NOSIZE);
	}

	LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
			case WM_PAINT:
				DwmExtendFrameIntoClientArea(hWnd, &margin); 
			break;
			case WM_DESTROY: 
				PostQuitMessage(0); 
				return 0; 
			break;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	void initD3D(HWND hWnd)
	{
		d3d = Direct3DCreate9(D3D_SDK_VERSION);    // create the Direct3D interface

		D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information

		ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
		d3dpp.Windowed = TRUE;    // program windowed, not fullscreen
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
		d3dpp.hDeviceWindow = hWnd;    // set the window to be used by Direct3D
		d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;     // set the back buffer format to 32-bit
		d3dpp.BackBufferWidth = gameWindowWidth;    // set the width of the buffer
		d3dpp.BackBufferHeight = gameWindowHeight;    // set the height of the buffer

		d3dpp.EnableAutoDepthStencil = TRUE;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

		// create a device class using this information and the info from the d3dpp stuct
		d3d->CreateDevice(D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			hWnd,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&d3dpp,
			&d3ddev);


		HMODULE hm = GetModuleHandleA("d3dx9_42");
		void *(WINAPI *D3DXCreateFont)(
			IDirect3DDevice9 *pDevice,
			INT               Height,
			UINT              Width,
			UINT              Weight,
			UINT              MipLevels,
			BOOL              Italic,
			DWORD             CharSet,
			DWORD             OutputPrecision,
			DWORD             Quality,
			DWORD             PitchAndFamily,
			LPCTSTR           pFacename,
			IUnknown        **ppFont
			);
		D3DXCreateFont = decltype(D3DXCreateFont)(GetProcAddress(hm, "D3DXCreateFontW"));


		pFont = 0;			// Overlay default font
		D3DXCreateFont(d3ddev, 24, 0, FW_ULTRABOLD, 1, false, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, General::utf8.from_bytes("Arial").c_str(), &pFont);
		if (pFont)
			DrawText = decltype(DrawText)(((void***)pFont)[0][15]);
		else
			LOGPRIONC(Logger::Priority::WARN) "Overlay Font creation failed:\r\n";

		Subtitles::Font = 0; // Subs Font
		D3DXCreateFont(d3ddev, Subtitles::fontSize, 0, FW_ULTRABOLD, 1, false, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, General::utf8.from_bytes(Subtitles::fontFamily).c_str(), &Subtitles::Font);
		if (Subtitles::Font)
			DrawText = decltype(DrawText)(((void***)Subtitles::Font)[0][15]);
		else
			LOGPRIONC(Logger::Priority::WARN) "Subs Font creation failed:\r\n";
		

	}

	void Render()
	{
		if (Overlay::overlayHwnd == NULL) return;
		// clear the window alpha
		d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(1, 75, 0, 0), 1.0f, 0);
		d3ddev->BeginScene();

		RECT rect = { 10, 40, 500, 500 }; // debug
		DrawText(pFont, 0, L"Overlay test v0.0.1", -1, &rect, DT_NOCLIP, D3DCOLOR_ARGB(100, 0, 255, 0));

		// 1) Subtitles
		if (g_Config.bDisplaySubs) {
			if (!Subtitles::lines.empty()) {
				if (Subtitles::outlineLayersCount != 0 || Subtitles::separateColorMale)
				{
					int line_num = 0;
					for each (const auto line in Subtitles::lines) // for each subs line
					{
						int top_offset = Subtitles::lineHeight * line_num;
						RECT *tempRect;
						for (int i = 0; i < Subtitles::outlineLayersCount; i++) // outline layers
						{
							tempRect = &Subtitles::rect[i];
							tempRect->top = tempRect->top + top_offset;
							tempRect->bottom = tempRect->bottom + top_offset;
							DrawText(Subtitles::Font, 0, std::get<0>(line).c_str(), -1, tempRect, 
								DT_NOCLIP | Subtitles::subsCentered, Subtitles::colors[0]);
							tempRect->top = tempRect->top - top_offset;
							tempRect->bottom = tempRect->bottom - top_offset;
						}
						// Colorized text
						tempRect = &Subtitles::rect[Subtitles::fontLayersCount - 1];
						tempRect->top = tempRect->top + top_offset;
						tempRect->bottom = tempRect->bottom + top_offset;
						DrawText(Subtitles::Font, 0, std::get<0>(line).c_str(), -1, &Subtitles::rect[Subtitles::fontLayersCount - 1], 
							DT_NOCLIP | Subtitles::subsCentered, Subtitles::colors[std::get<1>(line)]);
						tempRect->top = tempRect->top - top_offset;
						tempRect->bottom = tempRect->bottom - top_offset;

						line_num++;
					}
				}
				else { // Only Colorized text
					Subtitles::text.clear();
					for each (const auto line in Subtitles::lines)
						Subtitles::text += std::get<0>(line);
					DrawText(Subtitles::Font, 0, Subtitles::text.c_str(), -1, &Subtitles::rect[Subtitles::fontLayersCount - 1], 
						DT_NOCLIP | Subtitles::subsCentered, Subtitles::colors[1]);
				}
			}
		}


		// 2) Test image
		// ...

		LOGPRIONC(Logger::Priority::INFO) "Render()\r\n"; // debug
		d3ddev->EndScene();
		d3ddev->Present(NULL, NULL, NULL, NULL);   // displays the created frame on the screen
	}

	void CreateOverlay() {
		if (overlayHwnd != NULL) return;
		WNDCLASSEX wc;

		ZeroMemory(&wc, sizeof(WNDCLASSEX));

		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = General::DllInst;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)RGB(0, 0, 0);
		wc.lpszClassName = L"UnlimitedOverlay";

		if (RegisterClassEx(&wc) == 0) { LOGPRIONC(Logger::Priority::INFO) "------------------Register Class failed----------\r\n"; }
		
		overlayHwnd = CreateWindowEx(0,
			L"UnlimitedOverlay",
			L"AA2Unlimited Overlay",
			/*WS_EX_TOPMOST |*/ WS_POPUP,
			0, 0,
			gameWindowWidth, gameWindowHeight,
			gameHwnd,
			NULL,
			General::DllInst,
			NULL);
		if (overlayHwnd == NULL)
		{
			wchar_t *error = new wchar_t;
			wsprintf(error, L"error %X", GetLastError());
			LOGPRIONC(Logger::Priority::WARN) "Overlay error:" << error << "\r\n";
			return;
		}

		SetWindowLong(overlayHwnd, GWL_EXSTYLE, (int)GetWindowLong(overlayHwnd, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);
		SetLayeredWindowAttributes(overlayHwnd, RGB(0, 0, 0), 0, ULW_COLORKEY);
		SetLayeredWindowAttributes(overlayHwnd, 0, 255, LWA_ALPHA);

		ShowWindow(overlayHwnd, SW_SHOWDEFAULT);
		
		initD3D(overlayHwnd);

		::SetWindowPos(gameHwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		CheckPosition();
		Render();
	}
	
}
