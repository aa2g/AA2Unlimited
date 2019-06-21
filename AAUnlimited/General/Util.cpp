#include "StdAfx.h"





namespace General {


double PassiveTimer::m_freq;

void CreatePathForFile(const TCHAR* name) {
	
	int len = wcslen(name);
	TCHAR* nameCpy = new TCHAR[len+1];
	wcscpy_s(nameCpy,len+1,name);
	
	for (int i = 0; nameCpy[i]; i++) {
		if(nameCpy[i] == '\\' || nameCpy[i] == '/') {
			TCHAR tmp = nameCpy[i];
			nameCpy[i] = '\0';
			if(!DirExists(nameCpy)) {
				CreateDirectory(nameCpy,NULL);
			}
			nameCpy[i] = tmp;
		}
	}
	delete[] nameCpy;
}

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
const TCHAR* SaveFileDialog(const TCHAR* initialDir) {
	static OPENFILENAME opfn;
	static bool opfnInit = false;
	static TCHAR workingDir[512];
	static TCHAR path[512];
	if (!opfnInit) {
		ZeroMemory((void*)(&opfn),sizeof(opfn));
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
	GetCurrentDirectory(500,workingDir);
	BOOL ret = GetSaveFileName(&opfn); //changes the working dir cause it likes to troll ppl
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

void ScrollWindow(HWND wnd,WPARAM scrollType,DWORD scrollKind) {
	SCROLLINFO si;
	// Get all the vertial scroll bar information.
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	GetScrollInfo(wnd,scrollKind,&si);

	// Save the position for comparison later on.
	int oldPos = si.nPos;
	switch (LOWORD(scrollType))
	{
	case SB_TOP:
		si.nPos = si.nMin;
		break;
	case SB_BOTTOM:
		si.nPos = si.nMax;
		break;
	case SB_LINEUP:
		si.nPos -= 1;
		break;
	case SB_LINEDOWN:
		si.nPos += 1;
		break;
	case SB_PAGEUP:
		si.nPos -= si.nPage;
		break;
	case SB_PAGEDOWN:
		si.nPos += si.nPage;
		break;
	case SB_THUMBTRACK:
		si.nPos = si.nTrackPos;
		break;
	default:
		break;
	}

	// Set the position and then retrieve it.  Due to adjustments
	// by Windows it may not be the same as the value set.
	si.fMask = SIF_POS;
	SetScrollInfo(wnd,scrollKind,&si,TRUE);
	GetScrollInfo(wnd,scrollKind,&si);

	// If the position has changed, scroll window and update it.
	if (si.nPos != oldPos)
	{
		ScrollWindow(wnd,0,(oldPos - si.nPos),NULL,NULL);
		UpdateWindow(wnd);
	}
}

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

D3DMATRIX MatrixFromSRT(D3DXVECTOR3 & scales,D3DXVECTOR3 & rots,D3DXVECTOR3 & trans)
{
	D3DMATRIX matr;

	D3DMATRIX matrScale = { scales.x,0,0,0,
		0,scales.y,0,0,
		0,0,scales.z,0,
		0,0,0,1.0f };
	D3DMATRIX matrRot;
	(*Shared::D3DXMatrixRotationYawPitchRoll)(&matrRot,rots.y,rots.x,rots.z);
	D3DMATRIX matrTrans = { 1.0f,0,0,0,
		0,1.0f,0,0,
		0,0,1.0f,0,
		trans.x,trans.y,trans.z,1.0f };
	(*Shared::D3DXMatrixMultiply)(&matr,&matrScale,&matrRot);
	(*Shared::D3DXMatrixMultiply)(&matr,&matr,&matrTrans);
	return matr;
}


std::vector<BYTE> FileToBuffer(const TCHAR* path) {
	std::vector<BYTE> retval;

	HANDLE hFile = CreateFile(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) return retval;
	DWORD hi;
	DWORD size = GetFileSize(hFile,&hi);
	
	retval.resize(size);
	ReadFile(hFile,retval.data(),size,&hi,NULL);

	CloseHandle(hFile);
	return retval;
}


static int HexadecimalToDecimal(std::string hex) {
	int hexLength = hex.length();
	double dec = 0;

	for (int i = 0; i < hexLength; ++i)
	{
		char b = hex[i];

		if (b >= 48 && b <= 57)
			b -= 48;
		else if (b >= 65 && b <= 70)
			b -= 55;
		dec += b * pow(16, ((hexLength - i) - 1));
	}
	return (int)dec;
}


D3DCOLOR sHEX_sRGB_toRGBA(std::string stringHEX_RGB, D3DCOLOR colorDefault, int alphaChannel) {
	D3DCOLOR result = colorDefault;
	int R = 0;
	int G = 0;
	int B = 0;
	std::locale loc; std::string afterStrToUpper = "";
	for (std::string::size_type i = 0; i<stringHEX_RGB.length(); ++i)
		afterStrToUpper += std::toupper(stringHEX_RGB[i], loc);
	stringHEX_RGB = afterStrToUpper;

	std::smatch matches;
	std::regex regExpHEX("([A-F0-9]{6})");
	std::regex regExpRGB("(\\d{1,3})[^\\d]+(\\d{1,3})[^\\d]+(\\d{1,3})");
	std::regex regExpShortHEX("([A-F0-9]{3})");
	bool finded_color = false;
	if (std::regex_match(stringHEX_RGB, matches, regExpHEX))		// Try to find HEX
	{
		R = (unsigned char)HexadecimalToDecimal(matches[0].str().substr(0, 2));
		G = (unsigned char)HexadecimalToDecimal(matches[0].str().substr(2, 2));
		B = (unsigned char)HexadecimalToDecimal(matches[0].str().substr(4, 2));
		finded_color = true;
	}
	else if (std::regex_match(stringHEX_RGB, matches, regExpRGB)) { // Try to find RGB
		R = std::stoi(matches[1].str());
		G = std::stoi(matches[2].str());
		B = std::stoi(matches[3].str());
		finded_color = true;
	}
	else if (std::regex_match(stringHEX_RGB, matches, regExpShortHEX)) { // Try to find short HEX
		R = (unsigned char)HexadecimalToDecimal(matches[0].str().substr(0, 1) + matches[0].str().substr(0, 1));
		G = (unsigned char)HexadecimalToDecimal(matches[0].str().substr(1, 1) + matches[0].str().substr(1, 1));
		B = (unsigned char)HexadecimalToDecimal(matches[0].str().substr(2, 1) + matches[0].str().substr(2, 1));
		finded_color = true;
	}

	if (finded_color) {
		if (R > 255) { R = 255; }
		if (G > 255) { G = 255; }
		if (B > 255) { B = 255; }
		result = D3DCOLOR_RGBA(R, G, B, alphaChannel);
	}

	return result;
}


}