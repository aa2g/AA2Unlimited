#include "TextureImage.h"

#include <Windows.h>
#include <string>

#include "General\ModuleInfo.h"
#include "Files\Logger.h"
#include "config.h"


TextureImage::TextureImage(const char* fileName) : m_type(UNKNOWN), m_good(false), m_cache(NULL)
{
	//find out if its tga or bmp
	int length = strlen(fileName);
	if (length > 4) {
		if (strcmp(fileName+length - 4, ".bmp") == 0) {
			m_type = BMP;
		}
		else if (strcmp(fileName+length - 4, ".tga") == 0) {
			m_type = TGA;
		}
	}
	if (m_type == UNKNOWN) {
		//unknown file format
		return;
	}

	m_fileName = fileName;
	m_fullPath = General::BuildEditPath(OVERRIDE_IMAGE_PATH, fileName);

	HANDLE file = CreateFile(m_fullPath.c_str(), FILE_READ_ACCESS, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (file == INVALID_HANDLE_VALUE || file == NULL) {
		//could not open file
		return;
	}

	DWORD hi;
	m_fileSize = ::GetFileSize(file, &hi);

	if (m_type == BMP) FromBmp(file);
	else if (m_type == TGA) FromTga(file);

	CloseHandle(file);

}

TextureImage::TextureImage(const TextureImage& copy) : m_type(UNKNOWN), m_good(false), m_cache(NULL) {
	m_good = copy.m_good;
	m_width = copy.m_width;
	m_height = copy.m_height;
	m_fileSize = copy.m_fileSize;
	m_type = copy.m_type;

	m_fileName = copy.m_fileName;
	m_fullPath = copy.m_fullPath;
	if (copy.m_cache == NULL) m_cache = NULL;
	else {
		m_cache = new BYTE[m_fileSize];
		memcpy(m_cache, copy.m_cache, m_fileSize);
	}

}

TextureImage::TextureImage(TextureImage&& moveCopy) {
	m_good = moveCopy.m_good;
	m_width = moveCopy.m_width;
	m_height = moveCopy.m_height;
	m_fileSize = moveCopy.m_fileSize;
	m_type = moveCopy.m_type;

	m_fileName = std::move(moveCopy.m_fileName);
	m_fullPath = std::move(moveCopy.m_fullPath);
	
	m_cache = moveCopy.m_cache;
	moveCopy.m_cache = NULL;
}


TextureImage::~TextureImage()
{
	delete[] m_cache;
}

bool TextureImage::WriteToBuffer(BYTE * buffer) const
{
	if (m_cache) {
		//if cached, copy the cache
		memcpy(m_cache, buffer, m_fileSize);
		return true;
	}
	else {
		//we will have to read if from the file
		HANDLE file = CreateFile(m_fullPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		int fileError = GetLastError();
		DWORD written;
		ReadFile(file, buffer, m_fileSize, &written, NULL);
		int readError = GetLastError();
		CloseHandle(file);
		if (written != m_fileSize) {
			LOGPRIO(Logger::Priority::WARN) << "failed to read from file " << m_fullPath << ": "
				"Open Error " << fileError << ", readError " << readError << "\r\n";
			return false;
		}
		return true;
	}
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
