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

	enum PathStart {
		NONE = 0, AAPLAY = 1,AAEDIT = 2, OVERRIDE = 4
	};

	OverrideFile();
	//path it relative from editor / play root (version 1) or override path (version 2)
	OverrideFile(const TCHAR* path, PathStart tryPathStarts);
	~OverrideFile();

	inline int GetFileSize() const { return m_fileSize; }
	inline const std::wstring& GetFileName() const { return m_fileName; }
	inline const std::wstring& GetRelPath() const { return m_relPath; }
	inline const std::wstring& GetFilePath() const { return m_fullPath; }
	inline bool IsGood() const { return m_good; }
	inline PathStart GetPathStart() const { return m_pathStart; }

	bool WriteToBuffer(BYTE* buffer) const;
protected:
	bool m_good;
	DWORD m_fileSize;
	PathStart m_pathStart; //0: aaplay root, 1: aaedit, 2: override folder

	std::wstring m_fullPath; //full, absolute path; can be used to open file
	std::wstring m_fileName; //only file name
	std::wstring m_relPath;  //relative path, starting from the override folder
	std::vector<BYTE> m_cache;
};

