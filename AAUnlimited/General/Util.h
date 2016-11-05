#pragma once

#include <string>
#include <Windows.h>
#include <vector>

#include "DirectXStructs.h"

namespace General {
	

inline bool StartsWith(const TCHAR* str, const TCHAR* prefix) {
	while (*str && *prefix && *str == *prefix) str++, prefix++;
	return !*prefix;
}
inline bool StartsWith(const std::wstring& str, const TCHAR* prefix) {
	return wcscmp(str.c_str(), prefix) == 0;
}

inline TCHAR* FindFileInPath(TCHAR* path) {
	TCHAR* lastSlash = path;
	while (*path) {
		TCHAR c = *path;
		if (c == '\\' || c == '/') lastSlash = path+1;
		path++;
	}
	return lastSlash;
}

inline const TCHAR* FindFileInPath(const TCHAR* path) {
	const TCHAR* lastSlash = path;
	while (*path) {
		TCHAR c = *path;
		if (c == '\\' || c == '/') lastSlash = path+1;
		path++;
	}
	return lastSlash;
}

void CreatePathForFile(const TCHAR* name);

inline float GetRandomFloat(float min, float max) {
	return min + rand() / (RAND_MAX / (max - min));
}

//opens an open-file dialog and returns the path chosen, or NULL if cancle was pressed.
const TCHAR* OpenFileDialog(const TCHAR* initialDir);

inline bool DirExists(const TCHAR* dir) {
	DWORD type = GetFileAttributes(dir);
	return (type != INVALID_FILE_ATTRIBUTES && (type & FILE_ATTRIBUTE_DIRECTORY));
}

inline bool FileExists(const TCHAR* file) {
	DWORD type = GetFileAttributes(file);
	return (type != INVALID_FILE_ATTRIBUTES && !(type & FILE_ATTRIBUTE_DIRECTORY));
}

inline int GetEditInt(HWND ed) {
	TCHAR tempbuf[256];
	SendMessage(ed,WM_GETTEXT,256,(LPARAM)tempbuf);
	return _wtoi(tempbuf);
}

inline float GetEditFloat(HWND ed) {
	TCHAR tempbuf[256];
	SendMessage(ed,WM_GETTEXT,256,(LPARAM)tempbuf);
	return (float)_wtof(tempbuf);
}

//returns a pointer to the start of the Chunk, or NULL if the chunk was not found
BYTE* FindPngChunk(BYTE* buffer, DWORD bufferSize, DWORD chunkId);

/*
 * Rect utility functions
 */
inline LONG RectWidth(const RECT& rct) {
	return rct.right - rct.left;
}
inline LONG RectHeight(const RECT& rct) {
	return rct.bottom - rct.top;
}

inline void RectMoveTo(RECT& rct, LONG left, LONG top) {
	LONG height = RectHeight(rct);
	LONG width = RectWidth(rct);
	rct.left = left;
	rct.top = top;
	rct.right = left + width;
	rct.bottom = top + height;
}

inline void RectMoveBy(RECT& rct,LONG x,LONG y) {
	LONG height = RectHeight(rct);
	LONG width = RectWidth(rct);
	rct.left += x;
	rct.top += y;
	rct.right += x;
	rct.bottom += y;
}

inline void MoveWindowRect(HWND wnd, RECT& rct, BOOL redraw) {
	MoveWindow(wnd,rct.left,rct.top,RectWidth(rct),RectHeight(rct),redraw);
}

/*
 * Keeps Track of the Time it was started, and can be told to return the difference, in seconds,
 * that it is "running" allready. No active time measurement is performed, the time is only polled
 * when the currently passed time is requested.
 */
class PassiveTimer {
	static double m_freq;
	LARGE_INTEGER m_startTime;
public:
	inline void Start() {
		QueryPerformanceFrequency(&m_startTime);
		m_freq = (double)m_startTime.QuadPart;
		QueryPerformanceCounter(&m_startTime);
	}

	inline double GetTime() {
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		return (now.QuadPart - m_startTime.QuadPart) / m_freq;
	}
};


DWORD Crc32(BYTE* data,int len,DWORD regInit = 0xFFFFFFFF,bool invertResult = true);

D3DMATRIX MatrixFromSRT(D3DVECTOR3& scales,D3DVECTOR3& rots,D3DVECTOR3& trans);

std::vector<BYTE> FileToBuffer(const TCHAR* path);

}