#include "MeshTexture.h"

#include "MemMods\Hook.h"
#include "General\ModuleInfo.h"
#include "Functions\Shared\Overrides.h"
#include "Functions\Shared\SpecialOverrides.h"

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


}
}
