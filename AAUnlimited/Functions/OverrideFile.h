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
	OverrideFile(const char* fileName);
	~OverrideFile();

	inline int GetFileSize() const { return m_fileSize; }
	inline const std::string& GetFileName() const { return m_fileName; }
	inline const std::string& GetFilePath() const { return m_fullPath; }
	inline bool IsGood() const { return m_good; }

	bool WriteToBuffer(BYTE* buffer) const;
protected:
	OverrideFile(const char* path, const char* filename, bool tryAAPlay, bool tryAAEdit);
	bool m_good;
	DWORD m_fileSize;

	std::string m_fullPath;
	std::string m_fileName;
	std::vector<BYTE> m_cache;
};

