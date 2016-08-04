#include "OpenCard.h"

#include <Windows.h>
#include <intrin.h>

#include "MemMods\Hook.h"
#include "General\ModuleInfo.h"
#include "General\Util.h"
#include "Functions\AAEdit\Globals.h"
#include "Functions\AAEdit\UnlimitedDialog.h"

namespace EditInjections {
namespace OpenCard {


void __stdcall ReadUnlimitData(HANDLE hFile, DWORD /*illusionDataOffset*/) {
	//clear current data
	AAEdit::g_cardData.Reset();
	//first, find our unlimited data
	DWORD lo, hi;
	lo = GetFileSize(hFile, &hi);
	BYTE* needed = new BYTE[lo];
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	ReadFile(hFile, needed, lo, &hi, NULL);
	//find our png chunk
	BYTE* chunk = General::FindPngChunk(needed, lo, AAUCardData::PngChunkIdBigEndian);
	if (chunk != NULL) {
		AAEdit::g_cardData.FromBuffer((char*)chunk);
	}
	AAEdit::g_AAUnlimitDialog.Refresh();
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
	DWORD address = General::GameBase + 0x127E20;
	DWORD redirectAddress = (DWORD)(&ReadUnlimitDataRedirect);
	Hook((BYTE*)address,
		{ 0xFF, 0x15, 0xEC, 0x41, 0x4f, 0x00 },							//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress, 0x90 },	//redirect to our function
		NULL);
	ReadUnlimitDataOriginal = General::GameBase + 0x2C41EC;
}


}
}