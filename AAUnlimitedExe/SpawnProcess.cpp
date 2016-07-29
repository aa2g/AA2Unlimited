#include "SpawnProcess.h"

#include <Windows.h>

#include "Error.h"

bool SpawnProcess(char* path) {
	//we have a command line
	if (GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES && GetLastError() == ERROR_FILE_NOT_FOUND) {
		char errorMsg[512] = { "File does not exist: " };
		strcat_s(errorMsg, 512, path);
		MessageBox(NULL, errorMsg, "Error", MB_ICONERROR);
		return false;
	}
	char* file;
	char fullPath[MAX_PATH];
	GetFullPathName(path, MAX_PATH, fullPath, &file);
	if (file > fullPath) *(file - 1) = '\0';
	STARTUPINFO sInfo = { 0 };
	sInfo.cb = sizeof(sInfo);
	PROCESS_INFORMATION pInfo = { 0 };
	BOOL result = CreateProcess(path, file, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, fullPath, &sInfo, &pInfo);
	if (result == FALSE) {
		Error::SetLastError(GetLastError(), "CreateProcess");
		Error::PrintLastError("Failed to Create Process:");
	}
	CloseHandle(pInfo.hProcess);
	CloseHandle(pInfo.hThread);
	return true;
}