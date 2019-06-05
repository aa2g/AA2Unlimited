#include "StdAfx.h"


namespace Subtitles {

	std::wstring text;
	std::list<std::tuple< std::wstring, int >> lines;
	const int fontLayersCount = 9;
	int outlineLayersCount = 8;		// Count Outline (black) layers to show for each Subs lines (Depends outline quality)
	RECT rect[fontLayersCount] = {	// 0 - shadow position | 0-7 - outline font pos | 8 - main font pos
		{ 1, 41, 1024, 256 }, { -1, 41, 1024, 256 }, 
		{ -1, 39, 1024, 256 }, { 1, 39, 1024, 256 },
		{ -1, 40, 1024, 256 },{ 1, 40, 1024, 256 },
		{ 0, 39, 1024, 256 },{ 0, 41, 1024, 256 },
		{ 0, 40, 1024, 256 } 
	};
	// 0 - outline/shadow | 1 - female | 2 - male
	D3DCOLOR colors[3] = { D3DCOLOR_RGBA(0, 0, 0, 255), 
		D3DCOLOR_RGBA(255, 155, 255, 255), D3DCOLOR_RGBA(155, 244, 244, 255) };

	DWORD lastPopTime = 0;
	int duration = 4000;
	int maxLines = 3; // 3 for this var == 4 in game
	const char *fontFamily = "Arial";
	int fontSize = 24;
	int lineHeight = 36;
	int gameWindowWidth = 0;
	DWORD subsCentered = 0;
	bool separateColorMale = true;

	void AddSubtitles(const char *subtitle, const char *file_name) {
		int sexes_id = 1;
		if (file_name[0] == *"s") { sexes_id = 2; }

		if (lastPopTime == 0)
			lastPopTime = GetTickCount();
		if (lines.size() > maxLines)
			lines.pop_front();
		lines.push_back(std::make_tuple(General::utf8.from_bytes(subtitle) + L"\n", sexes_id));
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

	void InitSubtitlesParams(const char *font_family, int font_size, int line_height, int show_duration, int max_lines,
		const char *text_color_female, int diff_color_for_male, const char *text_color_male,
		int outline_quality, int outline_spread, const char *outline_color, int outline_col_A,
		int text_align, int area_pos_X, int area_pos_Y)
	{
		fontFamily = font_family;
		if (text_align == 1) { subsCentered = DT_CENTER; area_pos_X = 0; }
		fontSize = font_size;
		lineHeight = round(fontSize * line_height / 100);
		duration = show_duration * 1000;
		maxLines = max_lines - 1;

		if (outline_quality == 1)
			outlineLayersCount = 1;
		else if (outline_quality == 0)
			outlineLayersCount = 0;

		int area_coords_Right = area_pos_X + 100; // Temporary params
		int area_coords_Bottom = area_pos_Y + 100;
		rect[0] = { area_pos_X + outline_spread, area_pos_Y + outline_spread, area_coords_Right, area_coords_Bottom };
		rect[1] = { area_pos_X - outline_spread, area_pos_Y + outline_spread, area_coords_Right, area_coords_Bottom };
		rect[2] = { area_pos_X - outline_spread, area_pos_Y - outline_spread, area_coords_Right, area_coords_Bottom };
		rect[3] = { area_pos_X + outline_spread, area_pos_Y - outline_spread, area_coords_Right, area_coords_Bottom };
		rect[4] = { area_pos_X - outline_spread, area_pos_Y, area_coords_Right, area_coords_Bottom };
		rect[5] = { area_pos_X + outline_spread, area_pos_Y, area_coords_Right, area_coords_Bottom };
		rect[6] = { area_pos_X, area_pos_Y - outline_spread, area_coords_Right, area_coords_Bottom };
		rect[7] = { area_pos_X, area_pos_Y + outline_spread, area_coords_Right, area_coords_Bottom };
		rect[8] = { area_pos_X, area_pos_Y, area_coords_Right, area_coords_Bottom };
		if (gameWindowWidth > 0) { CorrectSubsAreaSize(); } // If game window Width is available - correct the rectangles

		colors[0] = sHEX_sRGB_toRGBA(outline_color, colors[0], outline_col_A);
		colors[1] = sHEX_sRGB_toRGBA(text_color_female, colors[1]);
		if (diff_color_for_male == 1) 
			colors[2] = sHEX_sRGB_toRGBA(text_color_male, colors[2]);
		else { 
			colors[2] = sHEX_sRGB_toRGBA(text_color_female, colors[1]);
			separateColorMale = false;
		}
	}

	void CorrectSubsAreaSize() {
		for (int i = 0; i < fontLayersCount; i++)
			rect[i].right = rect[i].left + gameWindowWidth;
	}

	void SetSubtitlesColor(int r, int g, int b) {
		colors[8] = D3DCOLOR_RGBA(r, g, b, 255);
	}

	VOID PopSubtitles() {
		if (lines.empty())
			return;
		DWORD now = GetTickCount();
		if (now - lastPopTime > duration) {
			lines.pop_front();
			lastPopTime = lines.empty() ? 0 : now;
		}
	}
}
