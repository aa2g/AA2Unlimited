#include "Globals.h"

#include "General\ModuleInfo.h"

namespace Shared {


DWORD IllusionMemAllocProc;
__declspec(naked) void* __stdcall IllusionMemAlloc(size_t size) {
	
	__asm {
		mov eax, [esp + 4]
		call [IllusionMemAllocProc]
		ret 4
	}
}

D3DMATRIX* (__stdcall *D3DXMatrixMultiply)(D3DMATRIX *pOut,const D3DMATRIX *pM1,const D3DMATRIX *pM2) = NULL;

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
		IllusionMemAllocProc = General::GameBase + 0x1FE160;
		//AA2Edit.exe+213EB8 - FF 25 C0443901        - jmp dword ptr[AA2Edit.exe+2C44C0]{ ->->d3dx9_42.dll+1A3ED8 }
		*(DWORD*)(&D3DXMatrixMultiply) = General::GameBase + 0x213EB8;
		//AA2Edit.exe+119BBA - E8 71B90C00           - call AA2Edit.exe+1E5530
		*(DWORD*)(&IllusionDeleteXXFileProc) = General::GameBase + 0x1E5530;
	}
	else if (General::IsAAPlay) {
		//"AA2Play v12 FP v1.4.0a.exe"+21BCA0  <-- memory alloc function, only parameter is eax = size
		IllusionMemAllocProc = General::GameBase + 0x21BCA0;
		//AA2Play v12 FP v1.4.0a.exe+2320D0 - FF 25 D8344501        - jmp dword ptr ["AA2Play v12 FP v1.4.0a.exe"+2E34D8] { ->->d3dx9_42.dll+1A3ED8 }
		*(DWORD*)(&D3DXMatrixMultiply) = General::GameBase + 0x2320D0;
	}
}

}