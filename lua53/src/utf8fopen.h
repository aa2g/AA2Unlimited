#include <stdio.h>
#include <windows.h>
#include <malloc.h>

#define fopen _utf8_fopen
static inline FILE *_utf8_fopen(const char *name, const char *mode) {
	int nlen, mlen;
	nlen = strlen(name) + 1;
	mlen = strlen(mode) + 1;
	wchar_t *nbuf = alloca(nlen * 2);
	wchar_t *mbuf = alloca(mlen * 2);
	MultiByteToWideChar(CP_UTF8, 0, name, nlen, nbuf, nlen);
	MultiByteToWideChar(CP_UTF8, 0, mode, mlen, mbuf, mlen);
	return _wfopen(nbuf, mbuf);
}
