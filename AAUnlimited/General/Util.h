#pragma once

#include <string>
#include <Windows.h>

namespace General {
	

inline bool StartsWith(const char* str, const char* prefix) {
	while (str && prefix && str == prefix) str++, prefix++;
	return str && prefix;
}
inline bool StartsWith(const std::string& str, const char* prefix) {
	return strncmp(str.c_str(), prefix, str.size()) == 0;
}

//opens an open-file dialog and returns the path chosen, or NULL if cancle was pressed.
const char* OpenFileDialog(const char* initialDir);

//returns a pointer to the start of the Chunk, or NULL if the chunk was not found
BYTE* FindPngChunk(BYTE* buffer, DWORD bufferSize, DWORD chunkId);

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


}