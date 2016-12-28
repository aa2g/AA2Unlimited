#pragma once
#include <Windows.h>
#include <string>

#include <config.h>

namespace General {


extern HINSTANCE DllInst;

extern DWORD GameBase; //base address of the game module
extern bool IsAAPlay;
extern bool IsAAEdit;
//path to the aa2 folders from the registry, including trailing backslash
extern std::wstring AAEditPath;
extern std::wstring AAPlayPath;
extern std::wstring GameExeName; //name of the exe we are hooked to

bool Initialize();

//subpath should not start with a backslash
inline std::wstring BuildPlayPath(const TCHAR* subpath, const TCHAR* file) {
	std::wstring retVal(AAPlayPath);
	if (subpath != NULL) retVal += subpath;
	if (file != NULL) retVal += file;
	return retVal;
}
inline std::wstring BuildPlayPath(const TCHAR* file) {
	std::wstring retVal(AAPlayPath);
	if (file != NULL) retVal += file;
	return retVal;
}

//subpath should not start with a backslash
inline std::wstring BuildEditPath(const TCHAR* subpath, const TCHAR* file) {
	std::wstring retVal(AAEditPath);
	if (subpath != NULL) retVal += subpath;
	if (file != NULL) retVal += file;
	return retVal;
}
inline std::wstring BuildEditPath(const TCHAR* file) {
	std::wstring retVal(AAEditPath);
	if (file != NULL) retVal += file;
	return retVal;
}

inline std::wstring BuildOverridePath(const TCHAR* subpath, const TCHAR* file) {
	std::wstring retVal(AAEditPath);
	retVal += OVERRIDE_PATH;
	if (subpath != NULL) retVal += subpath;
	if (file != NULL) retVal += file;
	return retVal;
}
inline std::wstring BuildOverridePath(const TCHAR* file) {
	std::wstring retVal(AAEditPath);
	retVal += OVERRIDE_PATH;
	if (file != NULL) retVal += file;
	return retVal;
}

}