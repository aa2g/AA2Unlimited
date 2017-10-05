#include "stdafx.h"
#include "pngdata.h"
#include "zstd.h"

using namespace ExtClass;


namespace Shared {
namespace PNG {


// This gets called when a png card data read routine asks about png size. First, we try to
// extract override files, as well as fetch aau data in case of aaedit. Secondly, we lie
// about the file size returned - we omit the large data blobs, so that those dont end
// up in game saves.
//
// The simple case, on the other hand, happens when scanning a directory. We want to exit
// quickly, while only removing the blob part for the caller.
DWORD OpenCard(HANDLE fh, DWORD *hi, bool simple) {
	bool converted = false;
	Footer footer = { 0 };
	DWORD sz = GetFileSize(fh, NULL);
	DWORD orig_sz = sz;
	if (hi)
		*hi = 0;

	// Read the magic footer
	SetFilePointer(fh, sz - sizeof(Footer), NULL, FILE_BEGIN);
	DWORD got = 0;
	ReadFile(fh, &footer, sizeof(footer), &got, NULL);

	// If a simple read is requested, we only truncate the card but wont otherwise touch it
	if (simple) {
		SetFilePointer(fh, 0, NULL, FILE_BEGIN);
		if (memcmp(footer.vermagic, vermagic, 8))
			return sz;
		return sz - footer.aaublob_delta;
	}

	AAUCardData &cd = g_currentChar->m_cardData;
	cd.Reset();

	LUA_EVENT_NORET("open_card", DWORD(fh));

	// Now try to extract aau data
	BYTE *aaud_ptr, *buf = NULL;
	int aaud_sz;
	int aaud_off;
	int blob_sz = 0;
	int blob_off = 0;
	int illusion_off = sz - footer.illusion_delta; // also points to end of our blob, if there is any
	char *cbuf = NULL, *dbuf = NULL;

	// seems corrupted
	if ((illusion_off < 64) || (illusion_off > sz))
		goto out;

	// newer card, has a magic (and possibly a blob)
	aaud_off = sz - footer.aaud_delta;
	if (!memcmp(footer.vermagic, vermagic, 8)) {
		// 4 is trailing crc, 12 is the IEND chunk
		aaud_sz = illusion_off - aaud_off - 4 - 12;
		if (aaud_sz < 0)
			goto out;
		// seek to AAUD contents
		SetFilePointer(fh, aaud_off, 0, FILE_BEGIN);
		aaud_ptr = buf = new BYTE[sz];
		got = 0;
		ReadFile(fh, buf, aaud_sz, &got, NULL);

		// set up blob
		blob_off = sz - footer.aaublob_delta;
		blob_sz = footer.aaublob_delta - sizeof(footer);
	}
	// older card, no magic, we'll scan for the chunk
	else {
		// no magic, no blob to substract
		footer.aaublob_delta = 0;

		buf = new BYTE[illusion_off];
		DWORD got = 0;
		SetFilePointer(fh, 0, NULL, FILE_BEGIN);
		ReadFile(fh, buf, illusion_off, &got, NULL);

		aaud_ptr = General::FindPngChunk(buf, illusion_off, AAUCardData::PngChunkIdBigEndian);

		// No aau data, plain card
		if (!aaud_ptr)
			goto out;

		aaud_sz = _byteswap_ulong(*((DWORD*)(aaud_ptr)));
		// get past size and chunk tag
		aaud_ptr += 8;

	}

	// No extraction enabled and in play, take a shortcut. In play we load
	// the AAUD info via pngbuffer much later on.
	if (General::IsAAPlay && (g_Config.savedFileUsage == 2))
		goto out;

	cd.FromBuffer((char*)aaud_ptr, aaud_sz);

	// if this is an old card with File entries, convert those to (already decompressed) blob first
	if ((cd.m_version < cd.CurrentVersion) && (blob_sz == 0)) {
		converted = true;
		cd.ConvertFilesToBlob();
		if (!cd.PrepareDumpBlob())
			goto skip_blob;
		goto skip_load;
	}

	// have no blob, do nothing
	if (!blob_sz)
		goto skip_blob;

	// rewrite file paths and check if something is missing or something requests to abort
	if (!cd.PrepareDumpBlob())
		goto skip_blob;

	// we need blob, so load and decompress it
	cbuf = new char[blob_sz];
	SetFilePointer(fh, blob_off, 0, FILE_BEGIN);
	got = 0;
	ReadFile(fh, cbuf, blob_sz, &got, NULL);
	if (got != blob_sz)
		goto skip_blob;
	LOGPRIO(Logger::Priority::SPAM) << std::dec << "Decompressing blob of " << blob_sz << " bytes\r\n";
	int dbsize = ZSTD_getDecompressedSize(cbuf, blob_sz);
	dbuf = new char[dbsize];
	if (ZSTD_decompress(dbuf, dbsize, cbuf, blob_sz) < 0)
		goto skip_blob;
	cd.Blob = dbuf;
	dbuf = NULL;

skip_load:;
	// and dump it; the files to be dumped were already picked by PrepareDumpBlob()
	if (cd.Blob) {
		cd.DumpBlob();
	}


skip_blob:;
	if (cbuf) delete cbuf;
	if (dbuf) delete dbuf;

	// if we have blob entries (even if not extracted), it means this flag has to be checked
	if (General::IsAAEdit)
		AAEdit::g_AAUnlimitDialog.SetSaveFiles((cd.m_blobInfo.size() > 0));
	cd.BlobReset();
	cd.GenAllFileMaps();

out:

	if (General::IsAAEdit)
		AAEdit::g_AAUnlimitDialog.Refresh();
	if (buf) delete buf;

	// return size without aau blob
	sz -= footer.aaublob_delta;
	if ((sz < 0) || (sz > orig_sz))
		sz = 0;
	SetFilePointer(fh, 0, NULL, FILE_BEGIN);
	return sz;
}

static int aaud_off;

static void SavePNGChunk(CharacterStruct *chr, BYTE **outbuf, DWORD *outlen) {
	BYTE *aaudata = General::FindPngChunk(*outbuf, *outlen, AAUCardData::PngChunkIdBigEndian);
	// if no aaud found, just nuke the IEND
	if (!aaudata)
		aaudata = *outbuf + *outlen - 12;
	int pos = aaudata - *outbuf;
	aaud_off = pos + 8; // size + AAUD
	CharInstData *ch;
	if (General::IsAAEdit) {
		AAEdit::g_currChar.m_char = chr;
		ch = &AAEdit::g_currChar;
		// we're about to save a card, so build up a new blob if needed
		if (AAEdit::g_AAUnlimitDialog.IsSaveFilesSet()) {
			auto *eyes = &ch->m_char->m_charData->m_eyes;
			if (eyes->bExtTextureUsed) {
				TCHAR buffer[256];
				mbstowcs(buffer, eyes->texture, 260);
				ch->m_cardData.SetEyeTexture(0, buffer, true);
			}
			else {
				ch->m_cardData.SetEyeTexture(0, NULL, false);
			}
			// The 2nd eye is already set by ui or preceding load
			if (eyes->bExtHighlightUsed) {
				TCHAR buffer[256];
				mbstowcs(buffer, eyes->highlight, 260);
				ch->m_cardData.SetEyeHighlight(buffer);
			}
			else {
				ch->m_cardData.SetEyeHighlight(NULL);
			}

			ch->m_cardData.PrepareSaveBlob();
		}
	}
	else {
		ch = &AAPlay::g_characters[chr->m_seat];
	}

	char *aaudbuf = NULL;
	ch->m_cardData.m_version = ch->m_cardData.CurrentVersion;
	int got = ch->m_cardData.ToBuffer(&aaudbuf);
	if (got) {
		*outbuf = (BYTE*)realloc((void*)*outbuf, pos + got + 12);
		memcpy(*outbuf + pos, aaudbuf, got);
		memcpy(*outbuf + pos + got, "\x00\x00\x00\x00IEND\xAE\x42\x60\x82", 12);
		*outlen = pos + got + 12;
	}
	if (aaudbuf)
		delete aaudbuf;
}

// This function is called by edit to generate raw PNG preview, right before character metadata are appended to it -
// so this is our best chance to stuff in whatever partains to AAU here.
bool(__stdcall *GetPNGOrig)(DWORD _this, CharacterStruct *chr, BYTE **outbuf, DWORD *outlen);
bool __stdcall GetPNG(DWORD _this, CharacterStruct *chr, BYTE **outbuf, DWORD *outlen) {
	bool stat = GetPNGOrig(_this, chr, outbuf, outlen);
	if (!stat) return stat;
	SavePNGChunk(chr, outbuf, outlen);
	LUA_EVENT_NORET("save_card", chr, stat, outbuf, outlen, outlen?*outlen:0);
	return stat;
}

// Called when finishing up writing a png and writing the last delta DWORD. Edit only.
bool __cdecl FinishPNG(HANDLE hf, DWORD *delta, bool dummy) {
	DWORD got = 0;

	// Write the original delta marker
	WriteFile(hf, delta, 4, &got, NULL);

	int blob_off = SetFilePointer(hf, 0, NULL, FILE_CURRENT);
	int illusion_off = blob_off - *delta;
	int blob_sz = 0; // compressed blob size

	AAUCardData &cd = AAEdit::g_currChar.m_cardData;
	if (cd.BlobAt) {
		int worst = ZSTD_compressBound(cd.BlobAt);
		BYTE *buf = new BYTE[worst];
		assert(buf);
		got = 0;

		blob_sz = ZSTD_compress(buf, worst, cd.Blob, cd.BlobAt, 22);
		LOGPRIO(Logger::Priority::SPAM) << std::dec << "PNG blob compressed to " << blob_sz << " bytes\r\n";

		WriteFile(hf, buf, blob_sz, &got, NULL);

		delete buf;
	}
	cd.BlobReset();
	int sz = blob_off + blob_sz + sizeof(Footer);

	Footer footer;
	memcpy(footer.vermagic, vermagic, 8);
	footer.aaublob_delta = sz - blob_off;
	footer.aaud_delta = sz - aaud_off;
	footer.illusion_delta = sz - illusion_off;
	got = 0;
	WriteFile(hf, &footer, sizeof(footer), &got, NULL);
	return true;
}

// Called by play to save a class
bool(__stdcall *SaveClassOrig)(void *cls);
bool __stdcall SaveClass(void *cls) {
	LUA_EVENT_NORET("save_class", cls);
	for (int i = 0; i < 25; i++) {
		CharInstData &ch = AAPlay::g_characters[i];
		if (ch.IsValid()) {
			SavePNGChunk(ch.m_char,(BYTE**)&ch.m_char->m_charData->m_pngBuffer, &ch.m_char->m_charData->m_pngBufferSize);
		}
	}
	return SaveClassOrig(cls);
}

DWORD __stdcall GetFileSizeHook(HANDLE fh, DWORD *hi) {
	return OpenCard(fh, hi, false);
}

DWORD __stdcall GetFileSizeHookSimple(HANDLE fh, DWORD *hi) {
	return OpenCard(fh, hi, true);
}

void InstallHooks() {
	static DWORD GetFileSize_imp = (DWORD)&GetFileSizeHook;
	static DWORD GetFileSizeSimple_imp = (DWORD)&GetFileSizeHookSimple;

	DWORD fsize_addr1 = General::GameBase;
	DWORD fsize_addr2 = General::GameBase;
	DWORD get_png = General::GameBase;

	if (General::IsAAEdit) {
		fsize_addr1 += 0x127DE7;
		fsize_addr2 += 0x127278;
	}
	else {
		fsize_addr1 += 0x139A2F;
		fsize_addr2 += 0x138EC8;
	}

//	Hook((BYTE*)fsize_addr1, { 0xFF, 0x15, HookControl::ANY_DWORD }, { 0xFF, 0x15, HookControl::ABSOLUTE_DWORD, (DWORD)&GetFileSizeSimple_imp }, NULL);
	Hook((BYTE*)fsize_addr2, { 0xFF, 0x15, HookControl::ANY_DWORD }, { 0xFF, 0x15, HookControl::ABSOLUTE_DWORD, (DWORD)&GetFileSize_imp }, NULL);

	if (General::IsAAEdit) {
		// This one is called to produce the initial png preview
		Hook((BYTE*)(General::GameBase + 0x12628b),
		{ 0xE8, HookControl::ANY_DWORD },						//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, (DWORD)&GetPNG },	//redirect to our function
			(DWORD*)&GetPNGOrig);

		Hook((BYTE*)(General::GameBase + 0x12701D),
		{ 0xE8, HookControl::ANY_DWORD },
		{ 0xE8, HookControl::RELATIVE_DWORD, (DWORD)&FinishPNG }, NULL);
	}
	else {
		Hook((BYTE*)(General::GameBase + 0x4732B),
		{ 0xE8, HookControl::ANY_DWORD },
		{ 0xE8, HookControl::RELATIVE_DWORD, (DWORD)&SaveClass }, (DWORD*)&SaveClassOrig);
	}
}

}
}