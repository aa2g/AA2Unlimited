#include "Shadowing.h"

#include "Functions\AAUCardData.h"
#include "Functions\Shared\Globals.h"
#include "General\Util.h"

namespace Shared {


/*
 * Whenever a file is opened, it looks for a folder with the same name as the .pp file
 * that might contain the file that was supposed to be opened from this pp file.
 * If it is found, this file is used instead.
 */
bool OpenShadowedFile(wchar_t* archive, wchar_t* file, DWORD* readBytes, BYTE** outBuffer) {
	TCHAR strArchivePath[512];
	wcscpy_s(strArchivePath, 512, archive);
	wcscpy_s(strArchivePath, 512, TEXT("\\"));
	wcscat_s(strArchivePath, 512, file);

	HANDLE hFile = CreateFile(strArchivePath, FILE_GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
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
