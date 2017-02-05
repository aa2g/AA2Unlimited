#include "Shadowing.h"

#include "Functions\AAUCardData.h"
#include "Functions\Shared\Globals.h"
#include "General\Util.h"

#include "Shlwapi.h"
#include "Strsafe.h"

#pragma comment(lib, "Shlwapi.lib")

namespace Shared {


/*
 * Whenever a file is opened, it looks for a folder with the same name as the .pp file
 * that might contain the file that was supposed to be opened from this pp file.
 * If it is found, this file is used instead.
 */
bool OpenShadowedFile(wchar_t* archive, wchar_t* file, DWORD* readBytes, BYTE** outBuffer) {
	TCHAR strArchivePath[512];
	int length = wcslen(archive);
	if (length < 3) return false;
	wcscpy_s(strArchivePath, 512, archive);
	wcscpy_s(strArchivePath+length-3, 512-length+3, TEXT("\\"));
	wcscpy_s(strArchivePath+length-2, 512-length+2, file);

	HANDLE hFile = CreateFile(strArchivePath, FILE_GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) {
		TCHAR strArchiveName[512];
		WIN32_FIND_DATA ffd;
		HANDLE hDirectory = INVALID_HANDLE_VALUE;

		strArchivePath[length - 3] = 0;

		wcscpy_s(strArchiveName, 512, strArchivePath);
		PathStripPath(strArchiveName);

		// Remove extension (-3), Archive File Name, trailing slash (-1); Add "\sets\ (+6)"
		length = length - wcslen(strArchiveName) + 2;

		wcscpy_s(strArchivePath + length - 6, 512 - length + 6, TEXT("\\sets\\!*"));

		hDirectory = FindFirstFileEx(strArchivePath, FindExInfoBasic, &ffd, FindExSearchLimitToDirectories, NULL, 0);
		if (hDirectory != INVALID_HANDLE_VALUE) {
			do {
				StringCbPrintf(strArchivePath + length, 512 - length, TEXT("%s\\%s\\%s"), ffd.cFileName, strArchiveName, file);
				hFile = CreateFile(strArchivePath, FILE_GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
				if (hFile != NULL && hFile != INVALID_HANDLE_VALUE)
					break;
			} while (FindNextFile(hDirectory, &ffd) != 0);
		}
		FindClose(hDirectory);
	}
	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) {
		return false;
	}
	DWORD lo, hi;
	lo = GetFileSize(hFile, &hi);
	void* fileBuffer = Shared::IllusionMemAlloc(lo);
	ReadFile(hFile, fileBuffer, lo, &hi, NULL);
	CloseHandle(hFile);
	*outBuffer = (BYTE*)fileBuffer;
	*readBytes = hi;
	return true;
}



}
