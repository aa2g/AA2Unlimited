#include "StdAfx.h"
#include "Files/PNGData.h"

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

int hairAddress;
static DWORD OrigLoadMale, OrigLoadFemale;
static DWORD OrigUpdateMale, OrigUpdateFemale;
static DWORD OrigDespawnMale, OrigDespawnFemale;
static DWORD OrigLoadXAMale, OrigLoadXAFemale;

bool loc_loadingCharacter = false;
void HiPolyLoadStartEvent(ExtClass::CharacterStruct* loadCharacter, DWORD &cloth, BYTE partial) {
	// Remove once they can cope
	if (!General::IsAAPlay) return;



	Shared::MeshTextureCharLoadStart(loadCharacter);
	//Add the character to the conversation list
	if (Shared::GameState::getIsPcConversation()) {
		Shared::GameState::addConversationCharacter(loadCharacter);
	}	

	//Add the character to conversation list in h
	if (General::IsAAPlay) {
		const DWORD offset4[]{ 0x3761CC, 0x28, 0x28 };
		BYTE* HSceneTrigger = (BYTE*)ExtVars::ApplyRule(offset4);

		if (*HSceneTrigger == 1) {
			Shared::GameState::addConversationCharacter(loadCharacter);
			Shared::GameState::setIsInH(true);
		}
	}
	//throw high poly event
	HiPolyInitData data;
	data.character = loadCharacter;
	data.clothState = &cloth;
	data.card = AAPlay::GetSeatFromStruct(loadCharacter);
	loc_hiPolyLoaded = data.card;
	ThrowEvent(&data);
}

void HiPolyLoadEndEvent(CharacterStruct *loadCharacter) {
	// Remove once they can cope
	if (!General::IsAAPlay) return;

	Shared::MeshTextureCharLoadEnd();
	//throw high poly end event
	HiPolyEndData data;
	data.card = loc_hiPolyLoaded;
	ThrowEvent(&data);
}

// wraps the calls to original load character events
DWORD __declspec(noinline) __stdcall CallOrigLoad(DWORD who, void *_this, DWORD cloth, DWORD a3, DWORD a4, DWORD partial) {
	loc_loadingCharacter = true;
	CharacterStruct *loadCharacter = (CharacterStruct*)_this;
	Poser::LoadCharacter(loadCharacter);

	LUA_EVENT_NORET("char_spawn", loadCharacter, cloth, a3, a4, partial);
	// Extra Hairs low poly infection fix
	// Maker loads hair twice. Once after character is loaded. Fix can be safely ignored in Maker
	if (General::IsAAPlay)
		Shared::GameState::setIsOverriding(true);
	HiPolyLoadStartEvent(loadCharacter, cloth, partial);

	DWORD retv;

	__asm {
		/*		lea eax, [who]
				push dword ptr [eax+20]
				push dword ptr [eax+16]
				push dword ptr [eax+12]
				push dword ptr [eax+8]
				mov ecx, [eax+4]
				call dword ptr [eax]*/
		push partial
		push a4
		push a3
		push cloth
		mov ecx, _this
		call dword ptr[who]
		mov retv, eax
	}

	LUA_EVENT_NORET("char_spawn_end", retv, loadCharacter, cloth, a3, a4, partial);
	if (General::IsAAPlay)
		Shared::GameState::setIsOverriding(false);
	HiPolyLoadEndEvent(loadCharacter);
	loc_loadingCharacter = false;
	return retv;
}

// wraps the calls to original load character events
DWORD __declspec(noinline) __stdcall CallOrigUpdate(DWORD who, void *_this, DWORD a, DWORD b) {
	CharacterStruct *loadCharacter = (CharacterStruct*)_this;
	Poser::UpdateCharacter(loadCharacter);

	LUA_EVENT_NORET("char_update", loadCharacter, a, b);
	// Extra Hairs low poly infection fix
	// Maker loads hair twice. Once after character is loaded. Fix can be safely ignored in Maker
	if (General::IsAAPlay)
		Shared::GameState::setIsOverriding(true);

	HiPolyLoadStartEvent(loadCharacter, a, b);

	DWORD retv;

	__asm {
		/*		lea eax, [who]
				push dword ptr[eax + 12]
				push dword ptr[eax + 8]
				mov ecx, [eax + 4]
				call dword ptr[eax]*/
		push b
		push a
		mov ecx, _this
		call dword ptr[who]
		mov retv, eax
	}

	HiPolyLoadEndEvent(loadCharacter);
	LUA_EVENT_NORET("char_update_end", retv, loadCharacter, a, b);
	if (General::IsAAPlay)
		Shared::GameState::setIsOverriding(false);

	return retv;
}

DWORD __declspec(noinline) __stdcall CallOrigDespawn(DWORD who, void *_this) {
	CharacterStruct *loadCharacter = (CharacterStruct*)_this;

	if (General::IsAAPlay) {
		const DWORD offset4[]{ 0x3761CC, 0x28, 0x28 };
		BYTE* HSceneTrigger = (BYTE*)ExtVars::ApplyRule(offset4);
		if (HSceneTrigger != nullptr) {
			if (*HSceneTrigger == 0 && Shared::GameState::getIsInH()) {
				Shared::GameState::clearConversationCharacter(-1);
				Shared::GameState::setIsInH(false);
			}
		}
	}

	if (!loc_loadingCharacter) {
		if (General::IsAAPlay) {
			HiPolyDespawnData data;
			data.card = AAPlay::GetSeatFromStruct(loadCharacter);
			ThrowEvent(&data);
		}
		LUA_EVENT_NORET("char_despawn", loadCharacter);
	}


	DWORD retv;

	__asm {
/*		lea eax, [who]
		mov ecx, [eax + 4]
		call dword ptr[eax]*/
		mov ecx, _this
		call dword ptr[who]
		mov retv, eax
	}

	if (!loc_loadingCharacter) {
		LUA_EVENT_NORET("char_despawn_after", retv, loadCharacter);
		Poser::RemoveCharacter(loadCharacter);
	}

	return retv;
}

DWORD  __declspec(noinline) __stdcall CallOrigLoadXA(DWORD who, void *_this, wchar_t *pp, wchar_t *xa, DWORD pose, DWORD z0, DWORD z1) {
	CharacterStruct *loadCharacter = (CharacterStruct*)_this;
	std::string pp_utf8(General::utf8.to_bytes(pp));
	std::string xa_utf8(General::utf8.to_bytes(xa));

	LUA_EVENT_NORET("char_xa", loadCharacter, pp_utf8.c_str(), xa_utf8.c_str(), pose, z0, z1);

	DWORD retv;
	__asm {
/*		lea eax, [who]
		push dword ptr[eax + 24]
		push dword ptr[eax + 20]
		push dword ptr[eax + 16]
		push dword ptr[eax + 12]
		push dword ptr[eax + 8]
		mov ecx, [eax + 4]*/
		push z1
		push z0
		push pose
		push xa
		push pp
		mov ecx, _this
		call dword ptr [who]
		mov retv, eax
	}
	LUA_EVENT_NORET("char_xa_end", loadCharacter, pp_utf8.c_str(), xa_utf8.c_str(), pose, z0, z1);

	return retv;
}


BYTE g_invisibraOverride[256] = { 0 };
BYTE loc_invisibraOverride;
CharacterStruct *loc_character;

void setInvisibra() {
	loc_invisibraOverride =
		g_boobGravityOverride != 2 ? g_boobGravityOverride :
		loc_character->m_clothState && ((1 << (loc_character->m_clothState-1)) & g_invisibraOverride[loc_character->m_currClothSlot]) ? 0 : 2;
}

void __declspec(naked) QueryBoobGravity() {
	__asm {
		pushad
		mov loc_character, esi
		call setInvisibra
		popad
		mov ebx, [esi+0F84h]
		mov cl, loc_invisibraOverride
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

virtual BYTE LoadXAMale(wchar_t *pp, wchar_t *xa, DWORD pose, DWORD z0, DWORD z1) { return CallOrigLoadXA(OrigLoadXAMale, this, pp,xa,pose,z0,z1); }
virtual BYTE __thiscall LoadXAFemale(wchar_t *pp, wchar_t *xa, DWORD pose, DWORD z0, DWORD z1) { return CallOrigLoadXA(OrigLoadXAFemale, this, pp, xa, pose, z0, z1); }
};

void HiPolyLoadsInjection() {

	DWORD female, male, boobs, skirt, eye;
	if (General::IsAAPlay) {
		female =0x32D260;
		male =  0x32CD80;
		boobs = 0x12DA8B;
		skirt = 0x1132B1;
		eye = 0x1CB809;
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
#define GET_VT(i,n,j) \
	Orig ## n ## Male = PatchIAT((void*)(male+(i-1)*4), fns[j*2]); \
	Orig ## n ## Female = PatchIAT((void*)(female + (i - 1) * 4), fns[j * 2 + 1]);

	GET_VT(1, Load, 0);
	GET_VT(2, Update, 1);
	GET_VT(4, Despawn, 2);
	GET_VT(9, LoadXA, 3);
#undef GET_VT

/*
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

	// #6
	// #7
	// #8 unused
*/

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

	if (General::IsAAEdit) {
		// Spawn clothed on preview window by default
		Hook((BYTE*)(General::GameBase + 0x1A483), { 0x6A, 0x00 }, { 0x6A, 0x01 }, NULL);
	}
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


void __stdcall hairUpdate(int hairTab, int hairSlot) {
	//Replace infected hair slot of the currently open hair tab with one in style
	if (Shared::preservingFrontHairSlot != -1) {
		int value = hairSlot;
		if (hairTab == 0) {
			value = Shared::preservingFrontHairSlot;
		}
		else if (hairTab == 1) {
			value = Shared::preservingSideHairSlot;
		}
		else if (hairTab == 2) {
			value = Shared::preservingBackHairSlot;
		}
		else if (hairTab == 3) {
			value = Shared::preservingExtHairSlot;
		}
		hairAddress = value;
		Shared::preservingFrontHairSlot = -1;
		Shared::preservingSideHairSlot = -1;
		Shared::preservingBackHairSlot = -1;
		Shared::preservingExtHairSlot = -1;
	}
}

void __declspec(naked) hairUpdateRedirect() {
	__asm {
		mov hairAddress, eax
		pushad
		push al
		//al is the hair slot
		push edx
		//edx is 0-3 whether it's front hair, side hair...
		call hairUpdate
		//original code
		popad
		mov eax, hairAddress
		mov[edx + ebx + 0x69C], al
		ret
	}
}


void hairUpdateInject() {
	//eax is the hair slot
	//AA2Edit.exe + 280E6 - 88 84 1A 9C060000 - mov[edx + ebx + 0000069C], al


	DWORD address = General::GameBase + 0x280E6;
	DWORD redirectAddress = (DWORD)(&hairUpdateRedirect);
	Hook((BYTE*)address,
	{ 0x88, 0x84, 0x1A, 0x9C, 0x06, 0x00, 0x00 },					//expected values
	{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress, 0x90, 0x90 },	//redirect to our function
		NULL);
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
