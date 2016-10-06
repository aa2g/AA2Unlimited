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

namespace {
	//standard crc 32 (IEEE) polynomal: x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1
	DWORD loc_crc32Poly = 0b1110'1101'1011'1000'1000'0011'0010'0000; //note the implicit 1 at x^32 and the little endian order
																	 //crc byte step table
	DWORD loc_crcTable[256];
	bool loc_dummy = []()->bool {
		//intialize table for every possible byte
		for (int i = 0; i < 256; i++) {
			//the table value. representing the effect of all (up to) 8 xors
			//that the polynomal division would be doing based on this hibyte
			DWORD combinedXors = 0;
			BYTE polySteps = i;
			for (int j = 0; j < 8; j++) {
				//note the 33 bit polynomal. since the first bit will always xor to 0, we handle it
				//seperatly and only save 32 bits of it
				bool currBitSet = polySteps & 1;
				combinedXors >>= 1;
				polySteps >>= 1;
				if (currBitSet) {
					combinedXors ^= loc_crc32Poly;
					polySteps ^= loc_crc32Poly;
				}
			}
			loc_crcTable[i] = combinedXors;
		}
		return true;
	}();
};

DWORD Crc32(BYTE* data,int len,DWORD regInit,bool invertResult) {
	DWORD reg = regInit;
	for (int i = 0; i < len; i++) {
		//optimisation here is that the data bytes are not put at the start of the register
		//and shifted through, but are xored at the indexiation of the table. Note that the register
		//still saves the xors that would be done on the bytes as they are shiftet through the register,
		//except that the actual byte is now only used for the indexiation when it actually has an effect,
		//making it unessesary to actually put it into the register and thus saving the appending 0s for
		//the polynomal division.
		reg = loc_crcTable[(reg ^ *data++) & 0xff] ^ (reg >> 8);
	}
	if (invertResult) reg = ~reg;
	return reg;
}


}