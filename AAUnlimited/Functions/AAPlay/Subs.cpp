#include "StdAfx.h"

namespace Subtitles {

	bool enabled = false;
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
	IUnknown *Font;
	int areaPosPercentsX = 0; // in `% * 100`
	int areaPosPercentsY = 0;
	int outlineSpread = 0;
	int gameWindowWidth = 0;
	int gameWindowHeight = 0;
	int gameWindowMarginY = 0;
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

	void InitSubtitlesParams(const char *font_family, int font_size, int line_height, int show_duration, int max_lines,
		const char *text_color_female, int diff_color_for_male, const char *text_color_male,
		int outline_quality, int outline_spread, const char *outline_color, int outline_col_A,
		int text_align, int area_pos_X, int area_pos_Y)
	{
		enabled = true;
		fontFamily = font_family;
		if (text_align == 1) { subsCentered = DT_CENTER; area_pos_X = 0; }
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
			SetSubsAreaSize(gameWindowWidth, gameWindowHeight, gameWindowMarginY);  // If game window Width is available - Set the rectangles

		colors[0] = General::sHEX_sRGB_toRGBA(outline_color, colors[0], outline_col_A);
		colors[1] = General::sHEX_sRGB_toRGBA(text_color_female, colors[1]);
		if (diff_color_for_male == 1) 
			colors[2] = General::sHEX_sRGB_toRGBA(text_color_male, colors[2]);
		else { 
			colors[2] = General::sHEX_sRGB_toRGBA(text_color_female, colors[1]);
			separateColorMale = false;
		}
	}

	void SetSubsAreaSize(int window_width, int window_height, int margin_Y) {
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

	void Render() {
		if (!enabled || lines.empty())
			return;

		PopSubtitles();

		if (outlineLayersCount != 0 || separateColorMale)
		{
			int line_num = 0;
			for each (const auto line in lines) // for each subs line
			{
				int top_offset = lineHeight * line_num;
				RECT *tempRect;
				for (int i = 0; i < outlineLayersCount; i++) // outline layers
				{
					tempRect = &rect[i];
					tempRect->top = tempRect->top + top_offset;
					tempRect->bottom = tempRect->bottom + top_offset;
					DrawD3D::DrawText(Font, 0, std::get<0>(line).c_str(), -1, tempRect,
						DT_NOCLIP | subsCentered, colors[0]);
					tempRect->top = tempRect->top - top_offset;
					tempRect->bottom = tempRect->bottom - top_offset;
				}
				// Colorized text
				tempRect = &rect[fontLayersCount - 1];
				tempRect->top = tempRect->top + top_offset;
				tempRect->bottom = tempRect->bottom + top_offset;
				DrawD3D::DrawText(Font, 0, std::get<0>(line).c_str(), -1, &rect[fontLayersCount - 1],
					DT_NOCLIP | subsCentered, colors[std::get<1>(line)]);
				tempRect->top = tempRect->top - top_offset;
				tempRect->bottom = tempRect->bottom - top_offset;
				
				line_num++;
			}
		}
		else { // Only Colorized text
			text.clear();
			for each (const auto line in lines)
				text += std::get<0>(line);
			DrawD3D::DrawText(Font, 0, text.c_str(), -1, &rect[fontLayersCount - 1],
				DT_NOCLIP | subsCentered, colors[1]);
		}
	}
}
