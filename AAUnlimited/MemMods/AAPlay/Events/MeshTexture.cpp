#include "MeshTexture.h"

#include "MemMods\Hook.h"
#include "General\ModuleInfo.h"
#include "Functions\AAPlay\Overrides.h"

/*
 * Again, mostly a copy from the AAEdit version, with slight adjustments to the values and registers.
 * look there for more info.
 */

namespace PlayInjections {
namespace MeshTexture {
void __declspec(naked) OverrideTextureListSizeRedirect() {
	__asm {
		//original code
		mov eax, [esi]

		push eax //rescue general purpose registers
		push ecx

		push eax
		push edi
		call AAPlay::MeshTextureListStart
		mov edx, eax
		test eax, eax
		pop ecx
		pop eax
		je OverrideTextureListSizeRedirect_NormalSize
		ret
	OverrideTextureListSizeRedirect_NormalSize :
		mov edx, [eax + edi]
		ret
	}
}

void OverrideTextureListSizeInject() {
	/*AA2Play v12 FP v1.4.0a.exe+206C80 - 8B 06                 - mov eax,[esi] { start of mesh texture loop }
	AA2Play v12 FP v1.4.0a.exe+206C82 - 8B 14 38              - mov edx,[eax+edi]
	AA2Play v12 FP v1.4.0a.exe+206C85 - 03 C7                 - add eax,edi
	AA2Play v12 FP v1.4.0a.exe+206C87 - 89 11                 - mov [ecx],edx
	*/
	//edi is xx file buffer, [esi] is pointer to current offset, [edx+edi] reads string length
	DWORD address = General::GameBase + 0x206C80;
	DWORD redirectAddress = (DWORD)(&OverrideTextureListSizeRedirect);
	Hook((BYTE*)address,
		{ 0x8B, 0x06, 0x8B, 0x14, 0x38 },						//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		NULL);
}

DWORD OverrideTextureListNameOriginal;
DWORD OverrideTextureListNameSkipExit;
void __declspec(naked) OverrideTextureListNameRedirect() {
	__asm {
		pushad
		push esi
		push ebp
		call AAPlay::MeshTextureListFill
		test eax, eax
		popad
		jnz OverrideTextureListNameRedirect_Skip
		jmp[OverrideTextureListNameOriginal] //original function call
	OverrideTextureListNameRedirect_Skip:
		mov eax, [OverrideTextureListNameSkipExit]
		mov dword ptr [esp], eax
		ret 0xC //to get rid of the parameters to the memcpy we are replacing (including original return
	}
}

void OverrideTextureListNameInject() {
	/*AA2Play v12 FP v1.4.0a.exe+206CE8 - 51                    - push ecx		<-- size of string
	AA2Play v12 FP v1.4.0a.exe+206CE9 - 03 D7                 - add edx,edi
	AA2Play v12 FP v1.4.0a.exe+206CEB - 52                    - push edx		<-- src in xx file
	AA2Play v12 FP v1.4.0a.exe+206CEC - 55                    - push ebp		<-- destination buffer
	AA2Play v12 FP v1.4.0a.exe+206CED - E8 4E880800           - call "AA2Play v12 FP v1.4.0a.exe"+28F540 { ->AA2Play v12 FP v1.4.0a.exe+28F540 }
	AA2Play v12 FP v1.4.0a.exe+206CF2 - 8B 4C 24 18           - mov ecx,[esp+18]
	AA2Play v12 FP v1.4.0a.exe+206CF6 - 83 C4 0C              - add esp,0C { 12 }
	AA2Play v12 FP v1.4.0a.exe+206CF9 - 8B C5                 - mov eax,ebp
	AA2Play v12 FP v1.4.0a.exe+206CFB - 85 C9                 - test ecx,ecx
	AA2Play v12 FP v1.4.0a.exe+206CFD - 76 11                 - jna "AA2Play v12 FP v1.4.0a.exe"+206D10 { ->AA2Play v12 FP v1.4.0a.exe+206D10 }
	AA2Play v12 FP v1.4.0a.exe+206CFF - 90                    - nop 
	AA2Play v12 FP v1.4.0a.exe+206D00 - 8A 10                 - mov dl,[eax]
	AA2Play v12 FP v1.4.0a.exe+206D02 - F6 D2                 - not dl
	AA2Play v12 FP v1.4.0a.exe+206D04 - 88 10                 - mov [eax],dl
	AA2Play v12 FP v1.4.0a.exe+206D06 - 40                    - inc eax
	AA2Play v12 FP v1.4.0a.exe+206D07 - 83 E9 01              - sub ecx,01 { 1 }
	AA2Play v12 FP v1.4.0a.exe+206D0A - 75 F4                 - jne "AA2Play v12 FP v1.4.0a.exe"+206D00 { ->AA2Play v12 FP v1.4.0a.exe+206D00 }
	AA2Play v12 FP v1.4.0a.exe+206D0C - 8B 4C 24 0C           - mov ecx,[esp+0C]
	AA2Play v12 FP v1.4.0a.exe+206D10 - 01 0E                 - add [esi],ecx*/
	//this part is memcpying the name from the buffer, then decrypts it
	DWORD address = General::GameBase + 0x206CED;
	DWORD redirectAddress = (DWORD)(&OverrideTextureListNameRedirect);
	Hook((BYTE*)address,
		{ 0xE8, 0x4E, 0x88, 0x08, 0x00 },						//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		&OverrideTextureListNameOriginal);
	OverrideTextureListNameSkipExit = General::GameBase + 0x206D10;
}
}
}



namespace PlayInjections {
namespace MeshTexture {

/*
* for step 1: start of the loop
*/
void __declspec(naked) LoadLoopRedirect() {
	_asm {
		pushad
		push[ebp] //pointer to current offset
		mov eax, [esp + 0x20 + 0x14] //xx file buffer
		push eax
		call AAPlay::MeshTextureStart //stdcall
		popad
		add ecx, 0x1408 //original code
		ret
	}
}

void OverrideStartInject() {
	//at this point, [esp+14] is the xx file buffer, [ebp] is the current offset
	/*AA2Play v12 FP v1.4.0a.exe+2071A0 - 8B 4C 24 18           - mov ecx,[esp+18]
	AA2Play v12 FP v1.4.0a.exe+2071A4 - 33 C0                 - xor eax,eax
	AA2Play v12 FP v1.4.0a.exe+2071A6 - 81 C1 08140000        - add ecx,00001408 { 5128 }
	*/
	DWORD address = General::GameBase + 0x2071A6;
	DWORD redirectAddress = (DWORD)(&LoadLoopRedirect);
	Hook((BYTE*)address,
	{ 0x81, 0xC1, 0x08, 0x14, 00,00 },						//expected values
	{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress,0x90 },	//redirect to our function
		NULL);
}

/*
* for step 2: we need to skip this while part that copys the name and insert our own
* if step 1 said that we want to override
*/
DWORD OverrideNameCustomExit;
DWORD OverrideNameDefaultExit;
void _declspec(naked) OverrideNameRedirect() {
	_asm {
		pushad
		push ebp //pointer to offset
		push esi //destination buffer
		call AAPlay::MeshTextureOverrideName
		test eax, eax
		popad
		jz OverrideNameRedirect_Original
		jmp[OverrideNameCustomExit]
		OverrideNameRedirect_Original:
		//orignal code
		push ebx
		add eax, edi
		push eax
		push esi
		jmp[OverrideNameDefaultExit]
	}
}

void OverrideNameInject() {
	/*AA2Play v12 FP v1.4.0a.exe+207227 - 8B 45 00              - mov eax,[ebp+00]
	AA2Play v12 FP v1.4.0a.exe+20722A - 53                    - push ebx { size }
	AA2Play v12 FP v1.4.0a.exe+20722B - 03 C7                 - add eax,edi
	AA2Play v12 FP v1.4.0a.exe+20722D - 50                    - push eax { cpy source (xx file) }
	AA2Play v12 FP v1.4.0a.exe+20722E - 56                    - push esi { copy dest (name on heap) }
	AA2Play v12 FP v1.4.0a.exe+20722F - E8 0C830800           - call "AA2Play v12 FP v1.4.0a.exe"+28F540 { ->AA2Play v12 FP v1.4.0a.exe+28F540 }
	AA2Play v12 FP v1.4.0a.exe+207234 - 83 C4 0C              - add esp,0C { 12 }
	AA2Play v12 FP v1.4.0a.exe+207237 - 8B C6                 - mov eax,esi
	AA2Play v12 FP v1.4.0a.exe+207239 - 85 DB                 - test ebx,ebx
	AA2Play v12 FP v1.4.0a.exe+20723B - 76 0F                 - jna "AA2Play v12 FP v1.4.0a.exe"+20724C { ->AA2Play v12 FP v1.4.0a.exe+20724C }
	AA2Play v12 FP v1.4.0a.exe+20723D - 8B CB                 - mov ecx,ebx
	AA2Play v12 FP v1.4.0a.exe+20723F - 90                    - nop 
	AA2Play v12 FP v1.4.0a.exe+207240 - 8A 10                 - mov dl,[eax]
	AA2Play v12 FP v1.4.0a.exe+207242 - F6 D2                 - not dl
	AA2Play v12 FP v1.4.0a.exe+207244 - 88 10                 - mov [eax],dl
	AA2Play v12 FP v1.4.0a.exe+207246 - 40                    - inc eax
	AA2Play v12 FP v1.4.0a.exe+207247 - 83 E9 01              - sub ecx,01 { 1 }
	AA2Play v12 FP v1.4.0a.exe+20724A - 75 F4                 - jne "AA2Play v12 FP v1.4.0a.exe"+207240 { ->AA2Play v12 FP v1.4.0a.exe+207240 }
	AA2Play v12 FP v1.4.0a.exe+20724C - 01 5D 00              - add [ebp+00],ebx
	*/
	DWORD address = General::GameBase + 0x20722A;
	DWORD redirectAddress = (DWORD)(&OverrideNameRedirect);
	Hook((BYTE*)address,
		{ 0x53,
		0x03, 0xC7,
		0x50,
		0x56, },
		{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		NULL);

	OverrideNameDefaultExit = General::GameBase + 0x20722F;
	OverrideNameCustomExit = General::GameBase + 0x20724C;
}

/*
* for step 3: adjust meta data
*/
void __declspec(naked) OverrideFileSizeRedirect() {
	_asm {
		pushad
		mov eax, [ebp]
		push eax
		push ecx
		call AAPlay::MeshTextureOverrideSize
		popad
		//original stuff that we replaced
		mov[edx + eax * 4 + 0x00001408], esi
		ret
	}
}

void OverrideFileSizeInject() {
	//gotta make sure that we change the meta-information of the image in the buffer
	/* AA2Play v12 FP v1.4.0a.exe + 207426 - 8B 54 24 18 - mov edx, [esp + 18]
	AA2Play v12 FP v1.4.0a.exe + 20742A - 8B 44 24 2C - mov eax, [esp + 2C]
	AA2Play v12 FP v1.4.0a.exe + 20742E - 8B 4C 24 14 - mov ecx, [esp + 14] { xxFile }
	AA2Play v12 FP v1.4.0a.exe + 207432 - 89 B4 82 08140000 - mov[edx + eax * 4 + 00001408], esi
	AA2Play v12 FP v1.4.0a.exe + 207439 - 89 1E - mov[esi], ebx
	AA2Play v12 FP v1.4.0a.exe + 20743B - 89 7E 04 - mov[esi + 04], edi
	AA2Play v12 FP v1.4.0a.exe + 20743E - 8B 55 00 - mov edx, [ebp + 00]
	AA2Play v12 FP v1.4.0a.exe + 207441 - 8B 04 0A - mov eax, [edx + ecx]
	*/
	DWORD address = General::GameBase + 0x207432;
	DWORD redirectAddress = (DWORD)(&OverrideFileSizeRedirect);
	Hook((BYTE*)address,
	{ 0x89, 0xB4, 0x82, 0x08, 0x14,00,00 },
	{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress, 0x90, 0x90 },	//redirect to our function
		NULL);
}


/*
* for step 4:
*/
DWORD OverrideFileOriginalCall;
void __declspec(naked) OverrideFileRedirect() {
	_asm {
		pushad
		push ebp //offset pointer
		push eax
		call AAPlay::MeshTextureOverrideFile
		test eax, eax
		popad
		jz OverrideFile_Original //if function returned false, do normal call
		ret //else, we did it allready, so just return
	OverrideFile_Original :
		jmp[OverrideFileOriginalCall]
	}
}

void OverrideFileInject() {
	//this function is the memcpy(buffer, file, filesize). the parameters are
	//removed way down there. cdecl n stuff.
	/*AA2Play v12 FP v1.4.0a.exe+2074E0 - 03 54 24 14           - add edx,[esp+14]
	AA2Play v12 FP v1.4.0a.exe+2074E4 - 57                    - push edi
	AA2Play v12 FP v1.4.0a.exe+2074E5 - 52                    - push edx
	AA2Play v12 FP v1.4.0a.exe+2074E6 - 50                    - push eax
	AA2Play v12 FP v1.4.0a.exe+2074E7 - E8 54800800           - call "AA2Play v12 FP v1.4.0a.exe"+28F540 { ->AA2Play v12 FP v1.4.0a.exe+28F540 }
	AA2Play v12 FP v1.4.0a.exe+2074EC - 8B 4E 04              - mov ecx,[esi+04]
	AA2Play v12 FP v1.4.0a.exe+2074EF - 83 C4 0C              - add esp,0C { 12 }
	*/
	DWORD address = General::GameBase + 0x2074E7;
	DWORD redirectAddress = (DWORD)(&OverrideFileRedirect);
	Hook((BYTE*)address,
		{ 0xE8, 0x54, 0x80, 0x08, 0x00, },
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		&OverrideFileOriginalCall);
}


}
}