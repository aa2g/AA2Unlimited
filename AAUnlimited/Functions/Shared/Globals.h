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
extern D3DXVECTOR4* (__stdcall *D3DXVec3Transform)(D3DXVECTOR4 *pOut,const D3DXVECTOR3 *pV,const D3DMATRIX  *pM);
inline D3DXQUATERNION* D3DXQuaternionConjugate(D3DXQUATERNION *pOut, CONST D3DXQUATERNION *pQ) {
	pOut->x = -pQ->x;
	pOut->y = -pQ->y;
	pOut->z = -pQ->z;
	pOut->w = pQ->w;
	return pOut;
}
inline D3DXQUATERNION* D3DXQuaternionIdentity(D3DXQUATERNION *pOut) {
	pOut->x = pOut->y = pOut->z = 0.0f;
	pOut->w = 1.0f;
	return pOut;
}
extern D3DXQUATERNION* (__stdcall *D3DXQuaternionMultiply) (D3DXQUATERNION *pOut, const D3DXQUATERNION *pQ1, const D3DXQUATERNION *pQ2);
extern D3DXQUATERNION* (__stdcall *D3DXQuaternionNormalize)(D3DXQUATERNION *pOut, D3DXQUATERNION *pQ);
extern D3DXQUATERNION* (__stdcall *D3DXQuaternionRotationAxis)(D3DXQUATERNION *pOut, const D3DXVECTOR3* pV, FLOAT Angle);
extern void (__stdcall *D3DXQuaternionToAxisAngle)(CONST D3DXQUATERNION *pQ, D3DXVECTOR3 *pAxis, FLOAT *pAngle);
extern D3DXQUATERNION* (__stdcall *D3DXQuaternionRotationYawPitchRoll)(D3DXQUATERNION *pOut, FLOAT Yaw, FLOAT Pitch, FLOAT Roll);
extern HRESULT (__stdcall *D3DXMatrixDecompose)(D3DXVECTOR3 *pOutScale,D3DXQUATERNION *pOutRotation,D3DXVECTOR3 *pOutTranslation,const D3DMATRIX *pM);
extern D3DMATRIX* (__stdcall *D3DXMatrixTranslation)(D3DMATRIX *pOut, float x, float y, float z);
extern D3DMATRIX* (__stdcall *D3DXMatrixScaling)(D3DMATRIX *pOut, float sx, float sy, float sz);
extern D3DMATRIX* (__stdcall *D3DXMatrixRotationQuaternion)(D3DMATRIX *pOut,const D3DXQUATERNION *pQ);
extern D3DMATRIX* (__stdcall *D3DXMatrixRotationYawPitchRoll)(D3DMATRIX *pOut,FLOAT Yaw,FLOAT Pitch,FLOAT Roll);
void Init();

}
