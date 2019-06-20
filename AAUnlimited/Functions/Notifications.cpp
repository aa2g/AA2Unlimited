#include "StdAfx.h"

namespace Notifications {

	bool enabled = false;
	std::wstring text;
	std::list<std::tuple< std::wstring, int >> lines;
	const int fontLayersCount = 9;
	int outlineLayersCount = 8;		// Count Outline (black) layers to show for each Notifs lines (Depends outline quality)
	RECT rect[fontLayersCount] = {	// 0 - shadow position | 0-7 - outline font pos | 8 - main font pos
		{ 1, 41, 1024, 256 },{ -1, 41, 1024, 256 },
	{ -1, 39, 1024, 256 },{ 1, 39, 1024, 256 },
	{ -1, 40, 1024, 256 },{ 1, 40, 1024, 256 },
	{ 0, 39, 1024, 256 },{ 0, 41, 1024, 256 },
	{ 0, 40, 1024, 256 }
	};
	// 0 - outline/shadow | 1 - normal notify | 2 - important
	D3DCOLOR colors[3] = { D3DCOLOR_RGBA(0, 0, 0, 255),
		D3DCOLOR_RGBA(255, 190, 100, 255), D3DCOLOR_RGBA(255, 70, 25, 255) };

	DWORD lastPopTime = 0;
	int duration = 10000;
	int maxLines = 4; // 4 for this var == 5 in game
	const char *fontFamily = "Arial";
	int fontSize = 18;
	int lineHeight = 24;
	IUnknown *Font;
	int areaPosPercentsX = 0; // in `% * 100`
	int areaPosPercentsY = 0;
	int outlineSpread = 0;
	int gameWindowWidth = 0;
	int gameWindowHeight = 0;
	int gameWindowMarginY = 0;
	DWORD notifyCentered = 0;
	bool separateColorType = true;

	void AddNotification(const char *text, int type_id) {
		if (!enabled) {
			LOGPRIO(Logger::Priority::INFO) << text << "\r\n";
			return;
		}
		if (type_id != 1) { type_id = 2; } // 1 - normal notify | 2 - important

		if (lastPopTime == 0)
			lastPopTime = GetTickCount();
		if (lines.size() > maxLines)
			lines.pop_front();
		lines.push_back(std::make_tuple(General::utf8.from_bytes(text) + L"\n", type_id));
	}

	static int HexadecimalToDecimal(std::string hex) {
		int hexLength = hex.length();
		double dec = 0;

		for (int i = 0; i < hexLength; ++i)
		{
			char b = hex[i];

			if (b >= 48 && b <= 57)
				b -= 48;
			else if (b >= 65 && b <= 70)
				b -= 55;
			dec += b * pow(16, ((hexLength - i) - 1));
		}
		return (int)dec;
	}

	D3DCOLOR sHEX_sRGB_toRGBA(std::string stringHEX_RGB, D3DCOLOR colorDefault, int alphaChannel = 255) {
		D3DCOLOR result = colorDefault;
		int R = 0;
		int G = 0;
		int B = 0;
		std::locale loc; std::string afterStrToUpper = "";
		for (std::string::size_type i = 0; i<stringHEX_RGB.length(); ++i)
			afterStrToUpper += std::toupper(stringHEX_RGB[i], loc);
		stringHEX_RGB = afterStrToUpper;

		std::smatch matches;
		std::regex regExpHEX("([A-F0-9]{6})");
		std::regex regExpRGB("(\\d{1,3})[^\\d]+(\\d{1,3})[^\\d]+(\\d{1,3})");
		std::regex regExpShortHEX("([A-F0-9]{3})");
		bool finded_color = false;
		if (std::regex_match(stringHEX_RGB, matches, regExpHEX))		// Try to find HEX
		{
			R = (unsigned char)HexadecimalToDecimal(matches[0].str().substr(0, 2));
			G = (unsigned char)HexadecimalToDecimal(matches[0].str().substr(2, 2));
			B = (unsigned char)HexadecimalToDecimal(matches[0].str().substr(4, 2));
			finded_color = true;
		}
		else if (std::regex_match(stringHEX_RGB, matches, regExpRGB)) { // Try to find RGB
			R = std::stoi(matches[1].str());
			G = std::stoi(matches[2].str());
			B = std::stoi(matches[3].str());
			finded_color = true;
		}
		else if (std::regex_match(stringHEX_RGB, matches, regExpShortHEX)) { // Try to find short HEX
			R = (unsigned char)HexadecimalToDecimal(matches[0].str().substr(0, 1) + matches[0].str().substr(0, 1));
			G = (unsigned char)HexadecimalToDecimal(matches[0].str().substr(1, 1) + matches[0].str().substr(1, 1));
			B = (unsigned char)HexadecimalToDecimal(matches[0].str().substr(2, 1) + matches[0].str().substr(2, 1));
			finded_color = true;
		}

		if (finded_color) {
			if (R > 255) { R = 255; }
			if (G > 255) { G = 255; }
			if (B > 255) { B = 255; }
			result = D3DCOLOR_RGBA(R, G, B, alphaChannel);
		}

		return result;
	}

	void InitNotificationsParams(const char *font_family, int font_size, int line_height, int show_duration, int max_lines,
		const char *text_color_normal, int diff_color_for_important, const char *text_color_important,
		int outline_quality, int outline_spread, const char *outline_color, int outline_col_A,
		int text_align, int area_pos_X, int area_pos_Y)
	{
		enabled = true;
		fontFamily = font_family;
		if (text_align == 1) { notifyCentered = DT_CENTER; area_pos_X = 0; }
		fontSize = font_size;
		lineHeight = round(fontSize * line_height / (float)100);
		duration = show_duration * 1000;
		maxLines = max_lines - 1;

		if (outline_quality == 1)
			outlineLayersCount = 1;
		else if (outline_quality == 0)
			outlineLayersCount = 0;

		areaPosPercentsX = area_pos_X;
		areaPosPercentsY = area_pos_Y;
		outlineSpread = outline_spread;
		if (gameWindowWidth > 0) 
			SetNotifyAreaSize(gameWindowWidth, gameWindowHeight, gameWindowMarginY); // If game window Width is available - Set the rectangles

		colors[0] = sHEX_sRGB_toRGBA(outline_color, colors[0], outline_col_A);
		colors[1] = sHEX_sRGB_toRGBA(text_color_normal, colors[1]);
		if (diff_color_for_important == 1)
			colors[2] = sHEX_sRGB_toRGBA(text_color_important, colors[2]);
		else {
			colors[2] = sHEX_sRGB_toRGBA(text_color_normal, colors[1]);
			separateColorType = false;
		}
	}

	void SetNotifyAreaSize(int window_width, int window_height, int margin_Y) {
		gameWindowWidth = window_width;
		gameWindowHeight = window_height;
		gameWindowMarginY = margin_Y;

		int area_pos_X = round(gameWindowWidth * areaPosPercentsX / (float)10000);
		int area_pos_Y = round(gameWindowHeight * areaPosPercentsY / (float)10000) + gameWindowMarginY;

		rect[0] = { area_pos_X + outlineSpread, area_pos_Y + outlineSpread,
			area_pos_X + outlineSpread + gameWindowWidth, area_pos_Y + outlineSpread + gameWindowHeight };
		rect[1] = { area_pos_X - outlineSpread, area_pos_Y + outlineSpread,
			area_pos_X - outlineSpread + gameWindowWidth, area_pos_Y + outlineSpread + gameWindowHeight };
		rect[2] = { area_pos_X - outlineSpread, area_pos_Y - outlineSpread,
			area_pos_X - outlineSpread + gameWindowWidth, area_pos_Y - outlineSpread + gameWindowHeight };
		rect[3] = { area_pos_X + outlineSpread, area_pos_Y - outlineSpread,
			area_pos_X + outlineSpread + gameWindowWidth, area_pos_Y - outlineSpread + gameWindowHeight };
		rect[4] = { area_pos_X - outlineSpread, area_pos_Y,
			area_pos_X - outlineSpread + gameWindowWidth, area_pos_Y + gameWindowHeight };
		rect[5] = { area_pos_X + outlineSpread, area_pos_Y,
			area_pos_X + outlineSpread + gameWindowWidth, area_pos_Y + gameWindowHeight };
		rect[6] = { area_pos_X, area_pos_Y - outlineSpread,
			area_pos_X + gameWindowWidth, area_pos_Y - outlineSpread + gameWindowHeight };
		rect[7] = { area_pos_X, area_pos_Y + outlineSpread,
			area_pos_X + gameWindowWidth, area_pos_Y + outlineSpread + gameWindowHeight };
		rect[8] = { area_pos_X, area_pos_Y, area_pos_X + gameWindowWidth, area_pos_Y + gameWindowHeight };
	}

	void SetNotificationsColor(int r, int g, int b) {
		colors[8] = D3DCOLOR_RGBA(r, g, b, 255);
	}

	VOID PopNotifications() {
		if (lines.empty())
			return;
		DWORD now = GetTickCount();
		if (now - lastPopTime > duration) {
			lines.pop_front();
			lastPopTime = lines.empty() ? 0 : now;
		}
	}

	void Render() {
		if (!enabled || lines.empty())
			return;

		PopNotifications();

		if (outlineLayersCount != 0 || separateColorType)
		{
			int line_num = 0;
			for each (const auto line in lines) // for each subs line
			{
				int top_offset = -lineHeight * (lines.size() - 1 - line_num);
				RECT *tempRect;
				for (int i = 0; i < outlineLayersCount; i++) // outline layers
				{
					tempRect = &rect[i];
					tempRect->top = tempRect->top + top_offset;
					tempRect->bottom = tempRect->bottom + top_offset;
					DrawD3D::DrawText(Font, 0, std::get<0>(line).c_str(), -1, tempRect,
						DT_NOCLIP | notifyCentered, colors[0]);
					tempRect->top = tempRect->top - top_offset;
					tempRect->bottom = tempRect->bottom - top_offset;
				}
				// Colorized text
				tempRect = &rect[fontLayersCount - 1];
				tempRect->top = tempRect->top + top_offset;
				tempRect->bottom = tempRect->bottom + top_offset;
				DrawD3D::DrawText(Font, 0, std::get<0>(line).c_str(), -1, &rect[fontLayersCount - 1],
					DT_NOCLIP | notifyCentered, colors[std::get<1>(line)]);
				tempRect->top = tempRect->top - top_offset;
				tempRect->bottom = tempRect->bottom - top_offset;

				line_num++;
			}
		}
		else { // Only Colorized text
			text.clear();
			for each (const auto line in lines)
				text = std::get<0>(line) + text;
			DrawD3D::DrawText(Font, 0, text.c_str(), -1, &rect[fontLayersCount - 1],
				DT_NOCLIP | notifyCentered, colors[1]);
		}
	}
}
