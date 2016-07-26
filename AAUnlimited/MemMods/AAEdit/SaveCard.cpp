#include "SaveCard.h"

#include <Windows.h>

#include "MemMods\Hook.h"
#include "General\ModuleInfo.h"
#include "Functions\AAEdit\Globals.h"

namespace EditInjections {
namespace SaveCard {


void __cdecl AddUnlimitData(HANDLE h) {
	static const BYTE IENDPart[] = { 0,0,0,0, 0x49,0x45,0x4E,0x44, 0xAE,0x42,0x60,0x82 };

	//write extended card data
	char* buffer = NULL;
	int size = 0;
	DWORD written;
	int partSize = AAEdit::g_cardData.ToBuffer(&buffer,&size,true);
	if(buffer != NULL) {
		WriteFile(h,buffer,partSize,&written,0);
	}

	//write iend part
	WriteFile(h,IENDPart,sizeof(IENDPart),&written,0);
}

DWORD AddUnlimitDataOriginalFunction;
void __declspec(naked) AddUnlimitDataRedirect() {
	_asm {
		sub esi, 0xC	//remove IEND from buffer
		push 00			//copy parameters
		push ecx
		push edi
		call [AddUnlimitDataOriginalFunction]
		add esp, 0xC	//note that its a _cdecl function
		push edi		//edi is still file handle
		call AddUnlimitData
		add esp, 4
		ret				//again, original function was cdecl
	}
}

void AddUnlimitDataInject() {
	//this code adds the png-part of a card (ecx = buffer, size = esi) to the file handle (edi).
	//note that the call has 3 stack parameters as well as esi and edi and is _cdecl. its a really awkward function...
	/*AA2Edit.exe+1262D0 - 8B 4B 34              - mov ecx,[ebx+34]
	AA2Edit.exe+1262D3 - 8B 73 30              - mov esi,[ebx+30]
	AA2Edit.exe+1262D6 - 6A 00                 - push 00 { 0 }
	AA2Edit.exe+1262D8 - 51                    - push ecx
	AA2Edit.exe+1262D9 - 57                    - push edi
	AA2Edit.exe+1262DA - E8 51CF0800           - call AA2Edit.exe+1B3230 { first writing chunk }
	*/
	DWORD address = General::GameBase + 0x1262DA;
	DWORD redirectAddress = (DWORD)(&AddUnlimitDataRedirect);
	Hook((BYTE*)address,
		{ 0xE8, 0x51, 0xCF, 0x08, 0x00 },						//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		&AddUnlimitDataOriginalFunction);
}


}
}