#pragma once
#include <Windows.h>
#include <string>

#include "defs.h"

#include <codecvt>

namespace General {


extern std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8;
const char *to_utf8(const std::wstring &ws);


extern HINSTANCE DllInst;

extern DWORD GameBase; //base address of the game module
extern bool IsAAPlay;
extern bool IsAAEdit;
//path to the aa2 folders from the registry, including trailing backslash
extern std::wstring AAEditPath;
extern std::wstring AAPlayPath;
extern std::wstring AAUPath;
extern std::wstring GameExeName; //name of the exe we are hooked to

bool InitializeExeType();
bool EarlyInit();
bool InitializePaths();

// CAVEAT: The returned strings are static, so that extracted c_strs() can be relied on
// at least until next call of the same function.

inline std::wstring BuildAAUPath(const TCHAR* file) {
	std::wstring retVal;
	retVal = AAUPath;
	if (file != NULL) retVal += file;
	return retVal;
}

//subpath should not start with a backslash
inline std::wstring BuildPlayPath(const TCHAR* subpath, const TCHAR* file) {
	std::wstring retVal;
	retVal = AAPlayPath;
	if (subpath != NULL) retVal += subpath;
	if (file != NULL) retVal += file;
	return retVal;
}
inline std::wstring BuildPlayPath(const TCHAR* file) {
	std::wstring retVal;
	retVal = AAPlayPath;
	if (file != NULL) retVal += file;
	return retVal;
}

//subpath should not start with a backslash
inline std::wstring BuildEditPath(const TCHAR* subpath, const TCHAR* file) {
	std::wstring retVal;
	retVal = AAEditPath;
	if (subpath != NULL) retVal += subpath;
	if (file != NULL) retVal += file;
	return retVal;
}
inline std::wstring BuildEditPath(const TCHAR* file) {
	std::wstring retVal;
	retVal = AAEditPath;
	if (file != NULL) retVal += file;
	return retVal;
}

inline std::wstring BuildOverridePath(const TCHAR* subpath, const TCHAR* file) {
	std::wstring retVal;
	retVal = AAEditPath;
	retVal += OVERRIDE_PATH;
	if (subpath != NULL) retVal += subpath;
	if (file != NULL) retVal += file;
	return retVal;
}
inline std::wstring BuildOverridePath(const TCHAR* file) {
	std::wstring retVal;
	retVal = AAEditPath;
	retVal += OVERRIDE_PATH;
	if (file != NULL) retVal += file;
	return retVal;
}

}
