#pragma once

#include <vector>

#include <Windows.h>

class ClothFile
{
public:
	ClothFile(std::vector<BYTE> &file) : m_isValid(false) {
		if (file.size() == 92) {
			const char* buffer = (const char*)file.data();
#define R(buffer, type, var) var = *(type*)(buffer); buffer += sizeof(type);
			R(buffer, BYTE, m_gender);
			R(buffer, int, m_slot);
			R(buffer, BYTE, m_shortSkirt);
			R(buffer, BYTE, m_socksId);
			R(buffer, BYTE, m_shoesIndoorId);
			R(buffer, BYTE, m_shoesOutdoorId);
			R(buffer, BYTE, m_isOnePiece);
			R(buffer, BYTE, m_hasUnderwear);
			R(buffer, BYTE, m_hasSkirt);
			R(buffer, COLORREF, m_colorTop1);
			R(buffer, COLORREF, m_colorTop2);
			R(buffer, COLORREF, m_colorTop3);
			R(buffer, COLORREF, m_colorTop4);
			R(buffer, COLORREF, m_colorBottom1);
			R(buffer, COLORREF, m_colorBottom2);
			R(buffer, COLORREF, m_colorUnderwear);
			R(buffer, COLORREF, m_colorSocks);
			R(buffer, COLORREF, m_colorIndoorShoes);
			R(buffer, COLORREF, m_colorOutdoorShoes);
			R(buffer, int, m_skirtTextureId);
			R(buffer, int, m_underwearTextureId);
			R(buffer, int, m_skirtHue);
			R(buffer, int, m_skirtBrightness);
			R(buffer, int, m_underwearHue);
			R(buffer, int, m_underwearBrightness);
			R(buffer, int, m_skirtShadowHue);
			R(buffer, int, m_skirtShadowBrightness);
			R(buffer, int, m_underwearShadowHue);
			R(buffer, int, m_underwearShadowBrightness);
#undef R
			m_isValid = true;
		}
	}

	inline bool IsValid() {
		return m_isValid;
	}

	BYTE m_gender;
	int m_slot;
	BYTE m_shortSkirt;
	BYTE m_socksId;
	BYTE m_shoesIndoorId;
	BYTE m_shoesOutdoorId;
	BYTE m_isOnePiece;
	BYTE m_hasUnderwear;
	BYTE m_hasSkirt;
	COLORREF m_colorTop1;
	COLORREF m_colorTop2;
	COLORREF m_colorTop3;
	COLORREF m_colorTop4;
	COLORREF m_colorBottom1;
	COLORREF m_colorBottom2;
	COLORREF m_colorUnderwear;
	COLORREF m_colorSocks;
	COLORREF m_colorIndoorShoes;
	COLORREF m_colorOutdoorShoes;
	int m_skirtTextureId;
	int m_underwearTextureId;
	int m_skirtHue;
	int m_skirtBrightness;
	int m_underwearHue;
	int m_underwearBrightness;
	int m_skirtShadowHue;
	int m_skirtShadowBrightness;
	int m_underwearShadowHue;
	int m_underwearShadowBrightness;
	bool m_isValid;
};
