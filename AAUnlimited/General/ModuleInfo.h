#pragma once
#include <Windows.h>
#include <string>

namespace General {


extern HINSTANCE DllInst;

extern DWORD GameBase; //base address of the game module
extern bool IsAAPlay;
extern bool IsAAEdit;
//path to the aa2 folders from the registry, including trailing backslash
extern std::string AAEditPath;
extern std::string AAPlayPath;
extern std::string GameExeName; //name of the exe we are hooked to

bool Initialize();

//subpath should not start with a backslash
inline std::string BuildPlayPath(const char* subpath, const char* file) {
	std::string retVal(AAPlayPath);
	if (subpath != NULL) retVal += subpath;
	if (file != NULL) retVal += file;
	return retVal;
}
inline std::string BuildPlayPath(const char* file) {
	std::string retVal(AAPlayPath);
	if (file != NULL) retVal += file;
	return retVal;
}

//subpath should not start with a backslash
inline std::string BuildEditPath(const char* subpath, const char* file) {
	std::string retVal(AAEditPath);
	if (subpath != NULL) retVal += subpath;
	if (file != NULL) retVal += file;
	return retVal;
}
inline std::string BuildEditPath(const char* file) {
	std::string retVal(AAEditPath);
	if (file != NULL) retVal += file;
	return retVal;
}


}