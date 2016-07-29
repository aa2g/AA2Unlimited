#include "PathInfo.h"

#include <Windows.h>

std::string g_AAPlayPath;
std::string g_AAEditPath;

bool GetPathsFromRegistry() {
	//all hail copy n paste
	BYTE buffer[512];
	DWORD keyType;
	DWORD outSize;
	LONG res;

	//aaplay path
	HKEY playKey;

	res = RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\illusion\\AA2Play"), 0, KEY_READ, &playKey);
	if (res != ERROR_SUCCESS) {
		MessageBox(NULL, TEXT("Could not find AAPlay path in registry"), TEXT("Critical Error"), 0);
		return false;
	}
	outSize = sizeof(buffer);
	RegQueryValueEx(playKey, TEXT("INSTALLDIR"), NULL, &keyType, buffer, &outSize);
	if (keyType != REG_SZ || outSize < 2) {
		MessageBox(NULL, TEXT("Could not find AAPlay INSTALLDIR key in registry"), TEXT("Critical Error"), 0);
		return false;
	}
	if (buffer[outSize - 1] != '\0') { buffer[outSize] = '\0'; outSize++; }
	if (buffer[outSize - 2] != '\\') {
		buffer[outSize - 1] = '\\';
		buffer[outSize] = '\0';
	}
	g_AAPlayPath = std::string((char*)buffer);

	//aaedit path
	HKEY editKey;

	res = RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\illusion\\AA2Edit"), 0, KEY_READ, &editKey);
	if (res != ERROR_SUCCESS) {
		MessageBox(NULL, TEXT("Could not find AAEdit path in registry"), TEXT("Critical Error"), 0);
		return false;
	}
	outSize = sizeof(buffer);
	RegQueryValueEx(editKey, TEXT("INSTALLDIR"), NULL, &keyType, buffer, &outSize);
	if (keyType != REG_SZ || outSize < 2) {
		MessageBox(NULL, TEXT("Could not find AAEdit INSTALLDIR key in registry"), TEXT("Critical Error"), 0);
		return false;
	}
	if (buffer[outSize - 1] != '\0') { buffer[outSize] = '\0'; outSize++; }
	if (buffer[outSize - 2] != '\\') {
		buffer[outSize - 1] = '\0';
		buffer[outSize] = '\0';
	}
	g_AAEditPath = std::string((char*)buffer);

	return true;

}

std::vector<std::string> GetPossiblePlayExeList() {
	int it = 0;
	int size = 64;
	std::vector<std::string> list;

	std::string filter;
	filter = g_AAPlayPath + "*.exe";
	WIN32_FIND_DATA data;
	HANDLE hSearch = FindFirstFile(filter.c_str(), &data);

	if (hSearch == INVALID_HANDLE_VALUE) return list;

	BOOL suc = FALSE;
	do {
		list.push_back(data.cFileName);
		suc = FindNextFile(hSearch, &data);
	} while (suc != FALSE);
	FindClose(hSearch);
	return list;
}

std::vector<std::string> GetPossibleEditExeList() {
	int it = 0;
	int size = 64;
	std::vector<std::string> list;

	std::string filter;
	filter = g_AAEditPath + "*.exe";
	WIN32_FIND_DATA data;
	HANDLE hSearch = FindFirstFile(filter.c_str(), &data);

	if (hSearch == INVALID_HANDLE_VALUE) return list;

	BOOL suc = FALSE;
	do {
		list.push_back(data.cFileName);
		suc = FindNextFile(hSearch, &data);
	} while (suc != FALSE);
	FindClose(hSearch);
	return list;
}