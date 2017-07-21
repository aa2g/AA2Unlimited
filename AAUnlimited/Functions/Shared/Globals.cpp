#include "Globals.h"

#include "General\ModuleInfo.h"

namespace Shared {


HANDLE *IllusionMemAllocHeap;
DWORD *IllusionMemUsed;
void* __stdcall IllusionMemAlloc(size_t size) {
	*IllusionMemUsed += size;
	if (!*IllusionMemAllocHeap)
		*IllusionMemAllocHeap = HeapCreate(0, 0, 0);
	return HeapAlloc(*IllusionMemAllocHeap, HEAP_ZERO_MEMORY, size);
}

D3DMATRIX* (__stdcall *D3DXMatrixMultiply)(D3DMATRIX *pOut,const D3DMATRIX *pM1,const D3DMATRIX *pM2);
D3DXVECTOR4* (__stdcall *D3DXVec3Transform)(D3DXVECTOR4 *pOut,const D3DXVECTOR3 *pV,const D3DMATRIX  *pM);
D3DXQUATERNION* (__stdcall *D3DXQuaternionRotationYawPitchRoll)(D3DXQUATERNION *pOut,FLOAT Yaw,FLOAT Pitch,FLOAT Roll);
D3DXQUATERNION* (__stdcall *D3DXQuaternionNormalize)(D3DXQUATERNION *pOut,D3DXQUATERNION *pQ);
D3DXQUATERNION* (__stdcall *D3DXQuaternionMultiply) (D3DXQUATERNION *pOut,const D3DXQUATERNION *pQ1,const D3DXQUATERNION *pQ2);
HRESULT(__stdcall *D3DXMatrixDecompose)(D3DXVECTOR3 *pOutScale,D3DXQUATERNION *pOutRotation,D3DXVECTOR3 *pOutTranslation,const D3DMATRIX *pM);
D3DMATRIX* (__stdcall *D3DXMatrixTranslation)(D3DMATRIX *pOut, float x, float y, float z);
D3DMATRIX* (__stdcall *D3DXMatrixScaling)(D3DMATRIX *pOut, float sx, float sy, float sz);
D3DMATRIX* (__stdcall *D3DXMatrixRotationQuaternion)(D3DMATRIX *pOut, const D3DXQUATERNION *pQ);
D3DMATRIX* (__stdcall *D3DXMatrixRotationYawPitchRoll)(D3DMATRIX *pOut, FLOAT Yaw, FLOAT Pitch, FLOAT Roll);
D3DXQUATERNION* (__stdcall *D3DXQuaternionRotationAxis)(D3DXQUATERNION *pOut, const D3DXVECTOR3* pV, FLOAT Angle);
void (__stdcall *D3DXQuaternionToAxisAngle)(CONST D3DXQUATERNION *pQ, D3DXVECTOR3 *pAxis, FLOAT *pAngle);

void (__cdecl *IllusionDeleteXXFileProc)(void* someStruct, ExtClass::XXFile* file);
void __stdcall IllusionDeleteXXFile(ExtClass::XXFile* file, ExtClass::CharacterStruct* owner) {
	//cdecl function, void (someStruct*, XXFile*), where someStruct* is *(owner->somePtr+0x10) (always base+0x353290?)
	//AA2Edit.exe+119BBA - E8 71B90C00           - call AA2Edit.exe+1E5530
	void* someStruct = *(void**)((BYTE*)(owner->m_somePointer) + 0x10);
	//(*IllusionDeleteXXFileProc)(someStruct,file);
}

void Init() {
	if (General::IsAAEdit) {
		//call AA2Edit.exe+1FE160 <-- memory alloc function, only parameter is eax = size
		//IllusionMemAllocProc = General::GameBase + 0x1FE160;
		IllusionMemAllocHeap = (HANDLE*)(General::GameBase + 0x383640);
		IllusionMemUsed = (DWORD*)(General::GameBase + 0x38363C);


		//AA2Edit.exe+213EB8 - FF 25 C0443901        - jmp dword ptr[AA2Edit.exe+2C44C0]{ ->->d3dx9_42.dll+1A3ED8 }
		/**(DWORD*)(&D3DXMatrixMultiply) = General::GameBase + 0x213EB8;
		//AA2Edit.exe+213EE8 - FF 25 BC440501        - jmp dword ptr[AA2Edit.exe+2C44BC]{ ->->d3dx9_42.dll+1A246F }
		*(DWORD*)(&D3DXVec3Transform) = General::GameBase + 0x213EE8;*/
		//AA2Edit.exe+119BBA - E8 71B90C00           - call AA2Edit.exe+1E5530
		*(DWORD*)(&IllusionDeleteXXFileProc) = General::GameBase + 0x1E5530;
		//direct x stuff
		/* D3DXQuaternionMultiply
		AA2Edit.exe+213F90 - FF 25 00453800        - jmp dword ptr [AA2Edit.exe+2C4500] { ->->d3dx9_42.dll+1A260E }*/
		
	}
	else if (General::IsAAPlay) {
		//"AA2Play v12 FP v1.4.0a.exe"+21BCA0  <-- memory alloc function, only parameter is eax = size
		//IllusionMemAllocProc = General::GameBase + 0x21BCA0;
		IllusionMemAllocHeap = (HANDLE*)(General::GameBase + 0x3A6744);
		IllusionMemUsed = (DWORD*)(General::GameBase + 0x3A6740);

		//AA2Play v12 FP v1.4.0a.exe+2320D0 - FF 25 D8344501        - jmp dword ptr ["AA2Play v12 FP v1.4.0a.exe"+2E34D8] { ->->d3dx9_42.dll+1A3ED8 }
		/**(DWORD*)(&D3DXMatrixMultiply) = General::GameBase + 0x2320D0;
		//AA2Play v12 FP v1.4.0a.exe+232100 - FF 25 D4345E00        - jmp dword ptr ["AA2Play v12 FP v1.4.0a.exe"+2E34D4] { ->->d3dx9_42.dll+1A246F }
		*(DWORD*)(&D3DXVec3Transform) = General::GameBase + 0x232100;*/
	}

	//get direct x functions
	HMODULE d3d = GetModuleHandle(TEXT("d3dx9_42.dll"));
#define GETPROC(procVar) procVar = reinterpret_cast<decltype(procVar)>(GetProcAddress(d3d,#procVar))
	GETPROC(D3DXMatrixMultiply);
	GETPROC(D3DXVec3Transform);
	GETPROC(D3DXMatrixDecompose);
	GETPROC(D3DXQuaternionMultiply);
	GETPROC(D3DXQuaternionNormalize);
	GETPROC(D3DXQuaternionRotationYawPitchRoll);
	GETPROC(D3DXMatrixTranslation);
	GETPROC(D3DXMatrixScaling);
	GETPROC(D3DXMatrixRotationQuaternion);
	GETPROC(D3DXMatrixRotationYawPitchRoll);
	GETPROC(D3DXQuaternionRotationAxis);
	GETPROC(D3DXQuaternionToAxisAngle);

#undef GETPROC

	
}

}