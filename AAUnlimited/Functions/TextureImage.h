#pragma once

#include <Windows.h>
#include <string>

#include "OverrideFile.h"

#pragma pack(push, 1)
struct TgaHeader {
	BYTE idSize;
	BYTE palletType;
	BYTE imageType;
	WORD palletStart;
	WORD palletEnd;
	BYTE palletEntrySize;
	WORD x0;
	WORD y0;
	WORD width;
	WORD height;
	BYTE bpp;
	BYTE attributes;
};
#pragma pack(pop)

/*
 * Represents a texture file. Holds dimensions and file size. used for texture overrides.
 */
class TextureImage : public OverrideFile
{
public:
	TextureImage();
	TextureImage(const TCHAR* path, PathStart tryPathStarts);
	~TextureImage();

	inline int GetWidth() const { return m_width; }
	inline int GetHeight() const { return m_height; }

private:
	int m_width;
	int m_height;

	enum ImageType {
		UNKNOWN, TGA, BMP
	};
	ImageType m_type;

private:
	void FromTga(HANDLE handle);
	void FromBmp(HANDLE handle);
};

