#include "Globals.h"

#include "General\ModuleInfo.h"

namespace AAPlay {


__declspec(naked) void* __stdcall IllusionMemAlloc(size_t size) {
	//"AA2Play v12 FP v1.4.0a.exe"+21BCA0  <-- memory alloc function, only parameter is eax = size
	__asm {
		mov eax, [esp + 4]
		mov ecx, [General::GameBase]
		add ecx, 0x21BCA0
		call ecx
		ret 4
	}
}

}