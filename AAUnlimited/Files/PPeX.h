#include <Windows.h>
#include <stdint.h>
#include "opus.h"

class PPeX {
	struct fileEntry {
		uint32_t hid; // index to handles[]
		uint32_t flags;
		
		uint32_t csize;
		uint32_t usize;
		
		uint64_t offset;
	};
	OpusDecoder *decoder[2];
	std::vector<HANDLE> handles;
	std::map<std::wstring, fileEntry> files;
public:;
	PPeX();
	void AddArchive(const wchar_t *fn);
	void AddPath(const std::wstring &path);
	bool ArchiveDecompress(wchar_t* paramArchive, wchar_t* paramFile, DWORD* readBytes, BYTE** outBuffer);
};

extern PPeX g_PPeX;