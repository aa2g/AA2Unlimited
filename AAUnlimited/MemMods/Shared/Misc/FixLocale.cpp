#include "StdAfx.h"


namespace SharedInjections {
namespace FixLocale {

int IsEmulated() {
	return GetACP() == 932;
}

void SetCP() {
	SetThreadLocale(1041);
	SetThreadUILanguage(1041);
	SetThreadPreferredUILanguages(MUI_LANGUAGE_NAME, L"ja-JP\0", NULL);
}

// AA2Play specific
// XXX TODO?: Move these upper in MemMods if there are more users
static void patchmem(DWORD iat, BYTE *val, int len)
{
	iat -= 0x00400000;
	BYTE *p = (BYTE *)(iat + General::GameBase);
	Memrights unprotect(p, len);
	memcpy(p, val, len);
}

static void patch1(DWORD p, BYTE val)
{
	patchmem(p, &val, 1);
}

static DWORD patch_iat(DWORD niat, DWORD fun)
{
	DWORD iat = (niat - 0x00400000);
	DWORD *p = (DWORD *)(iat + General::GameBase);
	DWORD orig = *p;
	patchmem(niat, (BYTE*)&fun, 4);
	return orig;
}

static int(WINAPI *orig_wcmb)(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e, DWORD f, DWORD g, DWORD h);
static int WINAPI my_wcmb(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e, DWORD f, DWORD g, DWORD h)
{
	a = 932;
	return orig_wcmb(a, b, c, d, e, f, g, h);
}

static int(WINAPI *orig_mbwc)(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e, DWORD f);
static int WINAPI my_mbwc(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e, DWORD f)
{
	a = 932;
	return orig_mbwc(a, b, c, d, e, f);
}

void PatchAA2Play() {
	// Fonts
	patch1(0x005BEBE3, SHIFTJIS_CHARSET);
	patch1(0x005C208C, SHIFTJIS_CHARSET);
	patch1(0x005C39A1, SHIFTJIS_CHARSET);
	patch1(0x005AEA80, SHIFTJIS_CHARSET);

	// multibyte -> sjis and back
	orig_wcmb = (decltype(orig_wcmb))patch_iat(0x006E318C, (DWORD)&my_wcmb);
	orig_mbwc = (decltype(orig_mbwc))patch_iat(0x006E3190, (DWORD)&my_mbwc);

	// eliminate explicit racism
	patchmem(0x0061BD45, (BYTE*)"\x90\x90", 2);
}

}
}