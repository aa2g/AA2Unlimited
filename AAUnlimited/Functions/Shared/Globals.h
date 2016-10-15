#pragma once

#include <d3d9.h>

#include "External\ExternalClasses\XXFile.h"
#include "External\ExternalClasses\CharacterStruct.h"

namespace Shared {


void* __stdcall IllusionMemAlloc(size_t size);
extern D3DMATRIX* (__stdcall *D3DXMatrixMultiply)(D3DMATRIX *pOut, const D3DMATRIX *pM1,const D3DMATRIX *pM2);
void __stdcall IllusionDeleteXXFile(ExtClass::XXFile* file, ExtClass::CharacterStruct* owner);

void Init();

}