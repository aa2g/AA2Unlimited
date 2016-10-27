#pragma once

#include <Windows.h>

#include "Functions\AAUCardData.h"
#include "Functions\CharInstData.h"
#include "External\ExternalClasses\CharacterStruct.h"

namespace Shared {

	/*
	* Override rules are taken from this card.
	*/
	extern CharInstData* g_currentChar;
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

	/*
	 * XX file modification
	 */ 
	void XXFileModification(ExtClass::XXFile* file, bool backup);

}