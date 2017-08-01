#include "StdAfx.h"

namespace SharedInjections {
namespace ArchiveFile {

/*
* If false is returned, the original function will be executed.
* else, the function is aborted and the results from this function are used.
*/
bool __stdcall OpenFileEvent(wchar_t** paramArchive, wchar_t** paramFile, DWORD* readBytes, BYTE** outBuffer) {
	bool ret;
	ret = Poser::OverrideFile(paramArchive, paramFile, readBytes, outBuffer);
	if (ret) return true;
	ret = Shared::ArchiveReplaceRules(paramArchive, paramFile, readBytes, outBuffer);
	if (ret) return true;
	ret = Shared::TanOverride(paramArchive, paramFile, readBytes, outBuffer);
	if (ret) return true;
	ret = Shared::HairRedirect(paramArchive, paramFile, readBytes, outBuffer);
	if (ret) return true;
	ret = Shared::ArchiveOverrideRules(*paramArchive, *paramFile, readBytes, outBuffer);
	if (ret) return true;
	if (g_Config.bUseShadowing) {
		ret = Shared::OpenShadowedFile(*paramArchive, *paramFile, readBytes, outBuffer);
		if (ret) return true;
	}
	if (g_Config.bUsePPeX)
		ret = g_PPeX.ArchiveDecompress(*paramArchive, *paramFile, readBytes, outBuffer);
	if (g_Config.bUsePP2)
		ret = g_PP2.ArchiveDecompress(*paramArchive, *paramFile, readBytes, outBuffer);

	return ret;
}

DWORD OpenFileNormalExit;
void __declspec(naked) OpenFileRedirect() {
	__asm {
		pushad
		push[esp + 0x20 + 0x10 + 0]
		push edi
		lea eax, [esp + 0x20 + 0xC + 8]
		push eax
		lea eax, [esp + 0x20 + 4 + 0xC]
		push eax
		call OpenFileEvent
		test al, al
		popad
		jz OpenFileRedirect_NormalExit
		mov al, 1
		ret
	OpenFileRedirect_NormalExit :
		push ebp
		mov ebp, esp
		and esp, -8
		jmp[OpenFileNormalExit]
	}
}

void OpenFileInject() {
	if (General::IsAAEdit) {
		//bool someFunc(edi = DWORD* readBytes, wchar* archive, 
		//				someClass* globalClass, wchar* filename, BYTE** outBuffer) {
		/*AA2Edit.exe+1F89F0 - 55                    - push ebp
		AA2Edit.exe+1F89F1 - 8B EC                 - mov ebp,esp
		AA2Edit.exe+1F89F3 - 83 E4 F8              - and esp,-08 { 248 }
		AA2Edit.exe+1F89F6 - 83 EC 18              - sub esp,18 { 24 }
		AA2Edit.exe+1F89F9 - 33 C0                 - xor eax,eax
		AA2Edit.exe+1F89FB - 53                    - push ebx
		AA2Edit.exe+1F89FC - 8B 5D 14              - mov ebx,[ebp+14]
		AA2Edit.exe+1F89FF - 89 44 24 08           - mov [esp+08],eax
		AA2Edit.exe+1F8A03 - 89 44 24 0C           - mov [esp+0C],eax
		AA2Edit.exe+1F8A07 - 89 44 24 10           - mov [esp+10],eax
		AA2Edit.exe+1F8A0B - 89 44 24 14           - mov [esp+14],eax
		AA2Edit.exe+1F8A0F - 89 44 24 18           - mov [esp+18],eax*/
		DWORD address = General::GameBase + 0x1F89F0;
		DWORD redirectAddress = (DWORD)(&OpenFileRedirect);
		Hook((BYTE*)address,
			{ 0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8 },						//expected values
			{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress, 0x90 },	//redirect to our function
			NULL);
		OpenFileNormalExit = General::GameBase + 0x1F89F6;
	}
	else if (General::IsAAPlay) {
		//bool someFunc(edi = DWORD* readBytes, wchar* archive, 
		//				someClass* globalClass, wchar* filename, BYTE** outBuffer) {
		/*AA2Play v12 FP v1.4.0a.exe+216470 - 55                    - push ebp
		AA2Play v12 FP v1.4.0a.exe+216471 - 8B EC                 - mov ebp,esp
		AA2Play v12 FP v1.4.0a.exe+216473 - 83 E4 F8              - and esp,-08 { 248 }
		AA2Play v12 FP v1.4.0a.exe+216476 - 83 EC 18              - sub esp,18 { 24 }
		AA2Play v12 FP v1.4.0a.exe+216479 - 33 C0                 - xor eax,eax
		AA2Play v12 FP v1.4.0a.exe+21647B - 53                    - push ebx
		AA2Play v12 FP v1.4.0a.exe+21647C - 8B 5D 14              - mov ebx,[ebp+14]
		AA2Play v12 FP v1.4.0a.exe+21647F - 89 44 24 08           - mov [esp+08],eax
		AA2Play v12 FP v1.4.0a.exe+216483 - 89 44 24 0C           - mov [esp+0C],eax
		AA2Play v12 FP v1.4.0a.exe+216487 - 89 44 24 10           - mov [esp+10],eax
		AA2Play v12 FP v1.4.0a.exe+21648B - 89 44 24 14           - mov [esp+14],eax
		AA2Play v12 FP v1.4.0a.exe+21648F - 89 44 24 18           - mov [esp+18],eax
		*/
		DWORD address = General::GameBase + 0x216470;
		DWORD redirectAddress = (DWORD)(&OpenFileRedirect);
		Hook((BYTE*)address,
			{ 0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8 },						//expected values
			{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress, 0x90 },	//redirect to our function
			NULL);
		OpenFileNormalExit = General::GameBase + 0x216476;
	}
			
}

}
}
