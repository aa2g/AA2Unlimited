#pragma once

//enum circle_type { full, half, quarter };
//enum text_alignment { lefted, centered, righted };

namespace DrawD3D {
	extern double D3DX_PI;

	extern bool fontCreated;
	extern bool canRender;
	extern bool waitRenderDelay;
	extern HWND gameHwnd;
	extern POINT cursor;
	extern RECT HUDarrayRect[];
	extern IUnknown *HUDarrayFont[];
	extern std::wstring HUDarrayText[];
	extern D3DCOLOR HUDarrayColor[];
	extern IUnknown *fontFPS;
	extern RECT rectFPS;

	extern void InitDraw(IDirect3DDevice9 *pDev);

	extern void *(WINAPI *DrawText)(IUnknown* Font, void*, LPCTSTR text, int, 
		LPRECT rect, DWORD dt_params, D3DCOLOR color);

	extern void *(WINAPI *D3DXCreateFont)( IDirect3DDevice9 *pDevice,
		INT Height, UINT Width, UINT Weight, UINT MipLevels, 
		BOOL Italic, DWORD CharSet, DWORD OutputPrecision, DWORD Quality, 
		DWORD PitchAndFamily, LPCTSTR pFacename, IUnknown **ppFont);

	extern void CreateFontD3d(INT Height, UINT Width, UINT Weight,
		UINT MipLevels, BOOL Italic, DWORD CharSet, DWORD OutputPrecision,
		DWORD Quality, DWORD PitchAndFamily, LPCTSTR pFacename,
		IUnknown **ppFont, bool autoScale, const char *error_msg);

	extern int CreateFontHUD(float x, float y, bool center_coords,
		DWORD DT_horiz_align, float rect_maxwidth, UINT font_size, UINT weight, BOOL italic,
		LPCTSTR pFacename, const LPCTSTR text, D3DCOLOR color, int key_node);
	
	extern int CreateBoxFilled(float x, float y, bool center_coords, 
		float height, int count_boxes_X, D3DCOLOR color, int key_node);

	extern int CreateHalfCircleFilled(bool leftSide, double x, double y, 
		bool center_coords, double height, DWORD color, int key_node);

	extern int CreateCircleFilled(double x, double y, bool center_coords,
		double height, DWORD color, int key_node);

	extern void renderFontHUD(int key_node);
	extern void renderShapeHUD(int key_node);

	extern void MakeFonts(double scale_coefficient, int true_game_margin_Y, HWND game_hwnd);
	extern void Render();

	extern void canRenderDelay(UINT delay_frames = 0);

}
