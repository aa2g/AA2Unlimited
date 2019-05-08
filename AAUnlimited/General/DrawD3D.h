#pragma once

enum circle_type { full, half, quarter };
enum text_alignment { lefted, centered, righted };

namespace DrawD3D {
	extern bool fontCreated;
	extern RECT HUDarrayRect[];
	extern IUnknown *HUDarrayFont[];
	extern const wchar_t *HUDarrayText[];
	extern D3DCOLOR HUDarrayColor[];
	extern IUnknown *fontFPS;
	extern RECT rectFPS;

	extern void InitDraw(IDirect3DDevice9 *pDev);
	extern void *(WINAPI *DrawText)(IUnknown* Font, void*, LPCTSTR text, int, LPRECT rect, DWORD dt_params, D3DCOLOR color);
	extern void *(WINAPI *D3DXCreateFont)(
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
	extern void CreateFontD3d(INT Height,
		UINT Width,
		UINT Weight,
		UINT MipLevels,
		BOOL Italic,
		DWORD CharSet,
		DWORD OutputPrecision,
		DWORD Quality,
		DWORD PitchAndFamily,
		LPCTSTR pFacename,
		IUnknown **ppFont,
		bool autoScale,
		const char *error_msg);

	extern int CreateBoxFilled(float x, float y, float height,
		int count_boxes_X, D3DCOLOR color, int key_node);

	extern int CreateHalfCircleFilled(bool leftSide, double x, double y,
		double height, DWORD color, int key_node);

	extern void MakeFonts(double scale_coefficient, int true_game_margin_Y);
	extern void Render();
	//extern void Sprite(LPDIRECT3DTEXTURE9 tex, float x, float y, float resolution, float scale, float rotation);

	//=============================================================================================
	//extern void Line(float x1, float y1, float x2, float y2, float width, bool antialias, DWORD color);

	//extern void Box(float x, float y, float w, float h, float linewidth, DWORD color);
	//extern void BoxFilled(float x, float y, float w, float h, DWORD color);
	//extern void BoxBordered(float x, float y, float w, float h, float border_width, DWORD color, DWORD color_border);
	//extern void BoxRounded(float x, float y, float w, float h, float radius, bool smoothing, DWORD color, DWORD bcolor);

	//extern void Circle(float x, float y, float radius, int rotate, int type, bool smoothing, int resolution, DWORD color);
	//extern void CircleFilled(LPDIRECT3DDEVICE9 pDev, float x, float y, float rad, float rotate, int type, int resolution, DWORD color);

	//extern void Text(char *text, float x, float y, int orientation, int font, bool bordered, DWORD color, DWORD bcolor);
	//extern void Message(char *text, float x, float y, int font, int orientation);
	//=============================================================================================

	//=============================================================================================
	//extern bool Font();
	//extern void AddFont(char* Caption, float size, bool bold, bool italic);
	//extern void FontReset();
	//extern void OnLostDevice();
	//=============================================================================================
	extern void Reset();


}
