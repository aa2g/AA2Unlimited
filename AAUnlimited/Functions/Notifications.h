#pragma once


#include <list>

namespace Notifications {
	extern bool enabled;
	extern int fontSize;
	extern const char *fontFamily;
	extern IUnknown *Font;
	extern int gameWindowWidth;
	extern int gameWindowHeight;

	void AddNotification(const char *text, int type_id);
	void InitNotificationsParams(const char *font_family, int font_size, int line_height, int show_duration, int max_lines,
		const char *text_color_normal, int diff_color_for_important, const char *text_color_important,
		int outline_quality, int outline_spread, const char *outline_color, int outline_col_A,
		int text_align, int area_pos_X, int area_pos_Y);
	void SetNotifyAreaSize(int window_width, int window_height, int margin_Y);
	void SetNotificationsColor(int r, int g, int b);
	void Render();
}
