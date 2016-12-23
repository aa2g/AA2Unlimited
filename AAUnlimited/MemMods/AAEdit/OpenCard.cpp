#include "OpenCard.h"

#include <Windows.h>
#include <intrin.h>
#include <algorithm>

#include "MemMods\Hook.h"
#include "General\ModuleInfo.h"
#include "General\Util.h"
#include "Files\Config.h"
#include "Files\Logger.h"
#include "Functions\AAEdit\Globals.h"
#include "Functions\AAEdit\UnlimitedDialog.h"

namespace EditInjections {
namespace OpenCard {


void __stdcall ReadUnlimitData(HANDLE hFile, DWORD /*illusionDataOffset*/) {
	DWORD charDataRule[]{ 0x353254, 0x2C, 0 };
	AAEdit::g_currChar.m_char = (ExtClass::CharacterStruct*) ExtVars::ApplyRule(charDataRule);
	//clear current data
	AAEdit::g_currChar.m_cardData.Reset();
	//first, find our unlimited data
	DWORD lo, hi;
	lo = GetFileSize(hFile, &hi);
	BYTE* needed = new BYTE[lo];
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	ReadFile(hFile, needed, lo, &hi, NULL);
	//load data
	AAEdit::g_currChar.m_cardData.FromFileBuffer((char*)needed, lo);
	AAEdit::g_currChar.m_cardData.DumpSavedOverrideFiles();
	//set save checkboxes in GUI depending on data
	//override files
	BOOL savedFiles = AAEdit::g_currChar.m_cardData.HasFilesSaved();
	AAEdit::g_AAUnlimitDialog.SetSaveFiles(savedFiles);
	//eye highlight
	savedFiles = AAEdit::g_currChar.m_cardData.GetEyeHighlightTextureBuffer().size() > 0;
	AAEdit::g_AAUnlimitDialog.SetSaveHighlight(savedFiles);
	//eye texture
	savedFiles = AAEdit::g_currChar.m_cardData.GetEyeTextureBuffer(0).size() > 0 ||
				 AAEdit::g_currChar.m_cardData.GetEyeTextureBuffer(1).size() > 0;
	AAEdit::g_AAUnlimitDialog.SetSaveEyes(savedFiles);
	AAEdit::g_AAUnlimitDialog.Refresh();
}

void __stdcall ReadUnlimitDataV2(const wchar_t* card) {
	auto& aauData = AAEdit::g_currChar.m_cardData;
	aauData.Reset();

	HANDLE hFile = CreateFile(card,FILE_GENERIC_READ | FILE_GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
	if (hFile == INVALID_HANDLE_VALUE || hFile == NULL) {
		int error = GetLastError();
		LOGPRIO(Logger::Priority::WARN) << "Could not read card " << card << " to preview; error: " << error << "\r\n";
		return;
	}

	//read file first, we will need to truncate it anyway
	DWORD hi,lo = GetFileSize(hFile,&hi);
	BYTE* fileBuffer = new BYTE[lo];
	ReadFile(hFile,fileBuffer,lo,&hi,NULL);
	//read from data
	bool suc = aauData.FromFileBuffer((char*)fileBuffer,lo);
	//set save checkboxes in GUI depending on data
	//override files
	BOOL savedFiles = aauData.HasFilesSaved();
	AAEdit::g_AAUnlimitDialog.SetSaveFiles(savedFiles);
	//eye highlight
	BOOL savedHighlights = aauData.GetEyeHighlightTextureBuffer().size() > 0;
	AAEdit::g_AAUnlimitDialog.SetSaveHighlight(savedHighlights);
	//eye texture
	BOOL savedEyeTextureLeft = aauData.GetEyeTextureBuffer(0).size() > 0;
	BOOL savedEyeTextureRight = aauData.GetEyeTextureBuffer(1).size() > 0;
	AAEdit::g_AAUnlimitDialog.SetSaveEyes(savedEyeTextureLeft || savedEyeTextureRight);
	AAEdit::g_AAUnlimitDialog.Refresh();

	bool dumped = aauData.DumpSavedOverrideFiles();

	if(!suc || !dumped || !g_Config.GetKeyValue(Config::SAVED_FILE_REMOVE).bVal) {
		//not an aau card
		CloseHandle(hFile);
		delete[] fileBuffer;
		return;
	}
	
	if(g_Config.GetKeyValue(Config::SAVED_FILE_BACKUP).bVal) {
		//do a backup
		const wchar_t* filePart = General::FindFileInPath(card);
		std::wstring backupName(card,filePart);
		backupName += TEXT("\\aaubackup\\");
		backupName += filePart;
		General::CreatePathForFile(backupName.c_str());
		CopyFile(card,backupName.c_str(),FALSE);
	}
	//remove the saved files
	
	//modify size first
	DWORD* chunkSize = (DWORD*)aauData.ret_chunkSize;
	DWORD currSize = _byteswap_ulong(*chunkSize);
	
	for(int i = 0; i < 4; i++) {
		currSize -= aauData.ret_files[i].size();
	}
	*chunkSize = _byteswap_ulong(currSize);

	//make a list of chunks that we need to write (the inverse of these ret_files, essentially)
	
	std::vector<std::pair<BYTE*,DWORD>> toWrite;
	
	DWORD it = 0;
	for(int i = 0; i < 4; i++) {
		if (aauData.ret_files[i].size() == 0) continue;
		std::pair<BYTE*,DWORD> pair;
		pair.first = fileBuffer + it;
		pair.second = (DWORD)(aauData.ret_files[i].fileStart - (char*)(fileBuffer + it));
		toWrite.push_back(pair);
		it += pair.second + aauData.ret_files[i].size();
	}
	{
		std::pair<BYTE*,DWORD> pair;
		pair.first = fileBuffer + it;
		pair.second = (DWORD)(lo - it);
		toWrite.push_back(pair);
	}


	//now write file
	LONG himove = 0;
	SetFilePointer(hFile,0,&himove,FILE_BEGIN);

	for(auto& pair : toWrite) {
		WriteFile(hFile,pair.first,pair.second,&hi,NULL);
	}
	
	SetEndOfFile(hFile);

	CloseHandle(hFile);
	delete[] fileBuffer;
}

DWORD ReadUnlimitDataOriginal;
void __declspec(naked) ReadUnlimitDataRedirect() {
	__asm {
		push [esp+8] //offset of illusions data
		push [esp+8] //formerly esp+4, file handle
		call ReadUnlimitData
		mov eax, [ReadUnlimitDataOriginal] //note: eax contains AA2Edit-exe+2C41EC now
		jmp dword ptr [eax] //redirect to original function; it will do the return for us
	}
}

void __declspec(naked) ReadUnlimitDataRedirectV2() {
	__asm {
		push [esp+8]
		push [esp+8]
		call [ReadUnlimitDataOriginal]
		add esp, 8
		push eax
		push [esp+8]
		call ReadUnlimitDataV2
		pop eax
		ret
	}
}

void ReadUnlimitDataInject() {
	//Part of the open card function. The last DWORD in the png file actually indicates the start of the custom data,
	//so that filesize-lastDword = offset of custom data.
	//edi is that offset at this point, and the SetFilePointer call will put the file (ebp) to that location.
	//the custom data part.
	/*AA2Edit.exe+127E1A - 6A 00                 - push 00 { 0 }
	AA2Edit.exe+127E1C - 6A 00                 - push 00 { 0 }
	AA2Edit.exe+127E1E - 57                    - push edi
	AA2Edit.exe+127E1F - 55                    - push ebp
	AA2Edit.exe+127E20 - FF 15 EC414F00        - call dword ptr [AA2Edit.exe+2C41EC] { ->->KERNELBASE.SetFilePointer }
	*/
	/*DWORD address = General::GameBase + 0x127E20;
	DWORD redirectAddress = (DWORD)(&ReadUnlimitDataRedirect);
	Hook((BYTE*)address,
		{ 0xFF, 0x15, 0xEC, 0x41, 0x4f, 0x00 },							//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress, 0x90 },	//redirect to our function
		NULL);
	ReadUnlimitDataOriginal = General::GameBase + 0x2C41EC;*/

	//the open card function. eax (second paramter) is the full path to the card, as a wchar_t
	/*AA2Edit.exe+1270BF - 50                    - push eax
	AA2Edit.exe+1270C0 - 51                    - push ecx
	AA2Edit.exe+1270C1 - 88 5C 24 34           - mov [esp+34],bl
	AA2Edit.exe+1270C5 - 89 5C 24 38           - mov [esp+38],ebx
	AA2Edit.exe+1270C9 - E8 920C0000           - call AA2Edit.exe+127D60
	AA2Edit.exe+1270CE - 83 C4 08              - add esp,08 { 8 }
	*/
	DWORD address = General::GameBase + 0x1270C9;
	DWORD redirectAddress = (DWORD)(&ReadUnlimitDataRedirectV2);
	Hook((BYTE*)address,
	{ 0xE8, 0x92, 0x0C, 0x00, 0x00 },							//expected values
	{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		&ReadUnlimitDataOriginal);
}

void __stdcall PreviewCardEvent(const wchar_t* card) {
	AAEdit::g_AAUnlimitDialog.Initialize();
	ReadUnlimitDataV2(card);
}

DWORD PreviewCardOriginal;
void __declspec(naked) PreviewCardRedirect() {
	__asm {
		pushad
		push ecx
		call PreviewCardEvent
		popad
		jmp [PreviewCardOriginal]
	}
}

void PreviewCardInject() {
	//eax is the card path. some stack parameter, some class. function allocates buffer and reads card, not sure where the buffer goes
	/*AA2Edit.exe+1A467 - 8D 84 24 64020000     - lea eax,[esp+00000264]
	AA2Edit.exe+1A46E - 89 74 24 18           - mov [esp+18],esi
	AA2Edit.exe+1A472 - E8 C9CD1000           - call AA2Edit.exe+127240
	*/
	/*DWORD address = General::GameBase + 0x1A472;
	DWORD redirectAddress = (DWORD)(&PreviewCardRedirect);
	Hook((BYTE*)address,
		{ 0xE8, 0xC9, 0xCD, 0x10, 0x00 },							//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		&PreviewCardOriginal);*/

	//no stack params, some registers tho; eax is path to card.
	//note that this is called when clicking a name in the list, not the preview button;
	//this function loads the image to preview, and that is done using a gdi createbitmapfromfile call,
	//which keeps the file open for some godfucking retarded reason
	/*AA2Edit.exe+18B5F - 8B 77 54              - mov esi,[edi+54]
	AA2Edit.exe+18B62 - E8 39F7FFFF           - call AA2Edit.exe+182A0*/
	DWORD address = General::GameBase + 0x18B62;
	DWORD redirectAddress = (DWORD)(&PreviewCardRedirect);
	Hook((BYTE*)address,
		{ 0xE8, 0x39, 0xF7, 0xFF, 0xFF },							//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		&PreviewCardOriginal);
}


}
}