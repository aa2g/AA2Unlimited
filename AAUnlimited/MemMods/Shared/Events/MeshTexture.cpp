#include "MeshTexture.h"

#include <d3d9.h>

#include "MemMods\Hook.h"
#include "General\ModuleInfo.h"
#include "Functions\Shared\Globals.h"
#include "Functions\Shared\Overrides.h"
#include "Functions\Shared\SpecialOverrides.h"
#include "Functions\AAPlay\Poser.h"
#include "Files\XXFile.h"
#include "Functions\AAPlay\GameState.h"

/*
* xx file textures are loaded in two steps. First, an array of mesh structs is created, each of which
* contain some unknown data as well as the names of the textures (up to 4).
* after that, in a second function, the texture list from the xx file is loaded into a global cache.
* after that is done, still in the same function, the mesh structs are iterated through, assigning
* a texture from the cache to each of theses names.
* we will have to catch both the name list and the texture read part.
*/

namespace SharedInjections {
namespace MeshTexture {

const TextureImage* MeshTextureEvent(wchar_t* fileName) {
	const TextureImage* ret = NULL;
	ret = Shared::MeshTextureOverrideRules(fileName);
	if (ret != NULL) return ret;
	ret = Shared::HairHighlightOverride(fileName);
	if (ret != NULL) return ret;
	return NULL;
}

namespace {
	bool loc_currentlyOverriding = false;
	const TextureImage* loc_match = NULL;
	DWORD loc_oldNameLength;
	DWORD loc_oldFileLength;
	HANDLE loc_matchFile;
}

/*
* List Step 1: Determine if we want to override this file, replace size if we want.
* string is saved as DWORD size, then BYTE[size], where the array is not-ed with itself.
* Function returns new size, or 0 if size shouldnt be changed
*/
DWORD __stdcall MeshTextureListStart(BYTE* xxFileBuffer, DWORD offset) {
	loc_currentlyOverriding = false;

	BYTE* file = xxFileBuffer + offset;
	DWORD size = *(DWORD*)(file);
	file += 4;

	for (DWORD i = 0; i < size; i++) file[i] = ~file[i];
	{
		size_t out;
		TCHAR nameBuffer[256];
		mbstowcs_s(&out, nameBuffer, (char*)file, 256);
		loc_match = MeshTextureEvent(nameBuffer);
	}
	for (DWORD i = 0; i < size; i++) file[i] = ~file[i];

	if (loc_match != NULL) {
		loc_currentlyOverriding = true;
		loc_oldNameLength = size;
		return loc_match->GetFileName().size() + 1;
	}
	else {
		return 0;
	}
}

/*
* List Step 2: if name is overriden, insert new name into buffer instead.
* also, adjust iterator so we dont shoot past the file.
*/
bool __stdcall MeshTextureListFill(BYTE* name, DWORD* xxReadOffset) {
	if (!loc_currentlyOverriding) return false;
	const std::wstring& matchname = loc_match->GetFileName();

	size_t out;
	wcstombs_s(&out, (char*)name, matchname.size() + 1, matchname.c_str(), matchname.size());

	DWORD difference = loc_oldNameLength - (matchname.size() + 1); //note that it incudes \0
	*xxReadOffset += difference;
	return true;
}


/*
* Step 1. Determine if we want to override this file, replace size member if we want.
* reminder:  //struct filelist {
//  DWORD nFiles;
//  struct {
//   DWORD nameLength; //including \0
//   char name[nameLength]; //"encrypted", the name is not-ed
//	  DWORD unknown;
//	  DWORD width;
//	  DWORD height;
//	  BYTE unknown[0x21-12]
//	  DWORD filelength;
//	  BYTE file[fileLength]; //the magic number of BMs might be 0
//  } files[nFiles]
//}
*/
void __stdcall MeshTextureStart(BYTE* xxFile, DWORD offset) {
	loc_currentlyOverriding = false;

	BYTE* file = xxFile + offset;
	DWORD* ptrNameLength = (DWORD*)(file);
	DWORD nameLength = *ptrNameLength;
	file += 4;
	//"decrypt" the string name in place so we can compare better
	for (DWORD i = 0; i < nameLength; i++) file[i] = ~file[i];
	{
		size_t out;
		TCHAR nameBuffer[256];
		mbstowcs_s(&out, nameBuffer, (char*)file, 256);
		loc_match = MeshTextureEvent(nameBuffer);
	}
	//remember to encrypt again
	for (DWORD i = 0; i < nameLength; i++) file[i] = ~file[i];

	//if we found a file to override with, change length (thats our job here after all)
	if (loc_match != NULL) {
		const std::wstring& matchPath = loc_match->GetFileName();
		loc_matchFile = CreateFile(loc_match->GetFilePath().c_str(), FILE_READ_ACCESS, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (loc_matchFile != NULL && loc_matchFile != INVALID_HANDLE_VALUE) {
			//successfully opened file
			loc_currentlyOverriding = true;
			//manipulate name length
			loc_oldNameLength = *ptrNameLength;
			*ptrNameLength = matchPath.size() + 1; //plus nullterminator
		}


	}
}

/*
* Step 2. If we want to replace the name, fill it here and return true.
* else, return false.
* ALSO, we need to adjust the xxReadOffset. because we changed the nameLength,
* he will do offset += nameLength, which is wrong now (it should be offset += oldNameLength).
* therefor, we will need to add (oldNameLength - nameLength)
*/
bool __stdcall MeshTextureOverrideName(BYTE* buffer, DWORD* xxReadOffset) {
	if (!loc_currentlyOverriding) return false;
	//we cant replace the name. it will erase it later for some reason, or not find the texture.
	const std::wstring& matchPath = loc_match->GetFileName();
	size_t out;
	wcstombs_s(&out, (char*)buffer, matchPath.size() + 1, matchPath.c_str(), matchPath.size());

	//also, adjust his current offset
	DWORD difference = loc_oldNameLength - (matchPath.size() + 1); //note that it incudes \0
	*xxReadOffset += difference;
	return true;
}

/*
* Step 3 . change file size and dimensions. offset points to first unknown data
*/
void __stdcall MeshTextureOverrideSize(BYTE* buffer, DWORD offset) {
	if (!loc_currentlyOverriding) return;
	//set width and height
	BYTE* file = buffer + offset + 4; //skip to width
	*(DWORD*)(file) = loc_match->GetWidth();
	file += 4;
	*(DWORD*)(file) = loc_match->GetHeight();
	file += 4 + 0x15;
	loc_oldFileLength = *(DWORD*)(file);
	*(DWORD*)(file) = loc_match->GetFileSize();
}

/*
* Step 4. Fill the buffer with the selected image file.
*/

bool __stdcall MeshTextureOverrideFile(BYTE* buffer, DWORD* xxReadOffset) {
	if (!loc_currentlyOverriding) return false;
	//just copy it all. the class has a method for that.
	loc_match->WriteToBuffer(buffer);
	//remember to change the xxFileOffsetm like above
	DWORD difference = loc_oldFileLength - loc_match->GetFileSize();
	*xxReadOffset += difference;
	return true;
}


void __declspec(naked) OverrideTextureListSizeRedirect() {
	__asm {
		//original code
		mov eax, [esi]

		push eax //rescue general purpose registers
		push ecx

		push eax
		push edi
		call MeshTextureListStart
		mov edx, eax
		pop ecx
		pop eax
		test edx, edx
		je OverrideTextureListSizeRedirect_NormalSize
		ret
	OverrideTextureListSizeRedirect_NormalSize :
		mov edx, [eax + edi]
		ret
	}
}


void OverrideTextureListSizeInject() {
	if (General::IsAAEdit) {
		/*AA2Edit.exe+1E9880 - 8B 06                 - mov eax,[esi] { start of mesh texture read loop }
		AA2Edit.exe+1E9882 - 8B 14 38              - mov edx,[eax+edi]
		AA2Edit.exe+1E9885 - 03 C7                 - add eax,edi
		AA2Edit.exe+1E9887 - 89 11                 - mov [ecx],edx { write name length }
		*/
		//edi is xx file buffer, [esi] is pointer to current offset, [eax+edi] reads string length
		DWORD address = General::GameBase + 0x1E9880;
		DWORD redirectAddress = (DWORD)(&OverrideTextureListSizeRedirect);
		Hook((BYTE*)address,
			{ 0x8B, 0x06, 0x8B, 0x14, 0x38 },						//expected values
			{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			NULL);
	}
	else if (General::IsAAPlay) {
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
}

DWORD OverrideTextureListNameOriginal;
DWORD OverrideTextureListNameSkipExit;
void __declspec(naked) OverrideTextureListNameRedirect() {
	__asm {
		pushad
		push esi
		push ebp
		call MeshTextureListFill
		test al, al
		popad
		jnz OverrideTextureListNameRedirect_Skip
		jmp[OverrideTextureListNameOriginal] //original function call
	OverrideTextureListNameRedirect_Skip:
		mov ecx, [OverrideTextureListNameSkipExit]
		mov [esp], ecx
		ret 0xC //to get rid of the parameters to the memcpy we are replacing (including original return
	}
}

void OverrideTextureListNameInject() {
	if (General::IsAAEdit) {
		/*AA2Edit.exe + 1E98E8 - 51 - push ecx				<-- size of string
		AA2Edit.exe + 1E98E9 - 03 D7 - add edx, edi
		AA2Edit.exe + 1E98EB - 52 - push edx				<-- src in xx file
		AA2Edit.exe + 1E98EC - 55 - push ebp				<-- destination buffer
		AA2Edit.exe + 1E98ED - E8 EE7C0800 - call AA2Edit.exe + 2715E0
		AA2Edit.exe + 1E98F2 - 8B 4C 24 18 - mov ecx, [esp + 18]
		AA2Edit.exe + 1E98F6 - 83 C4 0C - add esp, 0C{ 12 }
		AA2Edit.exe + 1E98F9 - 8B C5 - mov eax, ebp
		AA2Edit.exe + 1E98FB - 85 C9 - test ecx, ecx
		AA2Edit.exe + 1E98FD - 76 11 - jna AA2Edit.exe + 1E9910
		AA2Edit.exe + 1E98FF - 90 - nop
		AA2Edit.exe + 1E9900 - 8A 10 - mov dl, [eax]
		AA2Edit.exe + 1E9902 - F6 D2 - not dl
		AA2Edit.exe + 1E9904 - 88 10 - mov[eax], dl
		AA2Edit.exe + 1E9906 - 40 - inc eax
		AA2Edit.exe + 1E9907 - 83 E9 01 - sub ecx, 01 { 1 }
		AA2Edit.exe + 1E990A - 75 F4 - jne AA2Edit.exe + 1E9900
		AA2Edit.exe + 1E990C - 8B 4C 24 0C - mov ecx, [esp + 0C]
		AA2Edit.exe + 1E9910 - 01 0E - add[esi], ecx*/
		//this part is memcpying the name from the buffer, then decrypts it
		DWORD address = General::GameBase + 0x1E98ED;
		DWORD redirectAddress = (DWORD)(&OverrideTextureListNameRedirect);
		Hook((BYTE*)address,
				{ 0xE8, 0xEE, 0x7C, 0x08, 0x00 },						//expected values
				{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			&OverrideTextureListNameOriginal);
		OverrideTextureListNameSkipExit = General::GameBase + 0x1E990C;
	}
	else if (General::IsAAPlay) {
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
		OverrideTextureListNameSkipExit = General::GameBase + 0x206D0C;
	}

}



}
}

/*
* The function we are looking at here is loading textures from a xx file.
* The xx file has a list of files internally that looks somewhat like this
//struct filelist {
//  DWORD nFiles;
//  struct {
//    DWORD nameLength; //including \0
//    char name[nameLength]; //"encrypted", the name is not-ed
//	  DWORD unknown;
//	  DWORD width;
//	  DWORD height;
//	  BYTE unknown[0x21-12]
//	  DWORD filelength;
//	  BYTE file[fileLength]; //the magic number of BMs might be 0
//  } files[nFiles]
//}
* the function gets the offset to the nFiles member. It iterates through every file in these steps:
* 1: in the global texture array, find a free place
* 2.1: read the nameLength field
* 2.2: RtlAllocateHeap a piece of memory of this size
* 2.3: Decrypt the name (its not-ed)
* 3: look through the global array if a texture with this name allready exists. if yes, deallocate the name, go to next file
* 4: if name is new, allocate a buffer that can hold the texture and copy it in there
* a naive and dirty implementation might be to look at the start of this loop if we want to override, the file, then
* change the offset and xx file to point to a custom buffer that contains the file to override with. While this would be clean to write,
* it would be a really stupid solution, as it would have to read a file, allocate it on ram, let the function allocate it again and copy it,
* then delete our file again. thats retarded.
*
* instead, we will have to jump in at multiple places:
* 1: at the start, to determine if we want to replace the file by something else
*    --> the function has to allocate a bigger buffer in that case, so we need to jump in before step 2.1
*    (SHIT): also, since its read-offset skips nameLength in the xx file, we need to adjust the offset
*	  so that he doesnt jump to a wrong location now that the name length was changed
* 2: when he fills this buffer with the name, we need to put our name in it. we cant just modify the xx file, as our
*    name might be longer
* 3: when he starts to copy the file, the meta-data (width/height) must be adjustet; most importantly, fileLength must,
*    so that the buffer he allocates is big enough for the new file
* 4: when he actually copys the file itself, we need to fill it with our file INSTEAD. so we actually have to skip code here as well.
*    (SHIT AGAIN): same as in step 1 here
*/

namespace SharedInjections {
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
		call MeshTextureStart //stdcall
		popad
		add ecx, 0x1408 //original code
		ret
	}
}

void OverrideStartInject() {
	if (General::IsAAEdit) {
		//at this point, [esp+14] is the xx file buffer, [ebp] is the current offset
		/*AA2Edit.exe+1E9DD0 - 8B 4C 24 18           - mov ecx,[esp+18] { start of loop }
		AA2Edit.exe+1E9DD4 - 33 C0                 - xor eax,eax
		AA2Edit.exe+1E9DD6 - 81 C1 08140000        - add ecx,00001408 { 5128 }
		*/
		DWORD address = General::GameBase + 0x1E9DD6;
		DWORD redirectAddress = (DWORD)(&LoadLoopRedirect);
		Hook((BYTE*)address,
			{ 0x81, 0xC1, 0x08, 0x14, 00,00 },						//expected values
			{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress,0x90 },	//redirect to our function
			NULL);
	}
	else if (General::IsAAPlay) {
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
		call MeshTextureOverrideName
		test al,al
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
	if (General::IsAAEdit) {
		/*AA2Edit.exe+1E9E57 - 8B 45 00              - mov eax,[ebp+00]
		AA2Edit.exe+1E9E5A - 53                    - push ebx { size }
		AA2Edit.exe+1E9E5B - 03 C7                 - add eax,edi
		AA2Edit.exe+1E9E5D - 50                    - push eax { cpy source (xx file) }
		AA2Edit.exe+1E9E5E - 56                    - push esi { copy dest (name on heap) }
		AA2Edit.exe+1E9E5F - E8 7C770800           - call AA2Edit.exe+2715E0
		AA2Edit.exe+1E9E64 - 83 C4 0C              - add esp,0C { 12 }
		AA2Edit.exe+1E9E67 - 8B C6                 - mov eax,esi { afterwards, decrypt (not it all) }
		AA2Edit.exe+1E9E69 - 85 DB                 - test ebx,ebx
		AA2Edit.exe+1E9E6B - 76 0F                 - jna AA2Edit.exe+1E9E7C
		AA2Edit.exe+1E9E6D - 8B CB                 - mov ecx,ebx
		AA2Edit.exe+1E9E6F - 90                    - nop
		AA2Edit.exe+1E9E70 - 8A 10                 - mov dl,[eax]
		AA2Edit.exe+1E9E72 - F6 D2                 - not dl
		AA2Edit.exe+1E9E74 - 88 10                 - mov [eax],dl
		AA2Edit.exe+1E9E76 - 40                    - inc eax
		AA2Edit.exe+1E9E77 - 83 E9 01              - sub ecx,01 { 1 }
		AA2Edit.exe+1E9E7A - 75 F4                 - jne AA2Edit.exe+1E9E70
		AA2Edit.exe+1E9E7C - 01 5D 00              - add [ebp+00],ebx
		*/
		DWORD address = General::GameBase + 0x1E9E5A;
		DWORD redirectAddress = (DWORD)(&OverrideNameRedirect);
		Hook((BYTE*)address,
			{ 0x53,
			0x03, 0xC7,
			0x50,
			0x56, },
			{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			NULL);
		OverrideNameDefaultExit = General::GameBase + 0x1E9E5F;
		OverrideNameCustomExit = General::GameBase + 0x1E9E7C;
	}
	else if (General::IsAAPlay) {
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
		call MeshTextureOverrideSize
		popad
		//original stuff that we replaced
		mov[edx + eax * 4 + 0x00001408], esi
		ret
	}
}

void OverrideFileSizeInject() {
	if (General::IsAAEdit) {
		//gotta make sure that we change the meta-information of the image in the buffer
		/*AA2Edit.exe+1EA056 - 8B 54 24 18           - mov edx,[esp+18] { comes here if name was not found }
		AA2Edit.exe+1EA05A - 8B 44 24 2C           - mov eax,[esp+2C]
		AA2Edit.exe+1EA05E - 8B 4C 24 14           - mov ecx,[esp+14] { xx file }
		AA2Edit.exe+1EA062 - 89 B4 82 08140000     - mov [edx+eax*4+00001408],esi
		AA2Edit.exe+1EA069 - 89 1E                 - mov [esi],ebx
		AA2Edit.exe+1EA06B - 89 7E 04              - mov [esi+04],edi
		AA2Edit.exe+1EA06E - 8B 55 00              - mov edx,[ebp+00]
		AA2Edit.exe+1EA071 - 8B 04 0A              - mov eax,[edx+ecx]
		*/
		DWORD address = General::GameBase + 0x1EA062;
		DWORD redirectAddress = (DWORD)(&OverrideFileSizeRedirect);
		Hook((BYTE*)address,
			{ 0x89, 0xB4, 0x82, 0x08, 0x14,00,00 },
			{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress, 0x90, 0x90 },	//redirect to our function
			NULL);
	}
	else if (General::IsAAPlay) {
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
		call MeshTextureOverrideFile
		test al,al
		popad
		jz OverrideFile_Original //if function returned false, do normal call
		ret //else, we did it allready, so just return
	OverrideFile_Original :
		jmp[OverrideFileOriginalCall]
	}
}

void OverrideFileInject() {
	if (General::IsAAEdit) {
		//this function is the memcpy(buffer, file, filesize). the parameters are
		//removed way down there. cdecl n stuff.
		/*AA2Edit.exe+1EA110 - 03 54 24 14           - add edx,[esp+14]
		AA2Edit.exe+1EA114 - 57                    - push edi
		AA2Edit.exe+1EA115 - 52                    - push edx
		AA2Edit.exe+1EA116 - 50                    - push eax
		AA2Edit.exe+1EA117 - E8 C4740800           - call AA2Edit.exe+2715E0 { fill new heap (eax) with file }
		AA2Edit.exe+1EA11C - 8B 4E 04              - mov ecx,[esi+04]
		AA2Edit.exe+1EA11F - 83 C4 0C              - add esp,0C { 12 }
		*/
		DWORD address = General::GameBase + 0x1EA117;
		DWORD redirectAddress = (DWORD)(&OverrideFileRedirect);
		Hook((BYTE*)address,
			{ 0xE8, 0xC4, 0x74, 0x08, 0x00, },
			{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			&OverrideFileOriginalCall);
	}
	else if (General::IsAAPlay) {
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

void __stdcall OverrideOutlineColorFunc(float* colors) {
	if (!Shared::GameState::getIsOverriding()) return;
	if (!Shared::g_currentChar->m_cardData.HasOutlineColor()) return;
	COLORREF color = Shared::g_currentChar->m_cardData.GetOutlineColor();
	//colors are sequentially in rgba order in *colors
	*colors++ = GetRValue(color)/255.0f;
	*colors++ = GetGValue(color)/255.0f;
	*colors++ = GetBValue(color)/255.0f;
}

void __declspec(naked) OverrideOutlineColorRedirect() {
	__asm {
		pushad
		lea ecx, [eax+ebx]
		push ecx
		call OverrideOutlineColorFunc
		popad
		mov ecx, [eax+ebx]
		fldz
		ret
	}
}

void OverrideOutlineColorInject() {
	if (General::IsAAEdit) {
		//reads the color (floats starting there)
		/*AA2Edit.exe+1E8840 - 8B 0C 18              - mov ecx,[eax+ebx]
		AA2Edit.exe+1E8843 - D9EE                  - fldz 
		*/
		DWORD address = General::GameBase + 0x1E8840;
		DWORD redirectAddress = (DWORD)(&OverrideOutlineColorRedirect);
		Hook((BYTE*)address,
			{ 0x8B, 0x0c, 0x18, 0xD9, 0xEE, },
			{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			NULL);
	}
	else if (General::IsAAPlay) {
		/*AA2Play v12 FP v1.4.0a.exe+205C40 - 8B 0C 18              - mov ecx,[eax+ebx]
		AA2Play v12 FP v1.4.0a.exe+205C43 - D9EE                  - fldz 
		*/
		DWORD address = General::GameBase + 0x205C40;
		DWORD redirectAddress = (DWORD)(&OverrideOutlineColorRedirect);
		Hook((BYTE*)address,
			{ 0x8B, 0x0c, 0x18, 0xD9, 0xEE, },
			{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			NULL);
	}
}

void __stdcall OverrideBoneEvent(ExtClass::Frame* bone) {
	/*//note that this event fires for the child bones first, then the parent. dont enumerate child bones again.
	if (!Shared::g_isOverriding) return;
	TCHAR nameBuffer[256];
	size_t out;
	mbstowcs_s(&out,nameBuffer,bone->m_name,256);
	const D3DMATRIX* ret = Shared::g_currentChar->m_cardData.GetBoneTransformationRule(nameBuffer);
	if(ret != NULL) {
		//(*Shared::D3DXMatrixMultiply)(&bone->m_matrix5,ret,&bone->m_matrix5);
		//(*Shared::D3DXMatrixMultiply)(&bone->m_matrix1,ret,&bone->m_matrix1);
		bone->m_matrix1 = *ret;
		bone->m_matrix5 = *ret;
	}*/
	if (strcmp(bone->m_name,"a01_J_UdeR_01") != 0) return;
	using namespace ExtClass;
	Frame* newMatch = (Frame*)Shared::IllusionMemAlloc(sizeof(Frame));
	memcpy_s(newMatch,sizeof(Frame),bone,sizeof(Frame));

	//change parent and child stuff
	bone->m_parent = newMatch->m_parent;
	bone->m_nChildren = 1;
	bone->m_children = newMatch;
	newMatch->m_parent = bone;
	for (int i = 0; i < newMatch->m_nChildren; i++) {
		newMatch->m_children[i].m_parent = newMatch;
	}

	//change name
	int namelength = newMatch->m_nameBufferSize + 5;
	bone->m_name = (char*)Shared::IllusionMemAlloc(namelength);
	bone->m_nameBufferSize = namelength;
	//strcpy_s(match->m_name,match->m_nameBufferSize,newMatch->m_name);
	//strcat_s(match->m_name,match->m_nameBufferSize,"_artf");
	strcpy_s(bone->m_name,bone->m_nameBufferSize,"newname");

	//some values
	const D3DMATRIX idMatr = { 1.0f,0,0,0, 0,1.0f,0,0, 0,0,1.0f,0, 0,0,0,1.0f };
	bone->m_matrix1 = idMatr;
	bone->m_matrix5 = idMatr;
}

void __declspec(naked) OverrideBoneRedirect() {
	__asm {
		push eax
		push ebx
		call OverrideBoneEvent
		pop eax
		pop edi
		pop esi
		pop ebp
		pop ebx
		ret
	}
}

void OverrideBoneInject() {
	//end of function that reads a bone from the xx file
	if (General::IsAAEdit) {
		//this is where the function ends that generates a bone struct. this struct is currently in ebx.
		/*AA2Edit.exe+1E847A - 5F                    - pop edi
		AA2Edit.exe+1E847B - 5E                    - pop esi
		AA2Edit.exe+1E847C - 5D                    - pop ebp
		AA2Edit.exe+1E847D - 5B                    - pop ebx
		AA2Edit.exe+1E847E - C3                    - ret*/
		DWORD address = General::GameBase + 0x1E847A;
		DWORD redirectAddress = (DWORD)(&OverrideBoneRedirect);
		Hook((BYTE*)address,
		{ 0x5F, 0x5E, 0x5D, 0x5B, 0xC3, },
		{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			NULL);
	}
	else if (General::IsAAPlay) {
		/*AA2Play v12 FP v1.4.0a.exe+205867 - 40                    - inc eax
		AA2Play v12 FP v1.4.0a.exe+205868 - 83 C4 14              - add esp,14 { 20 }
		AA2Play v12 FP v1.4.0a.exe+20586B - 81 C7 F4420000        - add edi,000042F4 { 17140 }
		AA2Play v12 FP v1.4.0a.exe+205871 - 3B 43 08              - cmp eax,[ebx+08]
		AA2Play v12 FP v1.4.0a.exe+205874 - 89 44 24 20           - mov [esp+20],eax
		AA2Play v12 FP v1.4.0a.exe+205878 - 7C DB                 - jl "AA2Play v12 FP v1.4.0a.exe"+205855 { ->AA2Play v12 FP v1.4.0a.exe+205855 }
		AA2Play v12 FP v1.4.0a.exe+20587A - 5F                    - pop edi
		AA2Play v12 FP v1.4.0a.exe+20587B - 5E                    - pop esi
		AA2Play v12 FP v1.4.0a.exe+20587C - 5D                    - pop ebp
		AA2Play v12 FP v1.4.0a.exe+20587D - 5B                    - pop ebx
		AA2Play v12 FP v1.4.0a.exe+20587E - C3                    - ret 
		*/
		DWORD address = General::GameBase + 0x20587A;
		DWORD redirectAddress = (DWORD)(&OverrideBoneRedirect);
		Hook((BYTE*)address,
		{ 0x5F, 0x5E, 0x5D, 0x5B, 0xC3, },
		{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			NULL);
	}
}



void __stdcall OverrideBoneEventV2(ExtClass::XXFile* xxFile) {
	Shared::XXFileModification(xxFile, General::IsAAEdit);
}

DWORD OverrideBoneOriginalFunctionV2;
void __declspec(naked) OverrideBoneRedirectV2() {
	/*__asm {
		push [esp+0xC]
		push [esp+0xC]
		push [esp+0xC]
		call [OverrideBoneOriginalFunctionV2]
		add esp, 0x0C
		push eax //save return value
		push [esp+0x8] //esi, the xx file
		call OverrideBoneEventV2
		pop eax
		ret
	}*/
	__asm {
		push[esp+0x4] //esi, the xx file
		call OverrideBoneEventV2

		jmp [OverrideBoneOriginalFunctionV2]
	}
}

void OverrideBoneInjectV2() {
	//place that inserts animation into an xx file
	if (General::IsAAEdit) {
		/*
		AA2Edit.exe+1EA464 - 51                    - push ecx
		AA2Edit.exe+1EA465 - 57                    - push edi
		AA2Edit.exe+1EA466 - 56                    - push esi
		AA2Edit.exe+1EA467 - E8 14000000           - call AA2Edit.exe+1EA480 //reads animation struct for esi (xx file)
		AA2Edit.exe+1EA46C - 83 C4 0C              - add esp,0C { 12 }
		*/
		DWORD address = General::GameBase + 0x1EA467;
		DWORD redirectAddress = (DWORD)(&OverrideBoneRedirectV2);
		Hook((BYTE*)address,
			{ 0xE8, 0x14, 0x00, 0x00, 0x00, },
			{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			&OverrideBoneOriginalFunctionV2);
	}
	else if (General::IsAAPlay) {
		/*AA2Play v12 FP v1.4.0a.exe+207834 - 51                    - push ecx
		AA2Play v12 FP v1.4.0a.exe+207835 - 57                    - push edi
		AA2Play v12 FP v1.4.0a.exe+207836 - 56                    - push esi
		AA2Play v12 FP v1.4.0a.exe+207837 - E8 14000000           - call "AA2Play v12 FP v1.4.0a.exe"+207850 { ->AA2Play v12 FP v1.4.0a.exe+207850 }
		AA2Play v12 FP v1.4.0a.exe+20783C - 83 C4 0C              - add esp,0C { 12 }
		*/
		DWORD address = General::GameBase + 0x207837;
		DWORD redirectAddress = (DWORD)(&OverrideBoneRedirectV2);
		Hook((BYTE*)address,
			{ 0xE8, 0x14, 0x00, 0x00, 0x00, },
			{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			&OverrideBoneOriginalFunctionV2);
	}
}

void __stdcall OverrideFrameEvent(ExtClass::XXFile* xxFile) {
	Shared::XXFileModification(xxFile,General::IsAAEdit);
	if (General::IsAAPlay) Poser::FrameModEvent(xxFile);
}

void __declspec(naked) OverrideFrameRedirect() {
	__asm {
		add esp, 0x20
		pushad
		push eax
		call OverrideFrameEvent
		popad
		ret
	}
}

void OverrideFrameInject() {
	if(General::IsAAEdit) {
		//function that reads an xx file from an xx file (no animations and stuff)
		/*AA2Edit.exe+1F8B50 - 6A FF                 - push -01 { function that reads an xx file }
		AA2Edit.exe+1F8B52 - 68 B0BE5500           - push AA2Edit.exe+2BBEB0 { [0824548B] }
		AA2Edit.exe+1F8B57 - 64 A1 00000000        - mov eax,fs:[00000000] { 0 }
		AA2Edit.exe+1F8B5D - 50                    - push eax
		AA2Edit.exe+1F8B5E - 83 EC 14              - sub esp,14 { 20 }
		AA2Edit.exe+1F8B61 - A1 A00A5E00           - mov eax,[AA2Edit.exe+340AA0] { [1942D58A] }
		AA2Edit.exe+1F8B66 - 33 C4                 - xor eax,esp
		*/
		//...
		/*AA2Edit.exe+1F8D1A - 8B 4C 24 10           - mov ecx,[esp+10]
		AA2Edit.exe+1F8D1E - 33 CC                 - xor ecx,esp
		AA2Edit.exe+1F8D20 - E8 442F0700           - call AA2Edit.exe+26BC69
		AA2Edit.exe+1F8D25 - 83 C4 20              - add esp,20 { 32 }
		AA2Edit.exe+1F8D28 - C3                    - ret
		AA2Edit.exe+1F8D29 - CC                    - int 3
		AA2Edit.exe+1F8D2A - CC                    - int 3
		AA2Edit.exe+1F8D2B - CC                    - int 3
		AA2Edit.exe+1F8D2C - CC                    - int 3
		*/
		DWORD address = General::GameBase + 0x1F8D25;
		DWORD redirectAddress = (DWORD)(&OverrideFrameRedirect);
		Hook((BYTE*)address,
			{ 0x83, 0xC4, 0x20, 
				0xC3, 
				0xCC, },
			{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			NULL);
	}
	else if(General::IsAAPlay) {
		/*AA2Play v12 FP v1.4.0a.exe+2165D0 - 6A FF                 - push -01 { 255 }
		AA2Play v12 FP v1.4.0a.exe+2165D2 - 68 D0A4D500           - push "AA2Play v12 FP v1.4.0a.exe"+2DA4D0 { [0824548B] }
		AA2Play v12 FP v1.4.0a.exe+2165D7 - 64 A1 00000000        - mov eax,fs:[00000000] { 0 }
		AA2Play v12 FP v1.4.0a.exe+2165DD - 50                    - push eax
		AA2Play v12 FP v1.4.0a.exe+2165DE - 83 EC 14              - sub esp,14 { 20 }
		AA2Play v12 FP v1.4.0a.exe+2165E1 - A1 A03ADE00           - mov eax,["AA2Play v12 FP v1.4.0a.exe"+363AA0] { [198] }
		AA2Play v12 FP v1.4.0a.exe+2165E6 - 33 C4                 - xor eax,esp
		*/
		//...
		/*AA2Play v12 FP v1.4.0a.exe+21679A - 8B 4C 24 10           - mov ecx,[esp+10]
		AA2Play v12 FP v1.4.0a.exe+21679E - 33 CC                 - xor ecx,esp
		AA2Play v12 FP v1.4.0a.exe+2167A0 - E8 B4320700           - call "AA2Play v12 FP v1.4.0a.exe"+289A59 { ->AA2Play v12 FP v1.4.0a.exe+289A59 }
		AA2Play v12 FP v1.4.0a.exe+2167A5 - 83 C4 20              - add esp,20 { 32 }
		AA2Play v12 FP v1.4.0a.exe+2167A8 - C3                    - ret 
		AA2Play v12 FP v1.4.0a.exe+2167A9 - CC                    - int 3 
		AA2Play v12 FP v1.4.0a.exe+2167AA - CC                    - int 3 
		AA2Play v12 FP v1.4.0a.exe+2167AB - CC                    - int 3 
		AA2Play v12 FP v1.4.0a.exe+2167AC - CC                    - int 3 
		*/
		DWORD address = General::GameBase + 0x2167A5;
		DWORD redirectAddress = (DWORD)(&OverrideFrameRedirect);
		Hook((BYTE*)address,
			{ 0x83, 0xC4, 0x20,
				0xC3,
				0xCC, },
			{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			NULL);
	}

}

void __stdcall OverrideBoneManipulationEvent(ExtClass::Frame* boneFrame) {
	Shared::XXBoneModification(boneFrame,General::IsAAEdit);
}

DWORD OverrideBoneManipulationOriginal;
void __declspec(naked) OverrideBoneManipulationRedirect() {
	__asm {
		push [esp+8]
		push [esp+8]
		call [OverrideBoneManipulationOriginal]
		add esp, 8

		push eax

		push ebx
		call OverrideBoneManipulationEvent

		pop eax
		ret
	}
}

void OverrideBoneManipulationInject() {
	if (General::IsAAEdit) {
		/*
		AA2Edit.exe+1F9792 - 56                    - push esi
		AA2Edit.exe+1F9793 - 53                    - push ebx{ frame of which the subbones will be set }
		AA2Edit.exe+1F9794 - 8B C7                 - mov eax,edi
		AA2Edit.exe+1F9796 - E8 F5080000           - call AA2Edit.exe+1FA090{ changes all bones from this frame }
		AA2Edit.exe+1F979B - 83 C4 08              - add esp,08 { 8 }
		*/
		DWORD address = General::GameBase + 0x1F9796;
		DWORD redirectAddress = (DWORD)(&OverrideBoneManipulationRedirect);
		Hook((BYTE*)address,
			{ 0xE8, 0xF5, 0x08, 00, 00 },
			{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			&OverrideBoneManipulationOriginal);
	}
	else if (General::IsAAPlay) {
		/*
		AA2Play v12 FP v1.4.0a.exe+217212 - 56                    - push esi
		AA2Play v12 FP v1.4.0a.exe+217213 - 53                    - push ebx
		AA2Play v12 FP v1.4.0a.exe+217214 - 8B C7                 - mov eax,edi
		AA2Play v12 FP v1.4.0a.exe+217216 - E8 F5080000           - call "AA2Play v12 FP v1.4.0a.exe"+217B10 { ->AA2Play v12 FP v1.4.0a.exe+217B10 }
		AA2Play v12 FP v1.4.0a.exe+21721B - 83 C4 08              - add esp,08 { 8 }
		*/
		DWORD address = General::GameBase + 0x217216;
		DWORD redirectAddress = (DWORD)(&OverrideBoneManipulationRedirect);
		Hook((BYTE*)address,
			{ 0xE8, 0xF5, 0x08, 00, 00 },
			{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			&OverrideBoneManipulationOriginal);
	}
}

BYTE* loc_overrideObjectOrigFile = NULL;
DWORD loc_overrideObjectOrigOffset = 0;
DWORD* loc_overrideObjectOrigOffsetPtr = NULL;
BYTE* loc_overrideObjectFileBuffer = NULL;
bool loc_overrideObjectIsOverriding = false;
int loc_overrideObjectRecursionCount = 0;
void __stdcall OverrideObjectStart(BYTE** ptrFile, DWORD* offset) {
	if(loc_overrideObjectIsOverriding) {
		loc_overrideObjectRecursionCount++;
	}
	else {
		BYTE* objectBuffer = *ptrFile + *offset;
		DWORD size = *(DWORD*)(objectBuffer);
		if (size == 0 || size > 256) return;
		char buffer[256];
		char* it = (char*)(objectBuffer + 4);
		for (int i = 0; i < size; i++) buffer[i] = ~it[i];
		const XXObjectFile* match = Shared::ObjectOverrideRules(buffer);
		if (match == NULL) return;

		//read through this object so we can skip it
		DWORD osize = FileFormats::XXFile::ReadObjectLength(objectBuffer);
		if (osize == 0) return; //error

		loc_overrideObjectOrigFile = *ptrFile;
		loc_overrideObjectOrigOffset = *offset + osize;
		loc_overrideObjectOrigOffsetPtr = offset;
		loc_overrideObjectFileBuffer = new BYTE[match->GetFileSize()];
		match->WriteToBuffer(loc_overrideObjectFileBuffer);
		loc_overrideObjectIsOverriding = true;

		*ptrFile = loc_overrideObjectFileBuffer;
		*offset = 0;
	}
	
}

void __stdcall OverrideObjectEnd() {
	if (loc_overrideObjectIsOverriding) {
		loc_overrideObjectRecursionCount--;
		if(loc_overrideObjectRecursionCount < 0) {
			//we're done, restore values
			*loc_overrideObjectOrigOffsetPtr = loc_overrideObjectOrigOffset;
			loc_overrideObjectIsOverriding = false;
			loc_overrideObjectRecursionCount = 0;
			delete[] loc_overrideObjectFileBuffer;
			loc_overrideObjectFileBuffer = NULL;
		}
	}
}

void __declspec(naked) OverrideObjectRedirectStart() {
	__asm {
		pushad

		mov eax, [esp + 4 + 0x20 + 0x14] //pointer to offset (thats the parameter, a pointer to the offset)
		lea edx, [esp + 4 + 0x20 + 0x10] //pointer to the file (to the parameter on the stack)
		push eax
		push edx
		call OverrideObjectStart

		popad

		mov eax, [esp+ 4 + 0x0C]
		push [esp]
		mov [esp+4], ebx
		
		ret
	};
}
void __declspec(naked) OverrideObjectRedirectEnd() {
	__asm {
		pop edi
		pop esi
		pop ebp
		pop ebx
		pushad
		call OverrideObjectEnd
		popad

		ret
	};
}

void OverrideObjectInject() {
	if (General::IsAAEdit) {
		//reads an object from the file buffer, including its children. [esp+10] is the file, [esp+14] is the pointer to the current offset
		/*
		AA2Edit.exe+1E8120 - 8B 44 24 0C           - mov eax,[esp+0C] { creates a bone }
		AA2Edit.exe+1E8124 - 53                    - push ebx
		*/
		//...
		//
		/*
		AA2Edit.exe+1E847A - 5F                    - pop edi
		AA2Edit.exe+1E847B - 5E                    - pop esi
		AA2Edit.exe+1E847C - 5D                    - pop ebp
		AA2Edit.exe+1E847D - 5B                    - pop ebx
		AA2Edit.exe+1E847E - C3                    - ret
		AA2Edit.exe+1E847F - CC                    - int 3
		*/
		DWORD address = General::GameBase + 0x1E8120;
		DWORD redirectAddress = (DWORD)(&OverrideObjectRedirectStart);
		Hook((BYTE*)address,
			{ 0x8B, 0x44, 0x24, 0x0C, 0x53 },
			{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			NULL);
		address = General::GameBase + 0x1E847A;
		redirectAddress = (DWORD)(&OverrideObjectRedirectEnd);
		Hook((BYTE*)address,
		{ 0x5F, 0x5E, 0x5D, 0x5B, 0xC3 },
		{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			NULL);
	}
	else if (General::IsAAPlay) {
		/*AA2Play v12 FP v1.4.0a.exe+205520 - 8B 44 24 0C           - mov eax,[esp+0C]
		AA2Play v12 FP v1.4.0a.exe+205524 - 53                    - push ebx
		*/
		//...
		/*AA2Play v12 FP v1.4.0a.exe+20587A - 5F                    - pop edi
		AA2Play v12 FP v1.4.0a.exe+20587B - 5E                    - pop esi
		AA2Play v12 FP v1.4.0a.exe+20587C - 5D                    - pop ebp
		AA2Play v12 FP v1.4.0a.exe+20587D - 5B                    - pop ebx
		AA2Play v12 FP v1.4.0a.exe+20587E - C3                    - ret 
		*/
		DWORD address = General::GameBase + 0x205520;
		DWORD redirectAddress = (DWORD)(&OverrideObjectRedirectStart);
		Hook((BYTE*)address,
			{ 0x8B, 0x44, 0x24, 0x0C, 0x53 },
			{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			NULL);
		address = General::GameBase + 0x20587A;
		redirectAddress = (DWORD)(&OverrideObjectRedirectEnd);
		Hook((BYTE*)address,
			{ 0x5F, 0x5E, 0x5D, 0x5B, 0xC3 },
			{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			NULL);
	}
}

void __stdcall OverrideTanColorEvent(DWORD* tanColor, DWORD* unknownColor) {
	Shared::OverrideTanColor(tanColor,unknownColor);
}

DWORD OverrideTanColorOriginal;
void __declspec(naked) OverrideTanColorRedirect() {
	__asm {
		push [esp+0x8]
		push [esp+0x8]
		call [OverrideTanColorOriginal]
		add esp, 8
		mov eax, [esp+0x8]
		mov edx, [esp+0x4]
		push eax
		push edx
		call OverrideTanColorEvent
		ret
	}
}

void OverrideTanColorInject() {
	if (General::IsAAEdit) {
		//func(DWORD* outARGB, DWORD* ???, ...???
		//function returns the color in [esp+84] as a DWORD, ARGB (A being hi-byte)
		/*
		AA2Edit.exe+11B356 - E8 F5AFFBFF           - call AA2Edit.exe+D6350 { returns the tan color }
		AA2Edit.exe+11B35B - 8B 84 24 84000000     - mov eax,[esp+00000084]
		*/
		DWORD address = General::GameBase + 0x11B356;
		DWORD redirectAddress = (DWORD)(&OverrideTanColorRedirect);
		Hook((BYTE*)address,
			{ 0xE8, 0xF5, 0xAF, 0xFB, 0xFF },
			{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			&OverrideTanColorOriginal);
	}
	else if (General::IsAAPlay) {
		/*
		AA2Play v12 FP v1.4.0a.exe+12CE66 - E8 D573FBFF           - call "AA2Play v12 FP v1.4.0a.exe"+E4240 { ->AA2Play v12 FP v1.4.0a.exe+E4240 }
		AA2Play v12 FP v1.4.0a.exe+12CE6B - 8B 84 24 84000000     - mov eax,[esp+00000084]
		*/
		DWORD address = General::GameBase + 0x12CE66;
		DWORD redirectAddress = (DWORD)(&OverrideTanColorRedirect);
		Hook((BYTE*)address,
			{ 0xE8, 0xD5, 0x73, 0xFB, 0xFF },
			{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
			&OverrideTanColorOriginal);
	}
}


}
}

