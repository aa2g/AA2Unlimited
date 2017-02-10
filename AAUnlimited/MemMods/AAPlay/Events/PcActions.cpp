#include "PcActions.h"

#include "MemMods/Hook.h"
#include "General/ModuleInfo.h"

namespace PlayInjections {
namespace PcActions {





void ClothesPickInjection() {
	//constantly called while the clothing pick dialog is open.
	//the jump is taken if a choice has been made.
	//byte ptr [esi+38] will be new clothes slot
	/*
	AA2Play v12 FP v1.4.0a.exe+D9628 - E8 C3190000           - call "AA2Play v12 FP v1.4.0a.exe"+DAFF0 { ->AA2Play v12 FP v1.4.0a.exe+DAFF0 }
	AA2Play v12 FP v1.4.0a.exe+D962D - C7 84 24 88000000 00000000 - mov [esp+00000088],00000000 { 0 }
	AA2Play v12 FP v1.4.0a.exe+D9638 - 83 7C 24 70 00        - cmp dword ptr [esp+70],00 { 0 }
	AA2Play v12 FP v1.4.0a.exe+D963D - 0F84 D6010000         - je "AA2Play v12 FP v1.4.0a.exe"+D9819 { ->AA2Play v12 FP v1.4.0a.exe+D9819 }
	*/

	//this is the call to the function above. ecx (edi) is the esi from the function above
	/*AA2Play v12 FP v1.4.0a.exe+D957A - 8B CF                 - mov ecx,edi
	AA2Play v12 FP v1.4.0a.exe+D957C - E8 5F000000           - call "AA2Play v12 FP v1.4.0a.exe"+D95E0 { ->AA2Play v12 FP v1.4.0a.exe+D95E0 }
	*/


}



}
}

/*
 
 15E200 // called when starting the day
 15DAA0 // called when period changes (25 times, once for every character? changes clothing to appropriate ones)


 */