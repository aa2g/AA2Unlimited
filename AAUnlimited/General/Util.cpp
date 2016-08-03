#include "Util.h"

#include <Windows.h>


namespace General {


double PassiveTimer::m_freq;

const TCHAR* OpenFileDialog(const TCHAR* initialDir) {
	static OPENFILENAME opfn;
	static bool opfnInit = false;
	static TCHAR workingDir[512];
	static TCHAR path[512];
	if (!opfnInit) {
		ZeroMemory((void*)(&opfn), sizeof(opfn));
		opfn.lStructSize = sizeof(opfn);
		opfn.hwndOwner = NULL;
		opfn.nFilterIndex = 1;
		opfn.lpstrFileTitle = NULL;
		opfn.nMaxFileTitle = 0;
		opfn.lpstrInitialDir = NULL;
		opfn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		opfn.lpstrFilter = TEXT("All\0*\0");
	}
	opfn.lpstrFile = path;
	opfn.lpstrFile[0] = '\0';
	opfn.nMaxFile = 512;
	if (initialDir != NULL) {
		opfn.lpstrInitialDir = initialDir;
	}
	else {
		opfn.lpstrInitialDir = NULL;
	}
	GetCurrentDirectory(500, workingDir);
	BOOL ret = GetOpenFileName(&opfn); //changes the working dir cause it likes to troll ppl
	SetCurrentDirectory(workingDir);
	if (ret == FALSE) {
		return NULL;
	}
	return path;
}
BYTE* FindPngChunk(BYTE* buffer, DWORD bufferSize, DWORD targetChunk) {
	if (bufferSize < 12) return NULL;
	DWORD chunkLength = 0, chunkId = 0;
	for (DWORD i = 8; i < bufferSize+12; i += 12) { //8 to skip header, 12 for each chunk
		if (i > bufferSize - 12)  return NULL; //not even a chunk left. must be incorrect buffer
		chunkLength = _byteswap_ulong(*(DWORD*)(buffer + i));
		chunkId = *(DWORD*)(buffer + i + 4);
		if (chunkId == targetChunk) {
			//found
			return buffer + i;
		}
		else if (chunkId == *(const DWORD*)("IEND")) {
			break; //not found
		}
		i += chunkLength; //skip chunkData as well
	}
	return NULL;
}


}