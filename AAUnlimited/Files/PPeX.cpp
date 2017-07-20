#include "Functions/Shared/Globals.h"
#include "PPeX.h"
#include <stdint.h>
#include <fstream>
#include <iostream>
#include <io.h>
#include <algorithm>
#include <cctype>
#include "Files/Logger.h"
#include "Files/Config.h"
#include <assert.h>
#include "zstd.h"

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
	return true;
}

size_t PPeX::Read(char *buf, DWORD len) {
	DWORD got = 0;
	LOGPRIONC(Logger::Priority::SPAM) "reading from pipe " << int(len) << " bytes\r\n";
	while (got < len) {
		DWORD get = len - got;
		if (!ReadFile(pipe, buf + got, len, &get, NULL)) {
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
	buf[0] &= 3;
	size_t len = ((((size_t)buf[0]) << 8) | buf[1]);
	LOGPRIONC(Logger::Priority::SPAM) "Got string byte length " << int(len) << "\r\n";
	PPeX::Read((char*)buf, len);
	auto res = wstring((wchar_t*)buf, len/2);
	LOGPRIONC(Logger::Priority::SPAM) "Received string " << res << "\r\n";
	return res;
}

bool PPeX::Write(char *buf, size_t len) {
	LOGPRIONC(Logger::Priority::SPAM) "Sending " << int(len) << " bytes\r\n";

	DWORD got = 0;
	while (got < len) {
		DWORD get = len - got;
		if (!WriteFile(pipe, buf + got, len, &get, NULL)) {
			LOGPRIO(Logger::Priority::CRIT_ERR) << "Pipe write failed " << GetLastError() << "\r\n";
			return false;
		}
		got += get;
	}
	return true;
}

bool PPeX::PutString(wstring s) {
	LOGPRIONC(Logger::Priority::SPAM) "Sending string " << s << "\r\n";

	int len = s.size() * 2;
	uint8_t buf[2] = { (uint8_t) (len >> 8), (uint8_t)len };
	return Write((char*)buf, 2) && Write((char*)s.c_str(), len);
}

bool PPeX::ArchiveDecompress(const wchar_t* paramArchive, const wchar_t* paramFile, DWORD* readBytes, BYTE** outBuffer) {
	LOGPRIO(Logger::Priority::SPAM) << "Request " << wstring(paramArchive) << "/" << wstring(paramFile) <<"\r\n";

	wchar_t *p = wcsrchr((wchar_t*)paramArchive, L'\\');
	if (p) paramArchive = p + 1;
	wstring path = (wstring(paramArchive, wcslen(paramArchive) - 3) + L"/" + paramFile);
	transform(path.begin(), path.end(), path.begin(), ::tolower);

	PutString(L"load");
	PutString(path);

	auto slen = GetString();
	if (slen == L"" || slen == L"NotAvailable")
		return false;

	size_t len = stoi(slen);
	auto ubuf = Shared::IllusionMemAlloc(len);
	if (!ubuf)
		return false;
	*outBuffer = (BYTE*)ubuf;
	*readBytes = len;
	return Read((char*)ubuf, len) == len;
}

