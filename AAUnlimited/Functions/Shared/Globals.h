#pragma once

#include <d3d9.h>

namespace Shared {


void* __stdcall IllusionMemAlloc(size_t size);
extern D3DMATRIX* (__stdcall *D3DXMatrixMultiply)(D3DMATRIX *pOut, const D3DMATRIX *pM1,const D3DMATRIX *pM2);

void Init();

}