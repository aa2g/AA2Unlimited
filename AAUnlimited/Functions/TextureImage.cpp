#include "TextureImage.h"

#include <Windows.h>
#include <string>

#include "General\ModuleInfo.h"
#include "Files\Logger.h"
#include "config.h"


TextureImage::TextureImage() {

}

TextureImage::TextureImage(const TCHAR* path,PathStart tryPathStarts)
	: OverrideFile(path,tryPathStarts) {
	m_good = false; //we need additional things
	//find out if its tga or bmp
	int length = m_fileName.size();
	const TCHAR* fileName = m_fileName.c_str();
	if (length > 4) {
		if (wcscmp(fileName+length - 4, TEXT(".bmp")) == 0) {
			m_type = BMP;
		}
		else if (wcscmp(fileName+length - 4, TEXT(".tga")) == 0) {
			m_type = TGA;
		}
	}
	if (m_type == UNKNOWN) {
		//unknown file format
		return;
	}

	HANDLE file = CreateFile(m_fullPath.c_str(), FILE_READ_ACCESS, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (file == INVALID_HANDLE_VALUE || file == NULL) {
		//could not open file
		return;
	}

	if (m_type == BMP) FromBmp(file);
	else if (m_type == TGA) FromTga(file);

	CloseHandle(file);

}


TextureImage::~TextureImage()
{
	
}

void TextureImage::FromTga(HANDLE handle)
{
	DWORD read;
	TgaHeader tgaHeader;
	ReadFile(handle, &tgaHeader, sizeof(tgaHeader), &read, 0);
	m_height = tgaHeader.height;
	m_width = tgaHeader.width;
	m_good = true;
}

void TextureImage::FromBmp(HANDLE handle)
{
	BYTE buffer[54] = { 0 };
	DWORD read;
	BITMAPINFOHEADER bmpInfo;
	BITMAPFILEHEADER bmpFile;
	ReadFile(handle, buffer, 54, &read, NULL);
	memcpy(&bmpFile, buffer, 14);
	memcpy(&bmpInfo, buffer + 14, 40);

	//do sanity checks
	if (bmpFile.bfType != 0x4D42 //BM
		|| bmpFile.bfSize != m_fileSize)
	{
		return;
	}

	m_width = bmpInfo.biWidth;
	m_height = abs(bmpInfo.biHeight);
	m_good = true;
}
