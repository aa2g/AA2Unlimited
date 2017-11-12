#include <Windows.h>
#include <stdint.h>
#include "opus.h"


class PPeX {
	size_t Read(char *buf, DWORD len);
	std::wstring PPeX::GetString();
	bool Write(char *buf, size_t len);
	bool PPeX::PutString(std::wstring s);
	HANDLE pipe;
public:;
	bool Connect(const wchar_t *path);
	bool is_connected;
	PPeX();
	bool ArchiveDecompress(const wchar_t* paramArchive, const wchar_t* paramFile, DWORD* readBytes, BYTE** outBuffer);
    std::set<std::wstring> *FList(const wchar_t*);
};

extern PPeX g_PPeX;