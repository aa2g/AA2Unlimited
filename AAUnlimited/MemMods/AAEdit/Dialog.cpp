#include "Dialog.h"

#include <Windows.h>

#include "MemMods\Hook.h"
#include "General\ModuleInfo.h"

#include "External\ExternalVariables\AAEdit\WindowData.h"
#include "Functions\AAEdit\UnlimitedDialog.h"

/*
 * These are similar to the AAFace versions.
 * In fact, there are pretty much copied.
 * Good thing i have a cleaner hook this time, so i can make it work
 */
namespace EditInjections {
namespace Dialog {

void __stdcall DialogProcPre(void* internclass, HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (wnd != NULL && wnd == ExtVars::AAEdit::Dialogs[1]) {
		AAEdit::SystemDialogNotification(internclass, wnd, msg, wparam, lparam);
	}
	
}

void DialogProcPost() {

}

DWORD DialogProcReturnPoint;
void __declspec(naked) DialogProcRedirect() {
	__asm {
		//original code
		push ebx
		//also, save edx
		push edx
		//copy parameters
		push [esp+0x10]
		push [esp+0x10]
		push [esp+0x10]
		push [esp+0x10]
		push esi //also, class as parameter
		call DialogProcPre
		pop edx
		//call original function
		mov ecx, esi
		call edx
		push eax //save return value
		call DialogProcPost
		pop eax //restore return value
		jmp [DialogProcReturnPoint]
	}
}

void DialogProcInject() {
	//general virtual handler for all windows, including the main window.
	//we can use the hwnd to determine which window it is and distribute the calls accordingly.
	//4 param ecx stdcall
	/*AA2Edit.exe+19ABD9 - 8B 4C 24 1C           - mov ecx,[esp+1C]
	AA2Edit.exe+19ABDD - 8B 06                 - mov eax,[esi]
	AA2Edit.exe+19ABDF - 8B 50 04              - mov edx,[eax+04]
	AA2Edit.exe+19ABE2 - 55 - push ebp{ lparam }
	AA2Edit.exe+19ABE3 - 51 - push ecx{ wparam }
	AA2Edit.exe+19ABE4 - 57 - push edi{ msg }
	AA2Edit.exe+19ABE5 - 53 - push ebx{ hwnd }
	AA2Edit.exe+19ABE6 - 8B CE - mov ecx, esi
	AA2Edit.exe+19ABE8 - FF D2 - call edx{ class->handler(hwnd, msg, wparam, lparam) }
	AA2Edit.exe+19ABEA - 8B D8                 - mov ebx,eax*/

	DWORD address = General::GameBase + 0x19ABE5;
	DWORD redirectAddress = (DWORD)(&DialogProcRedirect);
	Hook((BYTE*)address,
		{ 0x53, 
		0x8B, 0xCE, 
		0xFF, 0xD2 },						//expected values
		{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		NULL);
	DialogProcReturnPoint = General::GameBase + 0x19ABEA;
}


}
}