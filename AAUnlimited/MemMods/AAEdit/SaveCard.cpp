#include "SaveCard.h"

#include <Windows.h>

#include "External\AddressRule.h"
#include "MemMods\Hook.h"
#include "General\ModuleInfo.h"
#include "General\Buffer.h"
#include "General\Util.h"
#include "Functions\AAEdit\Globals.h"
#include "Functions\AAEdit\UnlimitedDialog.h"
#include "Files\Logger.h"

namespace EditInjections {
namespace SaveCard {


void __stdcall AddUnlimitData(wchar_t* fileName) {
	DWORD charDataRule[]{ 0x353254, 0x2C, 0 };
	AAEdit::g_currChar.m_char = (ExtClass::CharacterStruct*) ExtVars::ApplyRule(charDataRule);

	if(AAEdit::g_AAUnlimitDialog.IsSaveFilesSet()) {
		AAEdit::g_currChar.m_cardData.SaveOverrideFiles();
	}
	else {
		AAEdit::g_currChar.m_cardData.ClearOverrides();
	}
	if(AAEdit::g_AAUnlimitDialog.IsSaveEyesSet()) {
		auto& eyes = AAEdit::g_currChar.m_char->m_charData->m_eyes;
		if(eyes.bExtTextureUsed) {
			TCHAR buffer[256];
			mbtowc(buffer,eyes.texture,260);
			AAEdit::g_currChar.m_cardData.SetEyeTexture(0,buffer,true);
		}
		else {
			AAEdit::g_currChar.m_cardData.SetEyeTexture(0,NULL,false);
		}
	}
	if(AAEdit::g_AAUnlimitDialog.IsSaveEyeHighlightSet()) {
		auto& eyes = AAEdit::g_currChar.m_char->m_charData->m_eyes;
		if(eyes.bExtHighlightUsed) {
			TCHAR buffer[256];
			mbtowc(buffer,eyes.highlight,260);
			AAEdit::g_currChar.m_cardData.SetEyeHighlight(buffer);
		}
		else {
			AAEdit::g_currChar.m_cardData.SetEyeHighlight(NULL); //reset in case it was still set
		}
		
	}
	//open card
	HANDLE hFile = CreateFile(fileName, FILE_GENERIC_READ | FILE_GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE || hFile == NULL) return;

	if(true) {
		//write in png chunk
		//since we need to insert into the file, read it first
		DWORD hi,lo = GetFileSize(hFile,&hi);
		BYTE* fileBuffer = new BYTE[lo];
		ReadFile(hFile,fileBuffer,lo,&hi,NULL);
		//find iend
		BYTE* chunk = General::FindPngChunk(fileBuffer,lo,*(DWORD*)"IEND");
		if(chunk == NULL) {
			LOGPRIO(Logger::Priority::WARN) << "Failed to add Unlimited data to card at " << fileName << ": could not find IEND chunk\r\n";
			CloseHandle(hFile);
			delete[] fileBuffer;
			return;
		}	 
		//generate unlimited data buffer
		int size = 0;
		char* buffer = NULL;
		int retSize = AAEdit::g_currChar.m_cardData.ToBuffer(&buffer,&size,true,true);
		if (retSize == 0) {
			CloseHandle(hFile);
			delete[] fileBuffer;
			return;
		}

		//rewrite file
		LONG himove = 0;
		SetFilePointer(hFile,0,&himove,FILE_BEGIN);
		WriteFile(hFile,fileBuffer,(DWORD)(chunk-fileBuffer),&hi,NULL);
		WriteFile(hFile,buffer,(DWORD)(retSize),&hi,NULL);
		WriteFile(hFile,chunk,lo-(DWORD)(chunk-fileBuffer),&hi,NULL);

		delete[] buffer;
		delete[] fileBuffer;
	}
	else {
		//write to end
		//generate unlimited data buffer
		int size = 0;
		char* buffer = NULL;
		int retSize = AAEdit::g_currChar.m_cardData.ToBuffer(&buffer,&size,true,false);
		if (retSize == 0) {
			CloseHandle(hFile);
			return;
		}

		DWORD foo = 0;

		//read data offset from end
		SetFilePointer(hFile,-4,NULL,FILE_END);
		DWORD aa2DataOffset = 0;
		ReadFile(hFile,&aa2DataOffset,4,&foo,NULL);

		//add our data
		WriteFile(hFile,buffer,retSize,&foo,NULL);

		//write new offset to end
		aa2DataOffset += retSize + 4; //+4 cause the old offset is also between it
		WriteFile(hFile,&aa2DataOffset,4,&foo,NULL);
		delete[] buffer;
	}
	

	CloseHandle(hFile);
}

DWORD AddUnlimitDataOriginalFunction;
void __declspec(naked) AddUnlimitDataRedirect() {
	__asm {
		push eax //save file path

		push [esp+8]
		call [AddUnlimitDataOriginalFunction]
		push eax //save return value

		mov eax, [esp+4]
		push eax //push rescued parameter
		call AddUnlimitData

		pop eax //get return value back
		add esp, 4 //get rid of file path that was saved
		ret 4
	}
}


void AddUnlimitDataInject() {
	//the save card data function call. one stack parameter stdcall, as well as eax (wchar_t* fileName) and ecx (?)
	/*AA2Edit.exe+1BEC8 - 50                    - push eax
	AA2Edit.exe+1BEC9 - 8D 84 24 34050000     - lea eax,[esp+00000534]
	AA2Edit.exe+1BED0 - E8 EBA21000           - call AA2Edit.exe+1261C0	*/
	DWORD address = General::GameBase + 0x1BED0;
	DWORD redirectAddress = (DWORD)(&AddUnlimitDataRedirect);
	Hook((BYTE*)address,
	{ 0xE8, 0xEB, 0xA2, 0x10, 0x00 },						//expected values
	{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		&AddUnlimitDataOriginalFunction);
}



}
}