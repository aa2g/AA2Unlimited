#pragma once

#include <Windows.h>

namespace AAEdit {

DWORD __stdcall MeshTextureListStart(BYTE* xxFileBuffer, DWORD offset);
bool __stdcall MeshTextureListFill(BYTE* name, DWORD* xxReadOffset);

void __stdcall MeshTextureStart(BYTE* xxFile, DWORD offset);
bool __stdcall MeshTextureOverrideName(BYTE* buffer, DWORD* xxReadOffset);
void __stdcall MeshTextureOverrideSize(BYTE* buffer, DWORD offset);
bool __stdcall MeshTextureOverrideFile(BYTE* buffer, DWORD* xxReadOffset);

}