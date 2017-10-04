#include "stdafx.h"

using namespace ExtClass;

#define MAGIC "MODCARD5"
namespace Shared {
namespace PNG {

struct Footer {
	DWORD aaublob_delta;	// delta points to aau blob data
	char magic[8];			// magic denoting special treatment of this file
	DWORD illusion_delta;	// this delta points to begin of illusion data
};

// If we're asked about a size of png file, omit the size of aau blob
DWORD __stdcall GetFileSizeHook(HANDLE fh, DWORD *hi) {
	Footer footer;
	DWORD sz = GetFileSize(fh, NULL);

	SetFilePointer(fh, sz - sizeof(Footer), NULL, FILE_BEGIN);
	DWORD got = 0;

	if (!ReadFile(fh, &footer, sizeof(footer), &got, NULL))
		return sz;

	// not ours
	if (memcmp(footer.magic, MAGIC, 8))
		return sz;

	// return size without aau blob
	return sz - footer.aaublob_delta;
}

// This function generates the PNG data of character, right before character metadata are appended to it -
// so this is our best chance to stuff in whatever partains to AAU here. This used both by edit cards and save games.
bool(__stdcall *GetPNGOrig)(DWORD _this, CharacterStruct *chr, BYTE **outbuf, DWORD *outlen);
bool __stdcall GetPNG(DWORD _this, CharacterStruct *chr, BYTE **outbuf, DWORD *outlen) {
	bool stat = GetPNGOrig(_this, chr, outbuf, outlen);
	LUA_EVENT_NORET("save_card", chr, stat, outbuf, outlen, outlen?*outlen:0);
	return stat;
}

void InstallHooks() {
	static DWORD GetFileSize_imp = (DWORD)&GetFileSizeHook;
	DWORD fsize_addr1 = General::GameBase;
	DWORD fsize_addr2 = General::GameBase;
	DWORD get_png = General::GameBase;

	if (General::IsAAEdit) {
		fsize_addr1 += 0x127DE9;
		fsize_addr2 += 0x12727A;
		get_png += 0x12628b;
	}
	else {
		fsize_addr1 += 0x139A31;
		fsize_addr2 += 0x138ECA;
		get_png += 0x137ECB;
	}

	Hook((BYTE*)fsize_addr1, { 0xFF, 0x15, HookControl::ANY_DWORD }, { 0xFF, 0x15, HookControl::ABSOLUTE_DWORD, (DWORD)&GetFileSize_imp }, NULL);
	Hook((BYTE*)fsize_addr2, { 0xFF, 0x15, HookControl::ANY_DWORD }, { 0xFF, 0x15, HookControl::ABSOLUTE_DWORD, (DWORD)&GetFileSize_imp }, NULL);


	Hook((BYTE*)get_png,
	{ 0xE8, HookControl::ANY_DWORD },						//expected values
	{ 0xE8, HookControl::RELATIVE_DWORD, (DWORD)&GetPNG },	//redirect to our function
		(DWORD*)&GetPNGOrig);


}

}
}