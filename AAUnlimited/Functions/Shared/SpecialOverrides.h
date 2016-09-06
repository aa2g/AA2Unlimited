#pragma once

#include <Windows.h>

/*
 * Handles special overrides, like the tan and hair highlights
 */

namespace Shared  {



bool TanOverride(wchar_t** archive, wchar_t** file, DWORD* readBytes, BYTE** outBuffer);
const TextureImage* HairHighlightOverride(wchar_t* texture);
bool HairRedirect(wchar_t** archive, wchar_t** file, DWORD* readBytes, BYTE** outBuffer);




}