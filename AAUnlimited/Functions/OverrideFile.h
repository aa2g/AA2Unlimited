#pragma once

#include <Windows.h>
#include <string>
#include <vector>

/*
 * Generic file.
 */
class OverrideFile
{
public:
	OverrideFile(const TCHAR* fileName);
	~OverrideFile();

	inline int GetFileSize() const { return m_fileSize; }
	inline const std::wstring& GetFileName() const { return m_fileName; }
	inline const std::wstring& GetFilePath() const { return m_fullPath; }
	inline bool IsGood() const { return m_good; }

	bool WriteToBuffer(BYTE* buffer) const;
protected:
	OverrideFile(const TCHAR* path, const TCHAR* filename, bool tryAAPlay, bool tryAAEdit);
	bool m_good;
	DWORD m_fileSize;

	std::wstring m_fullPath;
	std::wstring m_fileName;
	std::vector<BYTE> m_cache;
};

