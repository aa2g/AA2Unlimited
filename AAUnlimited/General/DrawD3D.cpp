#include "StdAfx.h"
#include <d3d9.h>
#include <vector>
#include "DrawD3D.h"

#pragma comment (lib, "d3d9.lib")

struct vertex
{
	FLOAT x, y, z, rhw;
	DWORD color;
};

struct sScreen
{
	float Width;
	float Height;
	float x_center;
	float y_center;
} Screen;

//#define MAX_FONTS 6


namespace DrawD3D {
	double D3DX_PI = 3.14159265358979323846;

	bool initialized = false;
	bool fontCreated = false;
	bool canRender = true;
	bool waitRenderDelay = false; // If can't render by waiting delay
	UINT waitRenderFrames = 0;
	int frame_after_start_H_now = 0;
	IDirect3DDevice9* pDevice;
	HWND gameHwnd = NULL;
	POINT cursor;
	bool cursorHUD = false;
	//LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL;
	//LPDIRECT3DINDEXBUFFER9 g_pIB = NULL;
	//int FontNr = 0;
	double scaleCoefficient = 0;
	int trueGameMarginY = 0;
	std::wstring buff_text;
	const std::wstring box_code = L"\u2586";
	const std::wstring circle_code_Left = L"\u25D6";
	const std::wstring circle_code_Right = L"\u25D7";
	const std::wstring circle_code_Full = L"\u25CF";
	float box1000MarginX = 0;	// Position and sizes of shapes for fontsize == 1000
	float box1000MarginY = 246;
	float box1000Width = 664;
	float box1000Height = 750.000;
	float circle1000MarginLeftSideX = 55;
	float circle1000MarginRightSideX = 250;
	float circle1000MarginY = 305;
	float circle1000Radius = 195.000; // (Width)
	float circle1000Height = 390;
	float circleFull1000MarginX = 94;
	float circleFull1000MarginY = 98;
	float circleFull1000Width = 812;
	float circleFull1000Height = 812;
	float cursorArrowScaledMarginX = 0;		// After scaling for current user resolution
	float cursorShadowScaledMarginX = 0;
	float cursorArrowScaledMarginY = 0;
	float cursorShadowScaledMarginY = 0;
	int cursorArrowKey = 0;
	int cursorShadowKey = 0;
	
	const int max_shapes = 1000; // 1000 - Maximum total shapes in HUD
	int key_next = 0;
	RECT HUDarrayRect[max_shapes];
	IUnknown *HUDarrayFont[max_shapes];
	std::wstring HUDarrayText[max_shapes];
	D3DCOLOR HUDarrayColor[max_shapes];
	DWORD HUDarrayFontHorizAlign[max_shapes];

	IUnknown *fontFPS;
	RECT rectFPS = { 0, 0, 256, 64 };
	IUnknown *fontTEST;

	void *(WINAPI *DrawText)(IUnknown* Font, void*, LPCTSTR text, int, 
		LPRECT rect, DWORD dt_params, D3DCOLOR color);
	void *(WINAPI *D3DXCreateFont)( IDirect3DDevice9 *pDevice,
		INT Height, UINT Width, UINT Weight, UINT MipLevels,
		BOOL Italic, DWORD CharSet, DWORD OutputPrecision, DWORD Quality,
		DWORD PitchAndFamily, LPCTSTR pFacename, IUnknown **ppFont);

	/*void Reset()
	{
		D3DVIEWPORT9 screen;
		pDevice->GetViewport(&screen);

		Screen.Width = screen.Width;
		Screen.Height = screen.Height;
		Screen.x_center = Screen.Width / 2;
		Screen.y_center = Screen.Height / 2;
	}*/

	void InitDraw(IDirect3DDevice9 *pDev) {
		pDevice = pDev;
		// fuck you microsoft for the d3dx9 SDK stupidity, no way im installing that shit
		HMODULE hm = GetModuleHandleA("d3dx9_42");
		D3DXCreateFont = decltype(D3DXCreateFont)(GetProcAddress(hm, "D3DXCreateFontW"));

		// Fill HUD RECT arrays with default values
		if (!initialized) {
			std::fill_n(HUDarrayRect, max_shapes, RECT{ 0, 0, 0, 0 });
			std::wstring empty_val = (std::wstring)(L"");
			std::fill_n(HUDarrayText, max_shapes, empty_val);
			std::fill_n(HUDarrayFontHorizAlign, max_shapes, DT_LEFT);
		}
		initialized = true;
	}

	/* This is simple font creation
	You can make his size (Height) scalable, thats depends on current game resolution. 
	(Keep in mind, all font sizes recommend to create and set 
	in CreateFontD3d() for 1920x1080 resolution)
	When you set 'autoScale' argument to 'true' in CreateFontD3d(),
	this function automaticaly scaled the size from 1920x1080 template to
	current user's resolution */
	void CreateFontD3d(INT Height, UINT Width, UINT Weight,
		UINT MipLevels, BOOL Italic, DWORD CharSet, DWORD OutputPrecision, DWORD Quality,
		DWORD PitchAndFamily, LPCTSTR pFacename, IUnknown **ppFont, bool autoScale, const char *error_msg) 
	{
		if (autoScale)
			Height = round(Height * scaleCoefficient);
		*ppFont = 0;
		D3DXCreateFont(pDevice, Height, Width, Weight, MipLevels, Italic, CharSet, OutputPrecision,
			Quality, PitchAndFamily, pFacename, *&ppFont);
		if (ppFont)
			DrawText = decltype(DrawText)(((void***)*ppFont)[0][15]);
		else
			LOGPRIONC(Logger::Priority::WARN) error_msg << "\r\n";
	}

	/* This Font Creation for HUD
	- Keep in mind, Font (with his RECT) storing in HUD memory,
	so don't create this time of fonts non-stop or every fps with key_node == -1
	- This Font support autoScale by default, as well, as all HUD Shapes.
	- If you need Vertical alignment, set the 'center_coords' to 'true' and 'x', 'y' 
	coords of center point of your font.
	- 'DT_horiz_align' == DT_LEFT / DT_CENTER / DT_RIGHT
	- 'weight' == FW_REGULAR / FW_ULTRABOLD / FW_BLACK / ...     */
	int CreateFontHUD(float x, float y, bool center_coords, // if need place Text RECT center in X:Y coords
		DWORD DT_horiz_align, float rect_maxwidth, UINT font_size, UINT weight, BOOL italic, 
		LPCTSTR pFacename, const LPCTSTR text, D3DCOLOR color, int key_node)
	{
		int key_In = key_node;
		if (key_In == -1) { // If need to find new place for Font in HUD data memory
			if (key_next >= max_shapes)
			{
				LOGPRIONC(Logger::Priority::WARN) "Out of memory for creating Font HUD\r\n";
				return -1;
			}
			key_node = key_next;
			key_next++;
		}
		// Scaling request params to current user's resolution + Margin, if resolution not 16:9
		x = x * scaleCoefficient;
		y = y * scaleCoefficient + trueGameMarginY;
		font_size = round(font_size * scaleCoefficient);
		rect_maxwidth = round(rect_maxwidth * scaleCoefficient);

		HUDarrayFont[key_node] = 0;
		D3DXCreateFont(pDevice, font_size, 0, weight, 1, false, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, pFacename, &HUDarrayFont[key_node]);
		if (HUDarrayFont[key_node])
			DrawText = decltype(DrawText)(((void***)HUDarrayFont[key_node])[0][15]);
		else
		{
			LOGPRIONC(Logger::Priority::WARN) "Error Creating Font HUD\r\n";
			if (key_In == -1)
				key_next--;
			return -1;
		}

		if (DT_horiz_align != DT_LEFT && DT_horiz_align != DT_CENTER && DT_horiz_align != DT_RIGHT)
			DT_horiz_align = DT_LEFT;

		HUDarrayFontHorizAlign[key_node] = DT_horiz_align;
		HUDarrayText[key_node] = text;
		HUDarrayColor[key_node] = color;
		HUDarrayRect[key_node].left = 0;
		HUDarrayRect[key_node].right = rect_maxwidth;
		HUDarrayRect[key_node].top = 0;
		HUDarrayRect[key_node].bottom = 0;

		// Get the total Font RECT Width and Height in HUDarrayRect[key_node]
		DrawText(HUDarrayFont[key_node], 0, HUDarrayText[key_node].c_str(), -1, &HUDarrayRect[key_node], 
			DT_NOCLIP | DT_WORDBREAK | HUDarrayFontHorizAlign[key_node] | DT_CALCRECT, HUDarrayColor[key_node]);

		int rect_height = HUDarrayRect[key_node].bottom; // total RECT Height for this font

		if (center_coords)		// if need place Shape center in X:Y coords (correction)
		{
			x -= round(rect_maxwidth * 0.5);
			y -= round(rect_height * 0.5);
		}

		HUDarrayRect[key_node].left = x;			// Total rect for result font
		HUDarrayRect[key_node].top = y;
		HUDarrayRect[key_node].right = x + rect_maxwidth;
		HUDarrayRect[key_node].bottom = y + rect_height;

		return key_node;
	}

	// Render Font for HUD with this
	void renderFontHUD(int key_node) {
		DrawText(HUDarrayFont[key_node], 0, HUDarrayText[key_node].c_str(), -1, &HUDarrayRect[key_node],
			DT_NOCLIP | DT_WORDBREAK | HUDarrayFontHorizAlign[key_node], HUDarrayColor[key_node]);
	}

	// Render Shapes for HUD with this
	void renderShapeHUD(int key_node) {
		DrawText(HUDarrayFont[key_node], 0, HUDarrayText[key_node].c_str(),
			-1, &HUDarrayRect[key_node], DT_NOCLIP, HUDarrayColor[key_node]);
	}

	void renderCursorHUD() {
		if (!cursorHUD) return; // Show only when it's need
		cursorHUD = false;
		// correct Cursor Shapes position before render
		HUDarrayRect[cursorArrowKey].left = cursor.x - cursorArrowScaledMarginX;
		HUDarrayRect[cursorArrowKey].top = cursor.y - cursorArrowScaledMarginY;
		HUDarrayRect[cursorArrowKey].right = cursor.x + 100;
		HUDarrayRect[cursorArrowKey].bottom = cursor.y + 100;

		HUDarrayRect[cursorShadowKey].left = cursor.x - cursorShadowScaledMarginX;
		HUDarrayRect[cursorShadowKey].top = cursor.y - cursorShadowScaledMarginY;
		HUDarrayRect[cursorShadowKey].right = cursor.x + 100;
		HUDarrayRect[cursorShadowKey].bottom = cursor.y + 100;

		renderShapeHUD(cursorShadowKey);
		renderShapeHUD(cursorArrowKey);
	}

	int CreateBoxFilled(float x, float y, bool center_coords, // if need place Shape center in X:Y coords
		float height, int count_boxes_X, // boxes 664x705 (for font size == 1000)
		D3DCOLOR color, int key_node)
	{
		int key_In = key_node;
		if (key_In == -1) { // If need to find new place for shape in HUD data memory
			if (key_next >= max_shapes)
			{
				LOGPRIONC(Logger::Priority::WARN) "Out of memory for creating BoxFilled HUD shape\r\n";
				return -1;
			}
			key_node = key_next;
			key_next++;
		}
		// Scaling request params to current user's resolution + Margin, if resolution not 16:9
		x = x * scaleCoefficient;
		y = y * scaleCoefficient + trueGameMarginY;
		height = height * scaleCoefficient;

		double fontRatio = height / box1000Height;
		int fontHeight = round(1000 * fontRatio); // Total Font Height
		
		HUDarrayFont[key_node] = 0;
		D3DXCreateFont(pDevice, fontHeight, 0, FW_REGULAR, 1, false, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
			General::utf8.from_bytes("Arial").c_str(), &HUDarrayFont[key_node]);
		if (HUDarrayFont[key_node])
			DrawText = decltype(DrawText)(((void***)HUDarrayFont[key_node])[0][15]);
		else
		{
			LOGPRIONC(Logger::Priority::WARN) "Error Creating BoxFilled HUD shape\r\n";
			if (key_In == -1)
				key_next--;
			return -1;
		}

		float center_correct_X = 0;  float center_correct_Y = 0;
		if (center_coords)		// if need place Shape center in X:Y coords (correction)
		{
			center_correct_X = round(box1000Width * fontRatio * count_boxes_X * 0.5);
			center_correct_Y = round(box1000Height * fontRatio * 0.5);
		}

		HUDarrayRect[key_node].left = x - center_correct_X;			// Total rect for result font
		HUDarrayRect[key_node].top = y - round(box1000MarginY * fontRatio) - center_correct_Y;
		HUDarrayRect[key_node].right = x + round(box1000Width * fontRatio * (count_boxes_X + 1)) //(+ 1 * box for reserve)
			- center_correct_X;
		HUDarrayRect[key_node].bottom = y + fontHeight - center_correct_Y;

		std::wstring buff_text;
		buff_text.clear();
		for (int i = 0; i < count_boxes_X; i++)
			buff_text += box_code;
		HUDarrayText[key_node] = buff_text;					// Total text

		HUDarrayColor[key_node] = color;

		return key_node;
	}

	int CreateHalfCircleFilled(bool leftSide, double x, double y, 
		bool center_coords, double height, DWORD color, int key_node)
	{
		int key_In = key_node;
		if (key_In == -1) { // If need to find new place for shape in HUD data memory
			if (key_next >= max_shapes)
			{
				LOGPRIONC(Logger::Priority::WARN) "Out of memory for creating HalfCircleFilled HUD shape\r\n";
				return -1;
			}
			key_node = key_next;
			key_next++;
		}
		// Scaling request params to current user's resolution
		x = x * scaleCoefficient;
		y = y * scaleCoefficient + trueGameMarginY;
		height = height * scaleCoefficient;

		double fontRatio = height / circle1000Height;
		float circle1000marginX = leftSide ? circle1000MarginLeftSideX : circle1000MarginRightSideX;
		
		int fontHeight = round(1000 * fontRatio); // Total Font Height
		
		HUDarrayFont[key_node] = 0;
		D3DXCreateFont(pDevice, fontHeight, 0, FW_REGULAR, 1, false, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
			General::utf8.from_bytes("Arial").c_str(), &HUDarrayFont[key_node]);
		if (HUDarrayFont[key_node])
			DrawText = decltype(DrawText)(((void***)HUDarrayFont[key_node])[0][15]);
		else
		{
			LOGPRIONC(Logger::Priority::WARN) "Error Creating HalfCircleFilled HUD shape\r\n";
			if(key_In == -1)
				key_next--;
			return -1;
		}

		float center_correct_X = 0;  float center_correct_Y = 0;
		if (center_coords)		// if need place Shape center in X:Y coords (correction)
		{
			center_correct_X = leftSide ? (circle1000Radius * fontRatio) : 0;
			center_correct_Y = round(circle1000Height * fontRatio * 0.5);
		}

		HUDarrayRect[key_node].left = x - round(circle1000marginX * fontRatio + center_correct_X);	// Total rect for result font
		HUDarrayRect[key_node].top = y - round(circle1000MarginY * fontRatio) - center_correct_Y;
		HUDarrayRect[key_node].right = x + round(circle1000Radius * fontRatio * 2) //(+ 1 shape for reserve)
			- center_correct_X;
		HUDarrayRect[key_node].bottom = y + fontHeight - center_correct_Y;

		HUDarrayText[key_node] = leftSide ? 				// Total text
			circle_code_Left :
			circle_code_Right;

		HUDarrayColor[key_node] = color;

		return key_node;
	}

	int CreateCircleFilled(double x, double y, bool center_coords, 
		double height, DWORD color, int key_node) 
	{
		int key_In = key_node;
		if (key_In == -1) { // If need to find new place for shape in HUD data memory
			if (key_next >= max_shapes)
			{
				LOGPRIONC(Logger::Priority::WARN) "Out of memory for creating CircleFilled HUD shape\r\n";
				return -1;
			}
			key_node = key_next;
			key_next++;
		}
		// Scaling request params to current user's resolution
		x = x * scaleCoefficient;
		y = y * scaleCoefficient + trueGameMarginY;
		height = height * scaleCoefficient;

		double fontRatio = height / circleFull1000Height;
		int fontHeight = round(1000 * fontRatio); // Total Font Height

		HUDarrayFont[key_node] = 0;
		D3DXCreateFont(pDevice, fontHeight, 0, FW_REGULAR, 1, false, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
			General::utf8.from_bytes("Arial").c_str(), &HUDarrayFont[key_node]);
		if (HUDarrayFont[key_node])
			DrawText = decltype(DrawText)(((void***)HUDarrayFont[key_node])[0][15]);
		else
		{
			LOGPRIONC(Logger::Priority::WARN) "Error Creating CircleFilled HUD shape\r\n";
			if (key_In == -1)
				key_next--;
			return -1;
		}

		float center_correct_X = 0;  float center_correct_Y = 0;
		if (center_coords)		// if need place Shape center in X:Y coords (correction)
		{
			center_correct_X = round(circleFull1000Width * fontRatio * 0.5);
			center_correct_Y = round(circleFull1000Height * fontRatio * 0.5);
		}

		HUDarrayRect[key_node].left = x - round(circleFull1000MarginX * fontRatio) - center_correct_X;	// Total rect for result font
		HUDarrayRect[key_node].top = y - round(circleFull1000MarginY * fontRatio) - center_correct_Y;
		HUDarrayRect[key_node].right = x + round(circleFull1000Width * fontRatio * 2) //(+ 1 shape for reserve)
			- center_correct_X;
		HUDarrayRect[key_node].bottom = y + fontHeight - center_correct_Y;

		HUDarrayText[key_node] = circle_code_Full;		// Total text
		HUDarrayColor[key_node] = color;

		return key_node;
	}

	int CreateCursorHUD(bool main_shape) { // main shape or shadow
		// Find new place for shape in HUD data memory
		if (key_next >= max_shapes)
		{
			LOGPRIONC(Logger::Priority::WARN) "Out of memory for creating Cursor HUD (LOL)\r\n";
			return -1;
		}
		int key_node = key_next;
		key_next++;
		// Scaling request params to current user's resolution + Margin, if resolution not 16:9
		cursorArrowScaledMarginX = round(-1 * scaleCoefficient); // Margins for Shapes
		cursorShadowScaledMarginX = round(3 * scaleCoefficient);
		cursorArrowScaledMarginY = round(8 * scaleCoefficient);
		cursorShadowScaledMarginY = round(20 * scaleCoefficient);

		int fontHeight = main_shape				// Total Font Height
			? round(50 * scaleCoefficient)		// Size for Main Shape
			: round(80 * scaleCoefficient);		// Size for Shadow Shape

		HUDarrayFont[key_node] = 0;
		D3DXCreateFont(pDevice, fontHeight, 0, FW_BLACK, 1, false, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
			General::utf8.from_bytes("Arial").c_str(), &HUDarrayFont[key_node]);
		if (HUDarrayFont[key_node])
			DrawText = decltype(DrawText)(((void***)HUDarrayFont[key_node])[0][15]);
		else
		{
			LOGPRIONC(Logger::Priority::WARN) "Error Creating Cursor HUD shape\r\n";
			key_next--;
			return -1;
		}

		HUDarrayRect[key_node].left = 0;			// Total rect for result font
		HUDarrayRect[key_node].top = 0;
		HUDarrayRect[key_node].right = 100;
		HUDarrayRect[key_node].bottom = 100;

		HUDarrayText[key_node] = L"\u25B6";// u2196 - alt arrow

		HUDarrayColor[key_node] = main_shape 
			? D3DCOLOR_ARGB(255, 0, 205, 247) 
			: D3DCOLOR_ARGB(255, 53, 26, 60);
		
		return key_node;
	}


	// ***************************** Font/Shapes Creation section ************************************
	void MakeFonts(double scale_coefficient, int true_game_margin_Y, HWND game_hwnd)
	{
		scaleCoefficient = scale_coefficient;
		trueGameMarginY = true_game_margin_Y;
		gameHwnd = game_hwnd;

		if (fontCreated) // If Second time make a fonts - overvrite shapes also (using last D3d device).
			key_next = 0;


		// FPS font
		CreateFontD3d(24, 0, FW_ULTRABOLD, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, General::utf8.from_bytes("Arial").c_str(),
			&fontFPS, false, "FPS Font creation failed");

		// Subs Font
		CreateFontD3d(Subtitles::fontSize, 0, FW_ULTRABOLD, 1, false, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
			General::utf8.from_bytes(Subtitles::fontFamily).c_str(),
			&Subtitles::Font, false, "Subs Font creation failed");

		// Notifications Font
		CreateFontD3d(Notifications::fontSize, 0, FW_ULTRABOLD, 1, false, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
			General::utf8.from_bytes(Notifications::fontFamily).c_str(),
			&Notifications::Font, false, "Notifications Font creation failed");

		// Other fonts
		// ...


		// Creating Shapes for HUD

		// Cursor shapes
		cursorArrowKey = CreateCursorHUD(true);
		cursorShadowKey = CreateCursorHUD(false);

		RadialMenu::CreateHUD(); // RadialMenu HUD fonts and shapes


		// Other HUD shapes
		// ...



		/* For Shapes _TEST
		// TEST font
		CreateBoxFilled(300, 500, true, 500, 1, D3DCOLOR_ARGB(166, 22, 255, 22), -1);
		CreateFontHUD(300, 500, true, DT_CENTER, 440, 72, FW_REGULAR, false, General::utf8.from_bytes("Arial").c_str(),
		L"Simple text for the mighty gods. Simple text for the mighty gods. Simple text for the ...",
		D3DCOLOR_ARGB(255, 244, 180, 20), -1);
		CreateFontD3d(300, 0, FW_BLACK, 1, false, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, General::utf8.from_bytes("Arial").c_str(),
		&fontTEST, false, "TEST Font creation failed");//*/

		/*CreateBoxFilled(1000, 500, false, 500, 2, D3DCOLOR_ARGB(166, 255, 22, 22), -1);
		CreateBoxFilled(1000, 500, true, 270, 2, D3DCOLOR_ARGB(166, 255, 22, 22), -1);
		CreateHalfCircleFilled(true, 1000, 500, true, 800, D3DCOLOR_ARGB(166, 255, 22, 22), -1);
		CreateHalfCircleFilled(false, 1000, 500, true, 600, D3DCOLOR_ARGB(166, 255, 22, 22), -1);//*/


		fontCreated = true;
	}

	// ************************************ Render section **************************************
	void Render() {
		GetCursorPos(&cursor);
		//RECT lbRect; GetWindowRect(gameHwnd, &lbRect); Alternative method, if not working this
		ScreenToClient(gameHwnd, &cursor); // Cursor pos inside game window


		/////////////////////////
		

		// Fonts Render
		// ...


		// Render HUD Shapes and text over them

		RadialMenu::Render();



		/* For Shapes _TEST
		RECT rectFullscreen5 = { 10, 200, 310, 700 };
		DrawText(fontTEST, 0, L"\u2586\u259B\u2586",
		-1, &rectFullscreen5, DT_NOCLIP | DT_WORDBREAK, D3DCOLOR_ARGB(166, 255, 22, 22));//*/

		// (all sign: u25D6 u25D7 u2586 | u2588 | u268B u2585  u26AB)
		/*
		RECT rectFullscreen6 = { 10, 10, 500, 500 };
		DrawText(fontTEST, 0, L"\u2586\u2588",
		-1, &rectFullscreen6, DT_NOCLIP, D3DCOLOR_ARGB(166, 255, 22, 22));
		renderShapeHUD(i);
		//*/


		// Render Cursor (Allways must be in the end of render!)
		renderCursorHUD();
	}

	// Fix against drawing on naked skin
	void canRenderDelay(UINT delay_frames) {
		if (delay_frames > 0) {
			canRender = false;
			waitRenderDelay = true;
			waitRenderFrames = (delay_frames > waitRenderFrames - frame_after_start_H_now) ?
				delay_frames : (waitRenderFrames - frame_after_start_H_now);
			frame_after_start_H_now = 0;
			return;
		}

		if (canRender)
			return;

		if (frame_after_start_H_now < waitRenderFrames) { // Wait some frames
			frame_after_start_H_now++;
			return;
		}
		frame_after_start_H_now = 0;
		waitRenderFrames = 0;
		canRender = true;
		waitRenderDelay = false;
	}

	// Currently not working (need to find a way for correct displaying 
	// D3DPT_TRIANGLELIST and D3DPT_TRIANGLEFAN to game Device)

	/*
	void CDraw::Line(float x1, float y1, float x2, float y2, float width, bool antialias, DWORD color)
	{
	ID3DXLine *m_Line;

	D3DXCreateLine(pDevice, &m_Line);
	D3DXVECTOR2 line[] = { D3DXVECTOR2(x1, y1), D3DXVECTOR2(x2, y2) };
	m_Line->SetWidth(width);
	if (antialias) m_Line->SetAntialias(1);
	m_Line->Begin();
	m_Line->Draw(line, 2, color);
	m_Line->End();
	m_Line->Release();
	}*/

	/*void Circle(float x, float y, float radius, int rotate, int type, bool smoothing, int resolution, DWORD color)
	{
	std::vector<vertex> circle(resolution + 2);
	float angle = rotate * D3DX_PI / 180;
	float pi;

	if (type == full) pi = D3DX_PI;        // Full circle
	if (type == half) pi = D3DX_PI / 2;      // 1/2 circle
	if (type == quarter) pi = D3DX_PI / 4;   // 1/4 circle

	float theta = 2.f * pi / resolution;

	for (int i = 0; i < resolution + 2; i++)
	{
	circle[i].x = (float)(x - radius * cos(i * theta));
	circle[i].y = (float)(y - radius * sin(i * theta));
	circle[i].z = 0;
	circle[i].rhw = 1;
	circle[i].color = color;
	}

	// Rotate matrix
	int _res = resolution + 2;
	for (int i = 0; i < _res; i++)
	{
	circle[i].x = x + cos(angle)*(circle[i].x - x) - sin(angle)*(circle[i].y - y);
	circle[i].y = y + sin(angle)*(circle[i].x - x) + cos(angle)*(circle[i].y - y);
	}

	pDevice->CreateVertexBuffer((resolution + 2) * sizeof(vertex), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB, NULL);

	VOID* pVertices;
	g_pVB->Lock(0, (resolution + 2) * sizeof(vertex), (void**)&pVertices, 0);
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(vertex));
	g_pVB->Unlock();


	pDevice->SetTexture(0, NULL);
	pDevice->SetPixelShader(NULL);
	if (smoothing)
	{
	pDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
	pDevice->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE);
	}
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	pDevice->SetStreamSource(0, g_pVB, 0, sizeof(vertex));
	pDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

	pDevice->DrawPrimitive(D3DPT_LINESTRIP, 0, resolution);
	if (g_pVB != NULL) g_pVB->Release();
	}

	void CircleFilled(float x, float y, float rad, float rotate, int type, int resolution, DWORD color)
	{
	std::vector<vertex> circle(resolution + 2);
	float angle = rotate * D3DX_PI / 180;
	float pi;

	if (type == full) pi = D3DX_PI;        // Full circle
	if (type == half) pi = D3DX_PI / 2;      // 1/2 circle
	if (type == quarter) pi = D3DX_PI / 4;   // 1/4 circle

	circle[0].x = x;
	circle[0].y = y;
	circle[0].z = 0;
	circle[0].rhw = 1;
	circle[0].color = color;

	for (int i = 1; i < resolution + 2; i++)
	{
	circle[i].x = (float)(x - rad * cos(pi*((i - 1) / (resolution / 2.0f))));
	circle[i].y = (float)(y - rad * sin(pi*((i - 1) / (resolution / 2.0f))));
	circle[i].z = 0;
	circle[i].rhw = 1;
	circle[i].color = color;
	}

	// Rotate matrix
	int _res = resolution + 2;
	for (int i = 0; i < _res; i++)
	{
	circle[i].x = x + cos(angle)*(circle[i].x - x) - sin(angle)*(circle[i].y - y);
	circle[i].y = y + sin(angle)*(circle[i].x - x) + cos(angle)*(circle[i].y - y);
	}

	pDevice->CreateVertexBuffer((resolution + 2) * sizeof(vertex), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB, NULL);

	VOID* pVertices;
	g_pVB->Lock(0, (resolution + 2) * sizeof(vertex), (void**)&pVertices, 0);
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(vertex));
	g_pVB->Unlock();

	pDevice->SetTexture(0, NULL);
	pDevice->SetPixelShader(NULL);
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	pDevice->SetStreamSource(0, g_pVB, 0, sizeof(vertex));
	pDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, resolution);
	if (g_pVB != NULL) g_pVB->Release();
	}

	void BoxFilled(float x, float y, float w, float h, DWORD color)
	{
	vertex V[4];

	V[0].color = V[1].color = V[2].color = V[3].color = color;

	V[0].z = V[1].z = V[2].z = V[3].z = 0;
	V[0].rhw = V[1].rhw = V[2].rhw = V[3].rhw = 0;

	V[0].x = x;
	V[0].y = y;
	V[1].x = x + w;
	V[1].y = y;
	V[2].x = x + w;
	V[2].y = y + h;
	V[3].x = x;
	V[3].y = y + h;

	unsigned short indexes[] = { 0, 1, 3, 1, 2, 3 };

	pDevice->CreateVertexBuffer(4 * sizeof(vertex), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB, NULL);
	pDevice->CreateIndexBuffer(2 * sizeof(short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &g_pIB, NULL);

	VOID* pVertices;
	g_pVB->Lock(0, sizeof(V), (void**)&pVertices, 0);
	memcpy(pVertices, V, sizeof(V));
	g_pVB->Unlock();

	VOID* pIndex;
	g_pIB->Lock(0, sizeof(indexes), (void**)&pIndex, 0);
	memcpy(pIndex, indexes, sizeof(indexes));
	g_pIB->Unlock();

	pDevice->SetTexture(0, NULL);
	pDevice->SetPixelShader(NULL);
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	pDevice->SetStreamSource(0, g_pVB, 0, sizeof(vertex));
	pDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	pDevice->SetIndices(g_pIB);

	pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);

	g_pVB->Release();
	g_pIB->Release();
	}

	void Box(float x, float y, float w, float h, float linewidth, DWORD color)
	{
	if (linewidth == 0 || linewidth == 1)
	{
	BoxFilled(x, y, w, 1, color);             // Top
	BoxFilled(x, y + h - 1, w, 1, color);         // Bottom
	BoxFilled(x, y + 1, 1, h - 2 * 1, color);       // Left
	BoxFilled(x + w - 1, y + 1, 1, h - 2 * 1, color);   // Right
	}
	else
	{
	BoxFilled(x, y, w, linewidth, color);                                     // Top
	BoxFilled(x, y + h - linewidth, w, linewidth, color);                         // Bottom
	BoxFilled(x, y + linewidth, linewidth, h - 2 * linewidth, color);               // Left
	BoxFilled(x + w - linewidth, y + linewidth, linewidth, h - 2 * linewidth, color);   // Right
	}
	}

	void BoxBordered(float x, float y, float w, float h, float border_width, DWORD color, DWORD color_border)
	{
	BoxFilled(x, y, w, h, color);
	Box(x - border_width, y - border_width, w + 2 * border_width, h + border_width, border_width, color_border);
	}//*/

	/*void CDraw::BoxRounded(float x, float y, float w, float h, float radius, bool smoothing, DWORD color, DWORD bcolor)
	{
	BoxFilled(x + radius, y + radius, w - 2 * radius - 1, h - 2 * radius - 1, color);   // Center rect.
	BoxFilled(x + radius, y + 1, w - 2 * radius - 1, radius - 1, color);            // Top rect.
	BoxFilled(x + radius, y + h - radius - 1, w - 2 * radius - 1, radius, color);     // Bottom rect.
	BoxFilled(x + 1, y + radius, radius - 1, h - 2 * radius - 1, color);            // Left rect.
	BoxFilled(x + w - radius - 1, y + radius, radius, h - 2 * radius - 1, color);     // Right rect.

	// Smoothing method
	if (smoothing)
	{
	CircleFilled(x + radius, y + radius, radius - 1, 0, quarter, 16, color);             // Top-left corner
	CircleFilled(x + w - radius - 1, y + radius, radius - 1, 90, quarter, 16, color);        // Top-right corner
	CircleFilled(x + w - radius - 1, y + h - radius - 1, radius - 1, 180, quarter, 16, color);   // Bottom-right corner
	CircleFilled(x + radius, y + h - radius - 1, radius - 1, 270, quarter, 16, color);       // Bottom-left corner

	Circle(x + radius + 1, y + radius + 1, radius, 0, quarter, true, 16, bcolor);          // Top-left corner
	Circle(x + w - radius - 2, y + radius + 1, radius, 90, quarter, true, 16, bcolor);       // Top-right corner
	Circle(x + w - radius - 2, y + h - radius - 2, radius, 180, quarter, true, 16, bcolor);    // Bottom-right corner
	Circle(x + radius + 1, y + h - radius - 2, radius, 270, quarter, true, 16, bcolor);      // Bottom-left corner

	Line(x + radius, y + 1, x + w - radius - 1, y + 1, 1, false, bcolor);       // Top line
	Line(x + radius, y + h - 2, x + w - radius - 1, y + h - 2, 1, false, bcolor);   // Bottom line
	Line(x + 1, y + radius, x + 1, y + h - radius - 1, 1, false, bcolor);       // Left line
	Line(x + w - 2, y + radius, x + w - 2, y + h - radius - 1, 1, false, bcolor);   // Right line
	}
	else
	{
	CircleFilled(x + radius, y + radius, radius, 0, quarter, 16, color);             // Top-left corner
	CircleFilled(x + w - radius - 1, y + radius, radius, 90, quarter, 16, color);        // Top-right corner
	CircleFilled(x + w - radius - 1, y + h - radius - 1, radius, 180, quarter, 16, color);   // Bottom-right corner
	CircleFilled(x + radius, y + h - radius - 1, radius, 270, quarter, 16, color);       // Bottom-left corner
	}
	}

	void CDraw::Text(char *text, float x, float y, int orientation, int font, bool bordered, DWORD color, DWORD bcolor)
	{
	RECT rect;

	switch (orientation)
	{
	case lefted:
	if (bordered)
	{
	SetRect(&rect, x - 1, y, x - 1, y);
	pFont[font]->DrawTextA(NULL, text, -1, &rect, DT_LEFT | DT_NOCLIP, bcolor);
	SetRect(&rect, x + 1, y, x + 1, y);
	pFont[font]->DrawTextA(NULL, text, -1, &rect, DT_LEFT | DT_NOCLIP, bcolor);
	SetRect(&rect, x, y - 1, x, y - 1);
	pFont[font]->DrawTextA(NULL, text, -1, &rect, DT_LEFT | DT_NOCLIP, bcolor);
	SetRect(&rect, x, y + 1, x, y + 1);
	pFont[font]->DrawTextA(NULL, text, -1, &rect, DT_LEFT | DT_NOCLIP, bcolor);
	}
	SetRect(&rect, x, y, x, y);
	pFont[font]->DrawTextA(NULL, text, -1, &rect, DT_LEFT | DT_NOCLIP, color);
	break;
	case centered:
	if (bordered)
	{
	SetRect(&rect, x - 1, y, x - 1, y);
	pFont[font]->DrawTextA(NULL, text, -1, &rect, DT_CENTER | DT_NOCLIP, bcolor);
	SetRect(&rect, x + 1, y, x + 1, y);
	pFont[font]->DrawTextA(NULL, text, -1, &rect, DT_CENTER | DT_NOCLIP, bcolor);
	SetRect(&rect, x, y - 1, x, y - 1);
	pFont[font]->DrawTextA(NULL, text, -1, &rect, DT_CENTER | DT_NOCLIP, bcolor);
	SetRect(&rect, x, y + 1, x, y + 1);
	pFont[font]->DrawTextA(NULL, text, -1, &rect, DT_CENTER | DT_NOCLIP, bcolor);
	}
	SetRect(&rect, x, y, x, y);
	pFont[font]->DrawTextA(NULL, text, -1, &rect, DT_CENTER | DT_NOCLIP, color);
	break;
	case righted:
	if (bordered)
	{
	SetRect(&rect, x - 1, y, x - 1, y);
	pFont[font]->DrawTextA(NULL, text, -1, &rect, DT_RIGHT | DT_NOCLIP, bcolor);
	SetRect(&rect, x + 1, y, x + 1, y);
	pFont[font]->DrawTextA(NULL, text, -1, &rect, DT_RIGHT | DT_NOCLIP, bcolor);
	SetRect(&rect, x, y - 1, x, y - 1);
	pFont[font]->DrawTextA(NULL, text, -1, &rect, DT_RIGHT | DT_NOCLIP, bcolor);
	SetRect(&rect, x, y + 1, x, y + 1);
	pFont[font]->DrawTextA(NULL, text, -1, &rect, DT_RIGHT | DT_NOCLIP, bcolor);
	}
	SetRect(&rect, x, y, x, y);
	pFont[font]->DrawTextA(NULL, text, -1, &rect, DT_RIGHT | DT_NOCLIP, color);
	break;
	}
	}

	void CDraw::Message(char *text, float x, float y, int font, int orientation)
	{
	RECT rect = { x, y, x, y };

	switch (orientation)
	{
	case lefted:
	pFont[font]->DrawTextA(NULL, text, -1, &rect, DT_CALCRECT | DT_LEFT, BLACK(255));

	BoxRounded(x - 5, rect.top - 5, rect.right - x + 10, rect.bottom - rect.top + 10, 5, true, DARKGRAY(150), SKYBLUE(255));

	SetRect(&rect, x, y, x, y);
	pFont[font]->DrawTextA(NULL, text, -1, &rect, DT_LEFT | DT_NOCLIP, ORANGE(255));
	break;
	case centered:
	pFont[font]->DrawTextA(NULL, text, -1, &rect, DT_CALCRECT | DT_CENTER, BLACK(255));

	BoxRounded(rect.left - 5, rect.top - 5, rect.right - rect.left + 10, rect.bottom - rect.top + 10, 5, true, DARKGRAY(150), SKYBLUE(255));

	SetRect(&rect, x, y, x, y);
	pFont[font]->DrawTextA(NULL, text, -1, &rect, DT_CENTER | DT_NOCLIP, ORANGE(255));
	break;
	case righted:
	pFont[font]->DrawTextA(NULL, text, -1, &rect, DT_CALCRECT | DT_RIGHT, BLACK(255));

	BoxRounded(rect.left - 5, rect.top - 5, rect.right - rect.left + 10, rect.bottom - rect.top + 10, 5, true, DARKGRAY(150), SKYBLUE(255));

	SetRect(&rect, x, y, x, y);
	pFont[font]->DrawTextA(NULL, text, -1, &rect, DT_RIGHT | DT_NOCLIP, ORANGE(255));
	break;
	}
	}

	void CDraw::Sprite(LPDIRECT3DTEXTURE9 tex, float x, float y, float resolution, float scale, float rotation)
	{
	float screen_pos_x = x;
	float screen_pos_y = y;

	// Texture being used is 64x64:
	D3DXVECTOR2 spriteCentre = D3DXVECTOR2(resolution / 2, resolution / 2);

	// Screen position of the sprite
	D3DXVECTOR2 trans = D3DXVECTOR2(screen_pos_x, screen_pos_y);

	// Build our matrix to rotate, scale and position our sprite
	D3DXMATRIX mat;

	D3DXVECTOR2 scaling(scale, scale);

	// out, scaling centre, scaling rotation, scaling, rotation centre, rotation, translation
	D3DXMatrixTransformation2D(&mat, NULL, 0.0, &scaling, &spriteCentre, rotation, &trans);

	//pDevice->SetRenderState(D3DRS_ZENABLE, false);
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	pDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	pDevice->SetPixelShader(NULL);
	sSprite->Begin(NULL);
	sSprite->SetTransform(&mat); // Tell the sprite about the matrix
	sSprite->Draw(tex, NULL, NULL, NULL, 0xFFFFFFFF);
	sSprite->End();
	}

	bool CDraw::Font()
	{
	for (int i = 0; i<FontNr; i++)
	if (pFont[i]) return false;
	return true;
	}

	void CDraw::AddFont(char* Caption, float size, bool bold, bool italic)
	{
	D3DXCreateFont(pDevice, size, 0, (bold) ? FW_BOLD : FW_NORMAL, 1, (italic) ? 1 : 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, Caption, &pFont[++FontNr]);
	}

	void CDraw::FontReset()
	{
	for (int i = 0; i < FontNr; i++) pFont[i]->OnLostDevice();
	for (int i = 0; i < FontNr; i++) pFont[i]->OnResetDevice();
	}

	void CDraw::OnLostDevice()
	{
	for (int i = 0; i < FontNr; i++) pFont[i]->OnLostDevice();
	}*/
}
