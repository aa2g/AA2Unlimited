#pragma once

#include <d3d9.h>
#include <string>
#include <list>
#include <regex>

namespace Subtitles {
	extern std::wstring text;
	extern std::list<std::tuple< std::wstring, int >> lines;
	extern const int fontLayersCount;
	extern int outlineLayersCount;
	extern RECT rect[];
	extern D3DCOLOR colors[];
	extern int fontSize;
	extern int lineHeight;
	extern const char *fontFamily;
	extern int gameWindowWidth;
	extern DWORD subsCentered;
	extern bool separateColorMale;

	void AddSubtitles(const char *subtitles, const char *file_name);
	void InitSubtitlesParams(const char *font_family, int font_size, int line_height, int show_duration, int max_lines,
		const char *text_color_female, int diff_color_for_male, const char *text_color_male, 
		int outline_quality, int outline_spread, const char *outline_color, int outline_col_A,
		int text_align, int area_pos_X, int area_pos_Y);
	void CorrectSubsAreaSize();
	void SetSubtitlesColor(int r, int g, int b);
	void PopSubtitles();
}
