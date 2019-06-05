#pragma once

//enum circle_type { full, half, quarter };
//enum text_alignment { lefted, centered, righted };

namespace DrawD3D {
	extern bool fontCreated;
	extern RECT HUDarrayRect[];
	extern IUnknown *HUDarrayFont[];
	extern const wchar_t *HUDarrayText[];
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

	extern int CreateBoxFilled(float x, float y, float height,
		int count_boxes_X, D3DCOLOR color, int key_node);

	extern int CreateHalfCircleFilled(bool leftSide, double x, double y,
		double height, DWORD color, int key_node);

	extern void MakeFonts(double scale_coefficient, int true_game_margin_Y);
	extern void Render();
	


}
