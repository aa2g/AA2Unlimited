#include "TanSlotUnlimited.h"
#include "MemMods/Hook.h"
#include "General/ModuleInfo.h"
#include "External/ExternalClasses.h"

/*
 * For the most part, this is the same as the AAPlay version (they compiled the same code i guess, just some registers are different),
 * so this is mostly cpynpasted with some values changed.
 */

#define TAN_MAX_SLOT 255
 //additional slotes saved are nSlots-5, as the first 5 are done by the game (we take over the 6th)
#define TAN_ADDITIONAL_SLOTS (TAN_MAX_SLOT+1-5)
#define TAN_MAX_LINE (TAN_ADDITIONAL_SLOTS+14)

namespace EditInjections { 
namespace TanSlotUnlimit {


/****************************************/
/* general variables for this injection */
/****************************************/
int loc_currLine = TAN_MAX_LINE; //used to read the file
int loc_currSlot = 0; //used for insertion
DWORD loc_paBuffer[TAN_ADDITIONAL_SLOTS][8]; //256 slots, minus 5 (we put the 6th one in here as well for simplicitys sake)

/*************************************/
/* PlayInjections for Tan load from file */
/*************************************/
DWORD LoadLoopStartOriginalFunction; //function that loads the line from file

void LoadLoopInitialize() {
	loc_currLine = 14; //start on line 14 in the file
	loc_currSlot = 5; //gonna start with 5 (6th tan, 0-based index)
						//this is the constant that pa structs point to when they are empty, so we have to memset our array to this
						//AA2Edit.exe+0x36F650
	DWORD address = General::GameBase + 0x36F650;
	for (int i = 0; i < TAN_ADDITIONAL_SLOTS; i++) {
		for (int j = 0; j < 8; j++) {
			loc_paBuffer[i][j] = address;
		}
	}
}

void __declspec(naked) LoadLoopStartRedirect() {
	_asm {
		cmp[loc_currLine],TAN_MAX_LINE					//check if we are allready initialized
		jne LoadLoopStartRedirect_ChangeParam
		pushad
		call LoadLoopInitialize
		popad
		LoadLoopStartRedirect_ChangeParam :
		mov eax,[loc_currLine]					//change current line parameter...
		mov [esp+4],eax						//...to our local variable
		jmp [LoadLoopStartOriginalFunction]		//redirect to original function
	}
}

void LoadLoopStartInject() {
	//reads a line from the file, line 14 in this case. We will call it multiple times, changing the 14 to different lines.
	//note that this function returns 0 in al if the line was not found, and else 1 
	/*AA2Edit.exe+115FF6 - 6A 10                 - push 10 { 16 }
	AA2Edit.exe+115FF8 - 6A 0E                 - push 0E { 14 }
	AA2Edit.exe+115FFA - 8D 8C 24 8C010000     - lea ecx,[esp+0000018C]
	AA2Edit.exe+116001 - E8 CA900600           - call AA2Edit.exe+17F0D0
	*/

	DWORD address = General::GameBase + 0x116001;
	DWORD redirectAddress = (DWORD)(&LoadLoopStartRedirect);
	Hook((BYTE*)address,
	{ 0xE8, 0xCA, 0x90, 0x06, 0x00 },						//expected values
	{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		&LoadLoopStartOriginalFunction);
}

void __declspec(naked) LoadLoopPaPointerRedirect() {
	_asm {
		mov esi,[loc_currLine]
		sub esi,14				//get offset in array (line 14 is first)
		lea esi,[esi*8]		//skip 8 images, each of which is described
		lea esi,[esi*4]		//by a 4 byte pointer
		add esi,OFFSET loc_paBuffer
		ret
	}
}

void LoadLoopPaPointerInject() {
	//in this part, esi made to point to the pa-struct on the stack that will hold the pointers to the texture structs
	//generated out of the tan info. Note that this buffer is only so big, so we will have to make it point to a custom
	//buffer once we go over the line 14 (slot 6)
	//(in case youre wondering why i call it Pa Pointer, thats because the first times i saw this array, the pointer values
	// spelled Pa in memory)
	/*AA2Edit.exe+11600B - 8D 74 24 60           - lea esi,[esp+60]
	AA2Edit.exe+11600F - 8D 5F 03              - lea ebx,[edi+03]
	AA2Edit.exe+116012 - C7 44 24 18 08000000  - mov [esp+18],00000008 { 8 }
	AA2Edit.exe+11601A - 8D 9B 00000000        - lea ebx,[ebx+00000000]
	*/

	//well, there is a hotpatch place in there... how about we just use it
	DWORD address = General::GameBase + 0x11601A;
	DWORD redirectAddress = (DWORD)(&LoadLoopPaPointerRedirect);
	Hook((BYTE*)address,
		{ 0x8D, 0x9B, 0x00, 0x00, 0x00, 0x00 },							//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress, 0x90 },	//redirect to our function
		NULL);

}

DWORD LoadLoopEndTarget;
DWORD LoadLoopEndRestartTarget;
void __declspec(naked) LoadLoopEndRedirect() {
	_asm {
		inc[loc_currLine]
		cmp[loc_currLine],TAN_MAX_LINE //last line was done (251+14)
		jb LoadLoopEndRedirect_Loop
		jmp dword ptr[LoadLoopEndTarget]
		LoadLoopEndRedirect_Loop:

		jmp dword ptr[LoadLoopEndRestartTarget]
	}
}

void LoadLoopEndInject() {
	//at the end, 2 things happen:
	//1: add esi, 28 puts esi to the next entry in the Pa array. The original Pa-array is Pa[imagePart][slot], which is retarded and obviously
	//   doesnt work if i make a bigger array, so we will need to change this part once we changed to our own pa buffer.
	//2: the last jmp instruction ends the loop, we will need to loop back up to load more lines.
	//(the jne is counting the image parts that make up one tan)
	/*AA2Edit.exe+11606C - 83 C6 28              - add esi,28 { 40 }
	AA2Edit.exe+11606F - 83 6C 24 18 01        - sub dword ptr [esp+18],01 { 1 }
	AA2Edit.exe+116074 - 75 AA                 - jne AA2Edit.exe+116020
	AA2Edit.exe+116076 - E9 76030000           - jmp AA2Edit.exe+1163F1
	*/

	//since we always use our own pa struct and not the stack one, we can just patch the add esi, 28 to fulfill 1
	DWORD address = General::GameBase + 0x11606C;
	Hook((BYTE*)address,
		{ 0x83, 0xC6, 0x28 },
		{ 0x83, 0xC6, 0x04 },	//our images are next to each other, so just add 4 to get to the next image
		NULL);

	//second, at the jmp, we need to loop back up if this is not the last line
	address = General::GameBase + 0x116076;
	DWORD redirectAddress = (DWORD)(&LoadLoopEndRedirect);
	Hook((BYTE*)address,
		{ 0xE9, 0x76, 0x03, 0x00, 0x00 },
		{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress },
		&LoadLoopEndTarget);

	//also, if we are not done yet, we will start back to the parameter push of that line read function
	//*AA2Edit.exe+115FF6 - 6A 10                 - push 10 { 16 }
	LoadLoopEndRestartTarget = General::GameBase + 0x115FF6;
}

/************************************/
/* PlayInjections to insert loaded tans */
/************************************/
DWORD InsertLoopOriginalCall;
void __declspec(naked) InsertLoopCallRedirect() {
	_asm {
		mov eax,[loc_currSlot]
		mov[esp+0x1C],eax				//note that its not 1C cause of the call
		mov edi,eax
		sub edi,5						//also, this code again
		lea edi,[edi*8]
		lea edi,[edi*4]
		lea edi,[edi+esi*4]				//we reference an image part now, esi is the image part
		add edi,OFFSET loc_paBuffer
		jmp[InsertLoopOriginalCall]
	}
}

void InsertLoopCall() {
	//this function inserts the pa struct in edi into the tan slot in [esp+18], so thats what we are going to have to change
	/*AA2Edit.exe+116702 - C7 44 24 18 05000000  - mov [esp+18],00000005 { 5 }
	AA2Edit.exe+11670A - E8 619A0200           - call AA2Edit.exe+140170 { sets a certain slot to the value it seems (5 being the 
	slot here). returns pointer for given slot in some list
		}
	*/
	DWORD address = General::GameBase + 0x11670A;
	DWORD redirectAddress = (DWORD)(&InsertLoopCallRedirect);
	Hook((BYTE*)address,
	{ 0xE8, 0x61, 0x9A, 0x02, 0x00 },
	{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },
		&InsertLoopOriginalCall);
};

DWORD InsertLoopStartTarget;
DWORD InsertLoopImageTarget;
void __declspec(naked) InsertLoopEndRedirect() {
	_asm {
		inc[loc_currSlot]
		cmp[loc_currSlot],TAN_MAX_SLOT
		jbe InsertLoopEndRedirect_LoopSlot
		mov[loc_currSlot],5					//restart at slot 5 for next image
											//original code
		inc esi
		cmp esi,8
		jl InsertLoopEndRedirect_LoopImage
		ret //return normally
		InsertLoopEndRedirect_LoopSlot :
		mov eax,[InsertLoopStartTarget]
		mov[esp],eax
		ret								//exit at start of loop
		InsertLoopEndRedirect_LoopImage :
		mov eax,[InsertLoopImageTarget]
		mov[esp],eax
		ret
	}
}
		 
void InsertLoopEnd() {
	//this is the end of the function. esi counts the image parts (cause for some retarded reason, this time the loop is the other way around).
	//what we want to do is loop BEFORE the inc to esi occurs.
	/*AA2Edit.exe+116766 - 46                    - inc esi
	AA2Edit.exe+116767 - 83 FE 08              - cmp esi,08 { 8 }
	AA2Edit.exe+11676A - 0F8C E0FCFFFF         - jl AA2Edit.exe+116450 { end of giant loop at copys tans }
	*/

	//this is the start of this loop. we wanna jump there.
	/*AA2Edit.exe+1166D7 - 8B 54 24 1C           - mov edx,[esp+1C] { this is where it starts }
	AA2Edit.exe+1166DB - 8B 82 3C010000        - mov eax,[edx+0000013C]
	AA2Edit.exe+1166E1 - 8D 7C 3C 60           - lea edi,[esp+edi+60] { edi = 0x28*esi at this point, an index in the PaPa struct on the stack }
	AA2Edit.exe+1166E5 - 83 FE 07              - cmp esi,07 { 7 }
	AA2Edit.exe+1166E8 - 77 7C                 - ja AA2Edit.exe+116766
	*/


	//thats a really stupid place to hook
	DWORD address = General::GameBase + 0x116766;
	DWORD redirectAddress = (DWORD)(&InsertLoopEndRedirect);
	Hook((BYTE*)address,
	{ 0x46,
		0x83, 0xFE, 0x08,
		0x0F, 0x8C, 0xE0, 0xFC, 0xFF, 0xFF },
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress, 0x90, 0x90, 0x90, 0x90, 0x90 },
		NULL);
	InsertLoopStartTarget = General::GameBase + 0x1166D7;
	InsertLoopImageTarget = General::GameBase + 0x116450;
}


}
}