#include "FrameManipulationTest.h"

#include "Files\Config.h"
#include "MemMods\Hook.h"
#include "General\ModuleInfo.h"
#include "Functions\Shared\Overrides.h"
#include "Functions\Shared\Globals.h"

namespace SharedInjections {
namespace FrameManipulation {


void __stdcall FrameMatrix1ManipulationEvent(const char* name, D3DMATRIX* matrix1) {
	static float yawpitchroll[] = {1,0,0};

	if(strcmp(name,"a01_J_UdeL_01") == 0) {
		D3DMATRIX tmp;
		(*Shared::D3DXMatrixRotationYawPitchRoll)(&tmp,yawpitchroll[0],yawpitchroll[1],yawpitchroll[2]);
		(*Shared::D3DXMatrixMultiply)(matrix1,&tmp,matrix1);
	}
}


DWORD FrameMatrix1ManipulationOriginalFunc;
void __declspec(naked) FrameMatrix1ManipulationRedircet() {
	__asm {
		push[esp+0xC]
		push[esp+0xC]
		push[esp+0xC]
		call FrameMatrix1ManipulationOriginalFunc
		mov eax,[esp+0x2E4 + 0x10]
		mov ecx,[esp+4 + 0x10]
		mov ecx, [ecx+4]
		push eax
		push ecx
		call FrameMatrix1ManipulationEvent

		ret 0xC
	}
}


void FrameMatrix1ManipulationInject() {
	if(General::IsAAEdit) {
		/*AA2Edit.exe+1E40F2 - 8B 84 24 E4020000     - mov eax,[esp+000002E4]
		AA2Edit.exe+1E40F9 - 8D 8C 24 90000000     - lea ecx,[esp+00000090]
		AA2Edit.exe+1E4100 - 51                    - push ecx
		AA2Edit.exe+1E4101 - 8D 54 24 54           - lea edx,[esp+54]
		AA2Edit.exe+1E4105 - 52                    - push edx { scale matrix }
		AA2Edit.exe+1E4106 - 50                    - push eax
		AA2Edit.exe+1E4107 - E8 ACFD0200           - call AA2Edit.exe+213EB8 { ->->d3dx9_42.D3DXMatrixMultiply }
		*/
		DWORD address = General::GameBase + 0x1E4107;
		DWORD redirectAddress = (DWORD)(&FrameMatrix1ManipulationRedircet);
		Hook((BYTE*)address,
		{ 0x8B, 0xAC, 0xFD, 0x02, 0x00 },						//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			&FrameMatrix1ManipulationOriginalFunc);

	}
	
}



}
}