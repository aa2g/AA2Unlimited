#pragma once

#include <list>
#include <regex>

namespace Subtitles {
	extern bool enabled;
	extern int fontSize;
	extern const char *fontFamily;
	extern IUnknown *Font;
	extern int gameWindowWidth;
	extern int gameWindowHeight;

	void AddSubtitles(const char *subtitles, const char *file_name);
	void InitSubtitlesParams(const char *font_family, int font_size, int line_height, int show_duration, int max_lines,
		const char *text_color_female, int diff_color_for_male, const char *text_color_male, 
		int outline_quality, int outline_spread, const char *outline_color, int outline_col_A,
		int text_align, int area_pos_X, int area_pos_Y);
	void SetSubsAreaSize(int window_width, int window_height, int margin_Y);
	void SetSubtitlesColor(int r, int g, int b);
	void Render();
}
