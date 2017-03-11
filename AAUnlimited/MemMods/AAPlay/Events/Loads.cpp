#include "Loads.h"

#include "MemMods\Hook.h"
#include "General\ModuleInfo.h"
#include "External\ExternalClasses\CharacterStruct.h"

#include "Functions\Shared\Overrides.h"
#include "Functions\AAPlay\Globals.h"
#include "Functions\AAPlay\Poser.h"
#include "Functions/AAPlay/Facecam.h"

#include "Functions\Shared\TriggerEventDistributor.h"

using namespace Shared::Triggers;

namespace PlayInjections {
/*
 * Events for the Loading of stuff, such as hi-poly models, lo-poly models etc
 */
namespace Loads {

namespace {
	int loc_hiPolyLoaded;
}


void __stdcall HiPolyLoadStartEvent(ExtClass::CharacterStruct* loadCharacter) {
	Shared::MeshTextureCharLoadStart(loadCharacter);
	Poser::SetTargetCharacter(loadCharacter);
	//throw high poly event
	HiPolyInitData data;
	data.card = AAPlay::GetSeatFromStruct(loadCharacter);
	loc_hiPolyLoaded = data.card;
	ThrowEvent(&data);
	
} 

void __stdcall HiPolyLoadEndEvent() {
	Shared::MeshTextureCharLoadEnd();
	//throw high poly end event
	HiPolyEndData data;
	data.card = loc_hiPolyLoaded;
	ThrowEvent(&data);
}

void __stdcall SaveLoadEvent() {
	AAPlay::InitOnLoad();
	Facecam::Cleanup();
}

void __stdcall TransferInEvent(ExtClass::CharacterStruct* character) {
	AAPlay::InitTransferedCharacter(character);
}

void __stdcall TransferOutEvent(ExtClass::CharacterStruct* character) {
	AAPlay::RemoveTransferedCharacter(character);
}

void __declspec(naked) HiPolyLoadsStartRedirect() {
	__asm {
		pushad
		push ecx
		call HiPolyLoadStartEvent
		popad
		//emulate original code (the push 518)
		push [esp]
		mov dword ptr [esp+4], 518
		ret
	}
}

void __declspec(naked) HiPolyLoadsEndRedirect() {
	__asm {
		pushad
		call HiPolyLoadEndEvent
		popad
		mov esp, ebp
		pop ebp
		ret 0x10
	}
}

//while the exit looks the same, the entry for the male method is slightly different,
//so it needs its own function
void __declspec(naked) HiPolyLoadsStartRedirectMale() {
	__asm {
		pushad
		push ecx
		call HiPolyLoadStartEvent
		popad
		//emulate original code (mov eax, fs[00000000])
		mov eax, fs:[00000000]
		ret
	}
}


void HiPolyLoadsInjection() {
	//female load function
	/*AA2Play v12 FP v1.4.0a.exe+111B70 - 55                    - push ebp			<-- beginning of function
	AA2Play v12 FP v1.4.0a.exe+111B71 - 8B EC                 - mov ebp,esp
	AA2Play v12 FP v1.4.0a.exe+111B73 - 83 E4 F8              - and esp,-08 { 248 }
	...
	AA2Play v12 FP v1.4.0a.exe+111BAA - 64 A3 00000000        - mov fs:[00000000],eax { 0 }
	AA2Play v12 FP v1.4.0a.exe+111BB0 - 68 06020000           - push 00000206 { 518 }
	AA2Play v12 FP v1.4.0a.exe+111BB5 - 8B D9                 - mov ebx,ecx
	AA2Play v12 FP v1.4.0a.exe+111BB5 - 8B D9                 - mov ebx,ecx
	AA2Play v12 FP v1.4.0a.exe+111BB7 - 33 FF                 - xor edi,edi
	AA2Play v12 FP v1.4.0a.exe+111BB9 - 8D 8C 24 B2000000     - lea ecx,[esp+000000B2]

		...

	AA2Play v12 FP v1.4.0a.exe+113627 - E8 2D641700           - call "AA2Play v12 FP v1.4.0a.exe"+289A59 { ->AA2Play v12 FP v1.4.0a.exe+289A59 }
	AA2Play v12 FP v1.4.0a.exe+11362C - 8B E5                 - mov esp,ebp			<-- ending of function
	AA2Play v12 FP v1.4.0a.exe+11362E - 5D                    - pop ebp
	AA2Play v12 FP v1.4.0a.exe+11362F - C2 1000               - ret 0010 { 16 }
	*/
	DWORD address = General::GameBase + 0x111BB0;
	DWORD redirectAddress = (DWORD)(&HiPolyLoadsStartRedirect);
	Hook((BYTE*)address,
		{ 0x68, 0x06, 0x02, 0x00, 0x00 },							//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress  },	//redirect to our function
		NULL);

	address = General::GameBase + 0x11362C;
	redirectAddress = (DWORD)(&HiPolyLoadsEndRedirect);
	Hook((BYTE*)address,
		{ 0x8B, 0xE5, 
		0x5D,
		0xC2, 0x10, 0x00 },							//expected values
		{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress, 0x90 },	//redirect to our function
		NULL);

	//male load function
	/*AA2Play v12 FP v1.4.0a.exe+108490 - 55                    - push ebp
	AA2Play v12 FP v1.4.0a.exe+108491 - 8B EC                 - mov ebp,esp
	AA2Play v12 FP v1.4.0a.exe+108493 - 83 E4 F8              - and esp,-08 { 248 }
	AA2Play v12 FP v1.4.0a.exe+108496 - 6A FF                 - push -01 { 255 }
	AA2Play v12 FP v1.4.0a.exe+108498 - 68 2A084A01           - push "AA2Play v12 FP v1.4.0a.exe"+2E082A { [139] }
	AA2Play v12 FP v1.4.0a.exe+10849D - 64 A1 00000000        - mov eax,fs:[00000000] { 0 }
	AA2Play v12 FP v1.4.0a.exe+1084A3 - 50                    - push eax
	AA2Play v12 FP v1.4.0a.exe+1084A4 - 81 EC A0040000        - sub esp,000004A0 { 1184 }
	AA2Play v12 FP v1.4.0a.exe+1084AA - A1 A03A5201           - mov eax,["AA2Play v12 FP v1.4.0a.exe"+363AA0] { [4ADAB32A] }
	AA2Play v12 FP v1.4.0a.exe+1084AF - 33 C4                 - xor eax,esp
	*/
	//...
	//
	/*AA2Play v12 FP v1.4.0a.exe+109656 - E8 FE031800           - call "AA2Play v12 FP v1.4.0a.exe"+289A59 { ->AA2Play v12 FP v1.4.0a.exe+289A59 }
	AA2Play v12 FP v1.4.0a.exe+10965B - 8B E5                 - mov esp,ebp
	AA2Play v12 FP v1.4.0a.exe+10965D - 5D                    - pop ebp
	AA2Play v12 FP v1.4.0a.exe+10965E - C2 1000               - ret 0010 { 16 }
	*/

	address = General::GameBase + 0x10849D;
	redirectAddress = (DWORD)(&HiPolyLoadsStartRedirectMale);
	Hook((BYTE*)address,
		{ 0x64, 0xA1, 0x00, 0x00, 0x00, 0x00 },							//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress, 0x90 },	//redirect to our function
		NULL);

	address = General::GameBase + 0x10965B;
	redirectAddress = (DWORD)(&HiPolyLoadsEndRedirect);
	Hook((BYTE*)address,
	{ 0x8B, 0xE5,
		0x5D,
		0xC2, 0x10, 0x00 },							//expected values
		{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress, 0x90 },	//redirect to our function
		NULL);
	
}

DWORD SaveFileLoadOriginalFunc;
void __declspec(naked) SaveFileLoadRedirect() {
	__asm {
		push [esp+4]
		call [SaveFileLoadOriginalFunc]

		call SaveLoadEvent

		ret 4
	}
}

void SaveFileLoadInjection() {
	//1 param stdcall function that loads characters into the global character array (and more).
	/*AA2Play v12 FP v1.4.0a.exe+B26A8 - 8B 50 28              - mov edx,[eax+28]
	AA2Play v12 FP v1.4.0a.exe+B26AB - 52                    - push edx
	AA2Play v12 FP v1.4.0a.exe+B26AC - E8 4F150400           - call "AA2Play v12 FP v1.4.0a.exe"+F3C00{ ->AA2Play v12 FP v1.4.0a.exe+F3C00 }*/
	DWORD address = General::GameBase + 0xB26AC;
	DWORD redirectAddress = (DWORD)(&SaveFileLoadRedirect);
	Hook((BYTE*)address,
		{ 0xE8, 0x4F, 0x15, 0x04, 0x00 },							//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		&SaveFileLoadOriginalFunc);
}

DWORD TransferInOriginalFunc;
void __declspec(naked) TransferInRedirect() {
	__asm {
		push [esp+8]
		push [esp+8]
		call [TransferInOriginalFunc]
		pushad
		push esi
		call TransferInEvent
		popad
		ret 8
	}
}

void TransferInInjection() {
	//sets the pointer in the pointer array to the character struct in [edi]. no stack parameters, no return value.
	/*AA2Play v12 FP v1.4.0a.exe+EBA5F - 8D 5E 68              - lea ebx,[esi+68]
	AA2Play v12 FP v1.4.0a.exe+EBA62 - 8D 7C 24 1C           - lea edi,[esp+1C]
	AA2Play v12 FP v1.4.0a.exe+EBA66 - 8B D3                 - mov edx,ebx
	AA2Play v12 FP v1.4.0a.exe+EBA68 - 89 5C 24 18           - mov [esp+18],ebx
	AA2Play v12 FP v1.4.0a.exe+EBA6C - E8 2FAA0100           - call "AA2Play v12 FP v1.4.0a.exe"+1064A0 { ->AA2Play v12 FP v1.4.0a.exe+1064A0 }
	*/
	/*DWORD address = General::GameBase + 0xEBA6C;
	DWORD redirectAddress = (DWORD)(&TransferInRedirect);
	Hook((BYTE*)address,
		{ 0xE8, 0x2F, 0xAA, 0x01, 0x00 },							//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		&TransferInOriginalFunc);*/

	//a little bit fater up. relationship data is initialized after the call. esi is character struct
	//2 param stdcall
	/*AA2Play v12 FP v1.4.0a.exe+EBC9C - 56                    - push esi
	AA2Play v12 FP v1.4.0a.exe+EBC9D - 57                    - push edi
	AA2Play v12 FP v1.4.0a.exe+EBC9E - E8 7DFCFFFF           - call "AA2Play v12 FP v1.4.0a.exe"+EB920 { ->AA2Play v12 FP v1.4.0a.exe+EB920 }
	AA2Play v12 FP v1.4.0a.exe+EBCA3 - 8B 56 3C              - mov edx,[esi+3C]
	*/
	DWORD address = General::GameBase + 0xEBC9E;
	DWORD redirectAddress = (DWORD)(&TransferInRedirect);
	Hook((BYTE*)address,
		{ 0xE8, 0x7D, 0xFC, 0xFF, 0xFF },							//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		&TransferInOriginalFunc);
}

DWORD TransferOutOriginalFunc;
void __declspec(naked) TransferOutRedirect() {
	__asm {
		pushad 

		push[esi]
		call TransferOutEvent

		popad

		jmp [TransferOutOriginalFunc]
	}
}

void TransferOutInjection() {
	//edi index in character array, esi is array base; therefor [esi] is character.
	//also, that "failsave". glorious nip coding.
	/*AA2Play v12 FP v1.4.0a.exe+EC3C6 - 8D 3C 85 00000000     - lea edi,[eax*4+00000000]
	AA2Play v12 FP v1.4.0a.exe+EC3CD - 03 F7                 - add esi,edi
	AA2Play v12 FP v1.4.0a.exe+EC3CF - 74 05                 - je "AA2Play v12 FP v1.4.0a.exe"+EC3D6 { ->AA2Play v12 FP v1.4.0a.exe+EC3D6 }
	AA2Play v12 FP v1.4.0a.exe+EC3D1 - E8 3A080000           - call "AA2Play v12 FP v1.4.0a.exe"+ECC10 { ->AA2Play v12 FP v1.4.0a.exe+ECC10 }
	*/
	DWORD address = General::GameBase + 0xEC3D1;
	DWORD redirectAddress = (DWORD)(&TransferOutRedirect);
	Hook((BYTE*)address,
		{ 0xE8, 0x2F, 0xAA, 0x01, 0x00 },							//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			&TransferOutOriginalFunc);
}


//sets selected club when transfered or club is changed by player action. eax is character struct
//AA2Play v12 FP v1.4.0a.exe+DED74 - 88 82 46040000        - mov [edx+00000446],al
//AA2Play v12 FP v1.4.0a.exe+DED7A - E8 21DA0100           - call "AA2Play v12 FP v1.4.0a.exe"+FC7A0 { ->AA2Play v12 FP v1.4.0a.exe+FC7A0 }
//AA2Play v12 FP v1.4.0a.exe+DED7F - 83 C4 1C              - add esp,1C { 28 }



}
}