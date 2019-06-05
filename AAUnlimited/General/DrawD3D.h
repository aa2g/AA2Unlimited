#pragma once

enum circle_type { full, half, quarter };
enum text_alignment { lefted, centered, righted };

namespace DrawD3D {
	extern void GetDevice(LPDIRECT3DDEVICE9 pDev);

	//extern void Sprite(LPDIRECT3DTEXTURE9 tex, float x, float y, float resolution, float scale, float rotation);

	//=============================================================================================
	//extern void Line(float x1, float y1, float x2, float y2, float width, bool antialias, DWORD color);

	extern void Box(float x, float y, float w, float h, float linewidth, DWORD color);
	extern void BoxFilled(float x, float y, float w, float h, DWORD color);
	extern void BoxBordered(float x, float y, float w, float h, float border_width, DWORD color, DWORD color_border);
	//extern void BoxRounded(float x, float y, float w, float h, float radius, bool smoothing, DWORD color, DWORD bcolor);

	extern void Circle(float x, float y, float radius, int rotate, int type, bool smoothing, int resolution, DWORD color);
	extern void CircleFilled(LPDIRECT3DDEVICE9 pDev, float x, float y, float rad, float rotate, int type, int resolution, DWORD color);

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
