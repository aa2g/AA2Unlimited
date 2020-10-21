#include "StdAfx.h"


using namespace std;

PPeX g_PPeX;

PPeX::PPeX() {
}

bool PPeX::Connect(const wchar_t *path) {
	while (1) {
		pipe = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (pipe != INVALID_HANDLE_VALUE)
			break;
		LOGPRIO(Logger::Priority::ERR) << "Failed to connect " << wstring(path) << "\r\n";
		Sleep(1000);
	}
	while (1) {
		PutString(L"ready");
		PutString(L"");
		auto status = GetString();
		if (status == L"True")
			break;
		Sleep(500);
		LOGPRIO(Logger::Priority::ERR) << "Waiting for server ready, status: " << wstring(status) << "\r\n";
	}
	is_connected = true;
	return true;
}

static bool is_pp_path(const wchar_t *path) {
    int pplen = wcslen(path);
    if (pplen < 5)
        return false;
    return !wcscmp(path + pplen - 4, L"*.pp");
}

static set<wstring> file_list;

set<wstring> *PPeX::FList(const wchar_t *path) {
	if (!is_connected)
		return NULL;
	if (!is_pp_path(path))
		return NULL;

	file_list.clear();

	PutString(L"matchfiles");
	PutString(path);
	while (1) {
		auto str = GetString();
		if (str == L"")
		    break;
		file_list.insert(str);
	}
	if (file_list.empty())
		return NULL;
	return &file_list;
}

size_t PPeX::Read(char *buf, DWORD len) {
	DWORD got = 0;
	while (got < len) {
		DWORD get = 0;
		if (!ReadFile(pipe, buf + got, len - got, &get, NULL)) {
			LOGPRIO(Logger::Priority::CRIT_ERR) << "Pipe read failed " << GetLastError() << "\r\n";
			return 0;
		}
		got += get;
	}
	return (size_t)got;
}

wstring PPeX::GetString() {
	uint8_t buf[1024];
	PPeX::Read((char*)buf, 2);
	size_t len = ((((size_t)buf[0]) << 8) | buf[1]);
	len &= 1023;
	PPeX::Read((char*)buf, len);
	auto res = wstring((wchar_t*)buf, len/2);
	return res;
}

bool PPeX::Write(char *buf, size_t len) {
	DWORD got = 0;
	while (got < len) {
		DWORD get = 0;
		if (!WriteFile(pipe, buf + got, len - got, &get, NULL)) {
			LOGPRIO(Logger::Priority::CRIT_ERR) << "Pipe write failed " << GetLastError() << "\r\n";
			return false;
		}
		got += get;
	}
	return true;
}

bool PPeX::PutString(wstring s) {
	int len = s.size() * 2;
	uint8_t buf[2] = { (uint8_t) (len >> 8), (uint8_t)len };
	return Write((char*)buf, 2) && Write((char*)s.c_str(), len);
}

bool PPeX::ArchiveDecompress(const wchar_t* paramArchive, const wchar_t* paramFile, DWORD* readBytes, BYTE** outBuffer) {
	LOGPRIO(Logger::Priority::SPAM) << "Request " << wstring(paramArchive) << "/" << wstring(paramFile) << "\r\n";

	wchar_t *p = wcsrchr((wchar_t*)paramArchive, L'\\');
	if (p) paramArchive = p + 1;
	int parchlen = wcslen(paramArchive);
	if (parchlen < 3)
		return false;
	if (wcscmp(paramArchive + parchlen - 3, L".pp"))
		return false;
	wstring path = (wstring(paramArchive, parchlen - 3) + L"/" + paramFile);
	transform(path.begin(), path.end(), path.begin(), ::tolower);

	PutString(L"load");
	PutString(path);

	auto slen = GetString();
	if (slen == L"NotAvailable")
		return false;

	int len = stoi(slen);
	auto ubuf = Shared::IllusionMemAlloc(len);
	if (!ubuf)
		return false;
	*outBuffer = (BYTE*)ubuf;
	*readBytes = len;
	return Read((char*)ubuf, len) == len;
}

