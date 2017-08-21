#include "StdAfx.h"

using namespace Shared::Triggers;

namespace PlayInjections {
namespace Loads {

// these can force boob gravity and skirt state regardless of clothing state
// you can update these on HiPolyLoadStartEvent, and restore back to 2 on EndEvent
BYTE g_skirtOffOverride = 2; // 0 - have skirt, 1 - noskirt, 2 - original (cloth state dependent)
BYTE g_boobGravityOverride = 2; // 0 - saggy, 1 bra, 2 - original (cloth state dependent)
int g_eyeTracking = -1;


//////////////////////////
namespace {
	int loc_hiPolyLoaded;
}

using namespace ExtClass;

static DWORD OrigLoadMale, OrigLoadFemale;
static DWORD OrigUpdateMale, OrigUpdateFemale;
static DWORD OrigDespawnMale, OrigDespawnFemale;

void HiPolyLoadStartEvent(ExtClass::CharacterStruct* loadCharacter, BYTE cloth, BYTE partial) {
	// Remove once they can cope
	if (!General::IsAAPlay) return;

	Shared::MeshTextureCharLoadStart(loadCharacter);
	Poser::LoadCharacter(loadCharacter);
	//Add the character to the conversation list
	if (Shared::GameState::getIsPcConversation()) {
		Shared::GameState::addConversationCharacter(loadCharacter);
	}
	//throw high poly event
	HiPolyInitData data;
	data.card = AAPlay::GetSeatFromStruct(loadCharacter);
	loc_hiPolyLoaded = data.card;
	ThrowEvent(&data);

}

void HiPolyLoadEndEvent(CharacterStruct *loadCharacter) {
	// Remove once they can cope
	if (!General::IsAAPlay) return;

	Shared::MeshTextureCharLoadEnd();
	Poser::LoadCharacterEnd();
	//throw high poly end event
	HiPolyEndData data;
	data.card = loc_hiPolyLoaded;
	ThrowEvent(&data);
}

// wraps the calls to original load character events
DWORD __stdcall CallOrigLoad(DWORD who, void *_this, BYTE cloth, BYTE a3, BYTE a4, BYTE partial) {
	CharacterStruct *loadCharacter = (CharacterStruct*)_this;

	LUA_EVENT_NORET("char_spawn", loadCharacter, cloth, a3, a4, partial);
	HiPolyLoadStartEvent(loadCharacter, cloth, partial);

	DWORD retv;

	__asm {
		lea eax, [who]
		push dword ptr [eax+20]
		push dword ptr [eax+16]
		push dword ptr [eax+12]
		push dword ptr [eax+8]
		mov ecx, [eax+4]
		call dword ptr [eax]
		mov retv, eax
	}

	LUA_EVENT_NORET("char_spawn_end", retv, loadCharacter, cloth, a3, a4, partial);
	HiPolyLoadEndEvent(loadCharacter);
	return retv;
}

// wraps the calls to original load character events
DWORD __stdcall CallOrigUpdate(DWORD who, void *_this, BYTE a, BYTE b) {
	CharacterStruct *loadCharacter = (CharacterStruct*)_this;

	LUA_EVENT_NORET("char_update", loadCharacter, a, b);

	HiPolyLoadStartEvent(loadCharacter, a, b);

	DWORD retv;

	__asm {
		lea eax, [who]
		push dword ptr[eax + 12]
		push dword ptr[eax + 8]
		mov ecx, [eax + 4]
		call dword ptr[eax]
		mov retv, eax
	}

	HiPolyLoadEndEvent(loadCharacter);
	LUA_EVENT_NORET("char_update_end", loadCharacter, retv, a, b);

	return retv;
}

DWORD __stdcall CallOrigDespawn(DWORD who, void *_this) {
	CharacterStruct *loadCharacter = (CharacterStruct*)_this;

	LUA_EVENT_NORET("char_despawn", loadCharacter);

	DWORD retv;

	__asm {
		lea eax, [who]
		mov ecx, [eax + 4]
		call dword ptr[eax]
		mov retv, eax
	}

	LUA_EVENT_NORET("char_despawn_after", loadCharacter, retv);

	return retv;
}


void __declspec(naked) QueryBoobGravity() {
	__asm {
		mov     ebx, [esi+0F84h]
		mov cl, g_boobGravityOverride
		test cl, 2
		jnz no_override
		mov al, cl
no_override:
		ret
	}
}

void __declspec(naked) QuerySkirt() {
	__asm {
		mov cl, g_skirtOffOverride
		test cl, 2
		jnz no_override
		ret
no_override:
		test dl, dl
		setz cl
		ret
	}
}

void __declspec(naked) QueryEye() {
	__asm {
		cmp g_eyeTracking, -1
		je skip
		mov ebx, g_eyeTracking
skip:
		cmp [esi+100Eh], bx
		jnz skip2
		cmp byte ptr[ebp+16], 0
skip2:  ret
	}
}

class HiPolyLoader {
public:;
virtual BYTE LoadMale(BYTE a2, BYTE a3, BYTE a4, BYTE a5) { return CallOrigLoad(OrigLoadMale, this, a2, a3, a4, a5); }
virtual BYTE LoadFemale(BYTE a2, BYTE a3, BYTE a4, BYTE a5) { return CallOrigLoad(OrigLoadFemale, this, a2, a3, a4, a5); }
virtual BYTE UpdateMale(BYTE a2, BYTE a3) { return CallOrigUpdate(OrigUpdateMale, this, a2, a3); }
virtual BYTE UpdateFemale(BYTE a2, BYTE a3) { return CallOrigUpdate(OrigUpdateFemale, this, a2, a3); }
virtual BYTE DespawnMale() { return CallOrigDespawn(OrigDespawnMale, this); }
virtual BYTE DespawnFemale() { return CallOrigDespawn(OrigDespawnFemale, this); }
};

void HiPolyLoadsInjection() {

	DWORD female, male, boobs, skirt, eye;
	if (General::IsAAPlay) {
		female =0x32D260;
		male =  0x32CD80;
		boobs = 0x12DA8B;
		skirt = 0x1132B1;
	}
	else if (General::IsAAEdit) {
		female =0x30C328;
		male =  0x30BE48;
		boobs = 0x11BF7B;
		skirt = 0x101C31;
		eye =   0x1ADFA9;
	}

	// dummy vtable to force stdcall
	HiPolyLoader dummy;
	auto fns = *((void***)(&dummy));

	female += General::GameBase;
	male += General::GameBase;
	boobs += General::GameBase;
	skirt += General::GameBase;
	eye += General::GameBase;

	// patch the vtable, save original pointers

	// #0 unused
	// #1
	OrigLoadMale = PatchIAT((void*)male, fns[0]);
	OrigLoadFemale = PatchIAT((void*)female, fns[1]);

	// #2
	OrigUpdateMale = PatchIAT((void*)(male+4), fns[2]);
	OrigUpdateFemale = PatchIAT((void*)(female+4), fns[3]);

	// #3 unused
	// #4
	OrigDespawnMale = PatchIAT((void*)(male + 12), fns[4]);
	OrigDespawnFemale = PatchIAT((void*)(female + 12), fns[5]);

	// #5 and more unused

	// inject calls in place of skirt/boob queries
	Hook((BYTE*)boobs,
	{ 0x8B, 0x9E, 0x84, 0x0F, 0x00, 0x00 },							//expected values
	{ 0x90, 0xE8, HookControl::RELATIVE_DWORD, (DWORD)&QueryBoobGravity},	//redirect to our function
		NULL);

	Hook((BYTE*)skirt,
	{ 0x84, 0xd2, 0x0F, 0x94, 0xC1 },							//expected values
	{ 0xE8, HookControl::RELATIVE_DWORD, (DWORD)&QuerySkirt },	//redirect to our function
		NULL);

	Hook((BYTE*)eye,
	{ 0x75, 0x06, 0x80, 0x7D, 0x10, 0x00 },							//expected values
	{ 0x90, 0xe8, HookControl::RELATIVE_DWORD, (DWORD)&QueryEye },	//redirect to our function
		NULL);

}

void __stdcall SaveLoadEvent() {
	AAPlay::InitOnLoad();
//	Facecam::Cleanup();
}

void __stdcall TransferInEvent(ExtClass::CharacterStruct* character) {
	AAPlay::InitTransferedCharacter(character);
}

void __stdcall TransferOutEvent(ExtClass::CharacterStruct* character) {
	AAPlay::RemoveTransferedCharacter(character);
}

#if 0
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
#endif

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
		{ 0xE8, HookControl::ANY_DWORD },							//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			&TransferOutOriginalFunc);
}


//sets selected club when transfered or club is changed by player action. eax is character struct
//AA2Play v12 FP v1.4.0a.exe+DED74 - 88 82 46040000        - mov [edx+00000446],al
//AA2Play v12 FP v1.4.0a.exe+DED7A - E8 21DA0100           - call "AA2Play v12 FP v1.4.0a.exe"+FC7A0 { ->AA2Play v12 FP v1.4.0a.exe+FC7A0 }
//AA2Play v12 FP v1.4.0a.exe+DED7F - 83 C4 1C              - add esp,1C { 28 }



}
}