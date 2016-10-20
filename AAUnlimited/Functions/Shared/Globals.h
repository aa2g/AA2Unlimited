#pragma once

#include <d3d9.h>

#include "External\ExternalClasses\XXFile.h"
#include "External\ExternalClasses\CharacterStruct.h"

struct D3DVECTOR4 {
	FLOAT x,y,z,w;
};

struct D3DVECTOR3 {
	FLOAT x,y,z;
};

namespace Shared {

void* __stdcall IllusionMemAlloc(size_t size);
extern D3DMATRIX* (__stdcall *D3DXMatrixMultiply)(D3DMATRIX *pOut, const D3DMATRIX *pM1,const D3DMATRIX *pM2);
extern D3DVECTOR4* (__stdcall *D3DXVec3Transform)(D3DVECTOR4 *pOut, const D3DVECTOR3 *pV, const D3DMATRIX  *pM);
void __stdcall IllusionDeleteXXFile(ExtClass::XXFile* file, ExtClass::CharacterStruct* owner);

void Init();

}