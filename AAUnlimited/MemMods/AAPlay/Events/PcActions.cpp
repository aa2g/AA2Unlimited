#include "PcActions.h"

#include "MemMods/Hook.h"
#include "General/ModuleInfo.h"

namespace PlayInjections {
namespace PcActions {


BYTE __stdcall ClothesPickEvent(BYTE newClothes) {
	return newClothes;
}


DWORD ClothesPickFunction;
void __declspec(naked) ClothesPickRedirect() {
	__asm {
		push [esp+0x0C]
		push [esp+0x0C]
		push [esp+0x0C]
		call [ClothesPickFunction]
		add esp, 0x0C
		movzx eax, byte ptr [esp+0x24 + 4]
		push eax
		call ClothesPickRedirect
		mov [esp+0x24 + 4], al
		ret
	}
}

void ClothesPickInjection() {
	//this is the call to the function above. ecx (edi) is the esi from the function above
	/*AA2Play v12 FP v1.4.0a.exe+D957A - 8B CF                 - mov ecx,edi
	AA2Play v12 FP v1.4.0a.exe+D957C - E8 5F000000           - call "AA2Play v12 FP v1.4.0a.exe"+D95E0 { ->AA2Play v12 FP v1.4.0a.exe+D95E0 }
	*/

	//constantly called while the clothing pick dialog is open.
	//the jump is taken if a choice has been made.
	//byte ptr [esi+38] will be new clothes slot
	/*
	AA2Play v12 FP v1.4.0a.exe+D9628 - E8 C3190000           - call "AA2Play v12 FP v1.4.0a.exe"+DAFF0 { ->AA2Play v12 FP v1.4.0a.exe+DAFF0 }
	AA2Play v12 FP v1.4.0a.exe+D962D - C7 84 24 88000000 00000000 - mov [esp+00000088],00000000 { 0 }
	AA2Play v12 FP v1.4.0a.exe+D9638 - 83 7C 24 70 00        - cmp dword ptr [esp+70],00 { 0 }
	AA2Play v12 FP v1.4.0a.exe+D963D - 0F84 D6010000         - je "AA2Play v12 FP v1.4.0a.exe"+D9819 { ->AA2Play v12 FP v1.4.0a.exe+D9819 }
	*/
	//...
	//where dl is the picked clothing (character is current player, obviously)
	/*AA2Play v12 FP v1.4.0a.exe+D9679 - 51                    - push ecx
	AA2Play v12 FP v1.4.0a.exe+D967A - 68 50A65600           - push "AA2Play v12 FP v1.4.0a.exe"+32A650{ ["SEL_%d"] }
	AA2Play v12 FP v1.4.0a.exe+D967F - 50                    - push eax
	AA2Play v12 FP v1.4.0a.exe+D9680 - E8 B71B1B00           - call "AA2Play v12 FP v1.4.0a.exe"+28B23C { ->AA2Play v12 FP v1.4.0a.exe+28B23C }
	AA2Play v12 FP v1.4.0a.exe+D9685 - 8A 54 24 24           - mov dl,[esp+24]
	AA2Play v12 FP v1.4.0a.exe+D9689 - 8B 46 04              - mov eax,[esi+04]
	AA2Play v12 FP v1.4.0a.exe+D968C - 88 56 38              - mov [esi+38],dl
	*/
	DWORD address = General::GameBase + 0xD9680;
	DWORD redirectAddress = (DWORD)(&ClothesPickRedirect);
	Hook((BYTE*)address,
		{ 0xE8, 0xB7, 0x1B, 0x1B, 0x00 },								//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		&ClothesPickFunction);
	


}



}
}

/*
 
 15E200 // called when starting the day
 15DAA0 // called when period changes (25 times, once for every character? changes clothing to appropriate ones)


 */