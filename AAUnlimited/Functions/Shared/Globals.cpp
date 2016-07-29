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

void Init() {
	if (General::IsAAEdit) {
		//call AA2Edit.exe+1FE160 <-- memory alloc function, only parameter is eax = size
		IllusionMemAllocProc = General::GameBase + 0x1FE160;
	}
	else if (General::IsAAPlay) {
		//"AA2Play v12 FP v1.4.0a.exe"+21BCA0  <-- memory alloc function, only parameter is eax = size
		IllusionMemAllocProc = General::GameBase + 0x21BCA0;
	}
}

}