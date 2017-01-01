#pragma once

#include <d3d9.h>

#include "General\DirectXStructs.h"
#include "External\ExternalClasses\XXFile.h"
#include "External\ExternalClasses\CharacterStruct.h"

namespace Shared {

void* __stdcall IllusionMemAlloc(size_t size);
void __stdcall IllusionDeleteXXFile(ExtClass::XXFile* file, ExtClass::CharacterStruct* owner);

//direct x functions
extern D3DMATRIX* (__stdcall *D3DXMatrixMultiply)(D3DMATRIX *pOut,const D3DMATRIX *pM1,const D3DMATRIX *pM2);
extern D3DVECTOR4* (__stdcall *D3DXVec3Transform)(D3DVECTOR4 *pOut,const D3DVECTOR3 *pV,const D3DMATRIX  *pM);
extern D3DQUATERNION* (__stdcall *D3DXQuaternionRotationYawPitchRoll)(D3DQUATERNION *pOut, FLOAT Yaw, FLOAT Pitch, FLOAT Roll);
extern D3DQUATERNION* (__stdcall *D3DXQuaternionNormalize)(D3DQUATERNION *pOut,D3DQUATERNION *pQ);
extern D3DQUATERNION* (__stdcall *D3DXQuaternionMultiply) (D3DQUATERNION *pOut, const D3DQUATERNION *pQ1, const D3DQUATERNION *pQ2);
extern HRESULT (__stdcall *D3DXMatrixDecompose)(D3DVECTOR3 *pOutScale,D3DQUATERNION *pOutRotation,D3DVECTOR3 *pOutTranslation,const D3DMATRIX *pM);
extern D3DMATRIX* (__stdcall *D3DXMatrixTranslation)(D3DMATRIX *pOut, float x, float y, float z);
extern D3DMATRIX* (__stdcall *D3DXMatrixScaling)(D3DMATRIX *pOut, float sx, float sy, float sz);
extern D3DMATRIX* (__stdcall *D3DXMatrixRotationQuaternion)(D3DMATRIX *pOut,const D3DQUATERNION *pQ);
extern D3DMATRIX* (__stdcall *D3DXMatrixRotationYawPitchRoll)(D3DMATRIX *pOut,FLOAT Yaw,FLOAT Pitch,FLOAT Roll);

void Init();

}