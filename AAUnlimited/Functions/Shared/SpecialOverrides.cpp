#pragma once

#include <regex>

#include "Overrides.h" //for g_currentCard
#include "General\Util.h"
#include "Functions\Shared\Globals.h"

namespace Shared {



bool TanOverride(wchar_t** archive, wchar_t** file, DWORD* readBytes, BYTE** outBuffer) {
	if (g_currentChar->m_cardData.GetTanName().size() == 0) return false;
	/* C:\Users\user1\Desktop\04.bmp (11-Aug-16 3:10:42 AM)
	StartOffset: 00000000, EndOffset: 0000007D, Length: 0000007E */

	static const BYTE transpBmp[] = {
			0x42, 0x4D, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7A, 0x00,
			0x00, 0x00, 0x6C, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
			0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00,
			0x00, 0x00, 0x13, 0x0B, 0x00, 0x00, 0x13, 0x0B, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x47, 0x52, 0x73, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};

	TCHAR* archiveFile = General::FindFileInPath(*archive);
	if (wcscmp(archiveFile, TEXT("jg2e03_00_00.pp")) != 0) return false;
	//tan format: A00_00_00_xx_yy.bmp|tga, with xx being the slot, and yy the image
	if (!General::StartsWith(*file, TEXT("A00_00_00_"))) return false;
	wchar_t* end;
	long slot = wcstol(*file + 10, &end, 10);
	if (*end != L'_') return false;
	end++;
	long imgNum = wcstol(end, &end, 10);
	if (wcscmp(end, TEXT(".bmp")) != 0 && wcscmp(end, TEXT(".tga")) == 0) return false;

	//slot is the tan slot, imgNum the image number. to checks on that next.
	if (imgNum < 0 || imgNum >= 5) return false;

	const TextureImage& img = g_currentChar->m_cardData.GetTanTex(imgNum);
	if (img.IsGood()) {
		void* fileBuffer = Shared::IllusionMemAlloc(img.GetFileSize());
		img.WriteToBuffer((BYTE*)fileBuffer);
		*outBuffer = (BYTE*)fileBuffer;
		*readBytes = img.GetFileSize();
		return true;
	}
	else {
		void* fileBuffer = Shared::IllusionMemAlloc(sizeof(transpBmp));
		memcpy_s(fileBuffer, sizeof(transpBmp), transpBmp, sizeof(transpBmp));
		*outBuffer = (BYTE*)fileBuffer;
		*readBytes = sizeof(transpBmp);
		return true;
	}

	return false;
}

const TextureImage* HairHighlightOverride(wchar_t* texture) {
	if (General::StartsWith(texture, TEXT("Asp00_20_00_00_00"))) {
		const TextureImage& img = g_currentChar->m_cardData.GetHairHighlightTex();
		if (img.IsGood()) {
			return &img;
		}
	}
	return NULL;
}

bool HairRedirect(wchar_t** archive, wchar_t** file, DWORD* readBytes, BYTE** outBuffer) {
	//jg2e(l)02_01|2|3|4_*.pp. file names AS00_20|1|2|3_slot_flip.xx|xa|bps
	//first, check if archive fits
	/*TCHAR* archiveFile = General::FindFileInPath(*archive);
	if (!General::StartsWith(archiveFile, TEXT("jg2e02_0"))) return false;
	wchar_t* it = archiveFile + 8;
	wchar_t* archiveKindIt = it;
	int hairKind = *it - L'0';
	if (hairKind < 0 || hairKind > 4) return false;
	
	//now parse the file name
	if (!General::StartsWith(*file, TEXT("AS00_2"))) return false;
	it = *file + 6;
	wchar_t* fileKindIt = it;
	int fileHairKind = *it - L'0';
	if (fileHairKind != hairKind) return false;

	it++; if (*it != L'_') return false;
	it++;

	long slot = wcstol(it, &it, 10);
	if (*it != L'_') return false;
	it++;
	long flip = wcstol(it, &it, 10);
	if (*it != L'.') return false;
	it++;

	BYTE newCat = g_currentChar->m_cardData.GetHairRedirect(fileHairKind);
	wchar_t wcNewCat = L'0' + newCat;
	*archiveKindIt = wcNewCat;
	*fileKindIt = wcNewCat;

	return false; //return false anyway, since this is merely a redirect*/
	return false;
}



}