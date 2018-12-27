#include "StdAfx.h"

#include <list>

namespace Subtitles {

	std::wstring text;
	std::list<std::wstring> lines;
	RECT rect = { 0, 40, 1024, 256 };
	D3DCOLOR color = D3DCOLOR_RGBA(255, 255, 255, 255);
	DWORD lastPopTime = 0;

	void AddSubtitles(const char *subtitle) {
		if (lastPopTime == 0)
			lastPopTime = GetTickCount();
		if (lines.size() > 4)
			lines.pop_front();
		lines.push_back(General::CastToWStringN(subtitle, strlen(subtitle) + 1) + L"\n");
		SetSubtitles();
	}

	void SetSubtitles() {
		text.clear();
		for each (const auto line in lines)
			text += line;
	}

	void SetSubtitlesColor(int r, int g, int b) {
		color = D3DCOLOR_RGBA(r, g, b, 255);
	}

	VOID PopSubtitles() {
		if (lines.empty())
			return;
		DWORD now = GetTickCount();
		if (now - lastPopTime > 4000) {
			lines.pop_front();
			SetSubtitles();
			lastPopTime = lines.empty() ? 0 : now;
		}
	}
}
