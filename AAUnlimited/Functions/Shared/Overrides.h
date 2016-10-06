#pragma once

#include <Windows.h>

#include "Functions\AAUCardData.h"
#include "External\ExternalClasses\CharacterStruct.h"

namespace Shared {

	/*
	* Override rules are taken from this card.
	*/
	extern AAUCardData* g_currentCard;
	extern bool g_isOverriding;

	bool ArchiveReplaceRules(wchar_t** archive, wchar_t** file, DWORD* readBytes, BYTE** outBuffer);
	/*
	* Event-Callback for archive-file opening
	*/
	bool ArchiveOverrideRules(wchar_t* archive, wchar_t* file, DWORD* readBytes, BYTE** outBuffer);

	/*
	* Event-Callback for Hi-Poly-Load to set the current card to get override rules from
	*/
	void MeshTextureCharLoadStart(ExtClass::CharacterStruct* loadCharacter);
	void MeshTextureCharLoadEnd();

	void __stdcall EyeTextureStart(int leftRight, TCHAR** texture);
	BYTE* EyeTextureDump(wchar_t* fileName, DWORD* readBytes);
	void __stdcall EyeTextureEnd(int leftRight, TCHAR** texture);

	const TextureImage* MeshTextureOverrideRules(wchar_t* fileName);


}