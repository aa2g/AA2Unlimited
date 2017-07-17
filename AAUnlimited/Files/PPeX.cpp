// This is a straightforward implementation assuming that the ppx files are well-formed.
// Store, zstd and opus compression only.
// No performance/caching/threading opts attempted yet - this is slow.
// No error checking is done whatsoever, corrupted files will simply crash.

#include "Functions/Shared/Globals.h"
#include "PPeX.h"
#include <stdint.h>
#include <fstream>
#include <iostream>
#include <io.h>
#include "zstd.h"

using namespace std;

PPeX g_PPeX;

// hardcoded. neither 24 or 96k are viable option for the game.
#define SRATE 48000


enum {
	FLAG_ZSTD = 1,
	FLAG_OPUS = 2,
	FLAG_AUDIO = 4,
};

#pragma pack(push, 1)
typedef struct {
	char magic[4];
	uint32_t frame;
	uint32_t bitrate;
	uint8_t nchan;
	uint16_t count;
	uint8_t data[0];
} xgg;

typedef struct {
	char riffid[4];
	uint32_t size;
	char waveid[4];
	char fmtid[4];
	uint32_t sub16;
	uint16_t fmt1;
	uint16_t nchan;
	uint32_t srate;
	uint32_t brate;
	uint16_t balign;
	uint16_t bps16;
	char dataid[4];
	uint32_t size2;
	uint8_t data[0];
} wav;
#pragma pack(pop)

class binstream : public ifstream {
public:;
	inline binstream(FILE *afh) : ifstream(afh) {};

	inline auto i8() { uint8_t n; read((char*)&n, sizeof(n)); return n; }
	inline auto i16() { uint16_t n; read((char*)&n, sizeof(n)); return n; }
	inline auto i32() { uint32_t n; read((char*)&n, sizeof(n)); return n; }
	inline auto i64() { uint64_t n; read((char*)&n, sizeof(n)); return n; }

	//inline short operator~() { short n; read((char*)&n, sizeof(n)); return n; }
	inline void skip(int n) { seekg(n, cur); }
};

PPeX::PPeX() {
	int err;
	decoder[0] = opus_decoder_create(SRATE, 1, &err);
	decoder[1] = opus_decoder_create(SRATE, 2, &err);
}

void PPeX::AddPath(const wstring &path) {
	WIN32_FIND_DATA fd;
	HANDLE fh;

	fh = FindFirstFile((path + wstring(L"\\*.ppx")).c_str(), &fd);
	if (fh == INVALID_HANDLE_VALUE)
		return;
	do {
		AddArchive(fd.cFileName);
	} while (FindNextFile(fh, &fd));
}

void PPeX::AddArchive(const wchar_t *fn) {
	// md5 -> fentry, for dupe backrefs
	map<uint64_t, fileEntry> hashes;


	FILE *afh = _wfopen(fn, L"rb");
	binstream fh(afh);

	// dummy header
	fh.skip(10);
	fh.skip(uint16_t(fh.i16()));

	// get number of files
	uint32_t number = fh.i32();
	fh.i32();

	for (int i = 0; i < number; i++) {
		uint8_t comp = fh.i8();
		fh.skip(7);
		uint64_t md5 = fh.i64();
		fh.skip(48+8);

		// path name
		uint16_t nlen = fh.i16();
		char *buf = new char[nlen];
		wchar_t *wfn = (wchar_t*)buf;
		fh.read(buf, nlen);

		fileEntry fe;

		// a dupe?
		if (comp == 0xff) {
			fe = hashes[md5];
			fh.skip(12);
		}
		else {
			// now fetch the important bits
			fe.hid = handles.size();
			fe.offset = fh.i64();
			fe.csize = fh.i32();
			fe.usize = fh.i32();
			fe.flags = 0;
			if (comp == 2)
				fe.flags |= FLAG_ZSTD;
			wchar_t *p = wcsrchr(wfn, L'.');
			if (p) {
				if (!wcscmp(p, L".xgg")) {
					wcscpy(p, L".wav");
					fe.flags |= FLAG_OPUS;
				}
				fe.flags |= FLAG_AUDIO;
			}
		}

		files[wstring(wfn)] = fe;
		hashes[md5] = fe;
	}

	handles.push_back(HANDLE(_get_osfhandle(_fileno(afh))));
}


bool PPeX::ArchiveDecompress(wchar_t* paramArchive, wchar_t* paramFile, DWORD* readBytes, BYTE** outBuffer) {
	auto path = wstring(paramArchive) + L"/" + paramFile;
	fileEntry &fe = files[path];
	uint64_t off = fe.offset;
	uint8_t *ubuf = NULL;
	uint8_t *cbuf = NULL;

	ubuf = (uint8_t*)Shared::IllusionMemAlloc(fe.usize);
	if (fe.flags & (FLAG_ZSTD|FLAG_OPUS)) {
		cbuf = (uint8_t*)malloc(fe.csize);
	}

	uint8_t *buf = cbuf ? cbuf : ubuf;
	OVERLAPPED over = { 0 };

	over.Offset = off;
	over.OffsetHigh = off >> 32;

	DWORD out = 0;
	ReadFile(handles[fe.hid], cbuf, fe.csize, &out, &over);

	*outBuffer = (BYTE*)*ubuf;

	if (!cbuf)
		return true;

	if (fe.flags & FLAG_ZSTD) {
		ZSTD_decompress(ubuf, fe.usize, cbuf, fe.csize);
	} if (fe.flags & FLAG_OPUS) {
		xgg *x = (xgg*)cbuf;
		uint8_t *data = x->data;
		uint16_t *pcm = (uint16_t*)(ubuf + sizeof(wav));
		uint32_t sz = 0;
		uint32_t nchan = x->nchan;
		for (int i = 0; i < x->count; i++) {
			uint32_t frm = *(uint32_t*)data;
			int got = opus_decode(decoder[nchan-1], data, frm, (opus_int16*)pcm, 6 * 960, 0);
			data += frm;
			if (got < 0) break; // minimal sanity
			// convert to little endian
			got *= nchan;
			for (int j = 0; j < got; j++, pcm++)
				*pcm = (*pcm >> 8) | (*pcm << 8);
			sz += got*2;
		}
		// let's make up some shoddy wav header
		wav twav = {
			{'R','I','F','F'},
			sz + sizeof(wav) - 8,
			{'W','A','V','E'},
			{'f','m','t',' '},
			16,
			1,
			nchan,
			SRATE,
			SRATE * nchan * 2,
			nchan * 2,
			16,
			{'d','a','t','a'},
			sz
		};
		*((wav*)ubuf) = twav;
	}
	free(cbuf);
	return true;
}

