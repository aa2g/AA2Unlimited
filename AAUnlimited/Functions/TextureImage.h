#pragma once

#include <Windows.h>
#include <string>

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
class TextureImage
{
public:
	TextureImage(const char* fileName);
	TextureImage(const TextureImage& copy);
	TextureImage(TextureImage&& moveCopy);
	~TextureImage();

	inline int GetFileSize() const { return m_fileSize; }
	inline int GetWidth() const { return m_width; }
	inline int GetHeight() const { return m_height; }
	inline const std::string& GetFileName() const { return m_fileName; }
	inline const std::string& GetFilePath() const { return m_fullPath; }
	inline bool IsGood() const { return m_good; }
	
	//dumps file to buffer. buffer is expected to be big enough
	bool WriteToBuffer(BYTE* buffer) const; 

private:
	bool m_good; //if the file actually exists and is accessable
	int m_width;
	int m_height;
	int m_fileSize;
	std::string m_fullPath;
	std::string m_fileName;
	enum ImageType {
		UNKNOWN, TGA, BMP
	};
	ImageType m_type;
	BYTE* m_cache;

private:
	void FromTga(HANDLE handle);
	void FromBmp(HANDLE handle);
};

