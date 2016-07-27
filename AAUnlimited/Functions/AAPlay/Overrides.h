#pragma once

#include <Windows.h>

#include "Functions\AAUCardData.h"
#include "External\ExternalClasses\CharacterStruct.h"

namespace AAPlay {

/*
* Override rules are taken from this card.
*/
extern AAUCardData g_currentCard;

/*
 * Event-Callback for archive-file opening
 */
bool ArchiveOverrideRules(wchar_t* archive, wchar_t* file, DWORD* readBytes, BYTE** outBuffer);

/*
* Event-Callback for Hi-Poly-Load to set the current card to get override rules from
*/
void MeshTextureCharLoadStart(ExtClass::CharacterStruct* loadCharacter);
void MeshTextureCharLoadEnd();

/*
* Functions that do the actual texture override in the xxFile analisys functions
*/
DWORD __stdcall MeshTextureListStart(BYTE* xxFileBuffer, DWORD offset);
bool __stdcall MeshTextureListFill(BYTE* name, DWORD* xxReadOffset);

void __stdcall MeshTextureStart(BYTE* xxFile, DWORD offset);
bool __stdcall MeshTextureOverrideName(BYTE* buffer, DWORD* xxReadOffset);
void __stdcall MeshTextureOverrideSize(BYTE* buffer, DWORD offset);
bool __stdcall MeshTextureOverrideFile(BYTE* buffer, DWORD* xxReadOffset);

}