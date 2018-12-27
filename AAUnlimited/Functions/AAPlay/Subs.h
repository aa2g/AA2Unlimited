#pragma once

#include <d3d9.h>
#include <string>

namespace Subtitles {
	extern std::wstring text;
	extern RECT rect;
	extern D3DCOLOR color;
	extern DWORD lastPopTime;

	void AddSubtitles(const char *subtitles);
	void SetSubtitles();
	void SetSubtitlesColor(int r, int g, int b);
	void PopSubtitles();
}
