#include "HInjections.h"
#include "MemMods/Hook.h"
#include "General/ModuleInfo.h"
#include "External/ExternalClasses.h"
#include "External/ExternalClasses/Frame.h"

#include "Functions/AAPlay/HAi.h"
#include "Functions/AAPlay/HButtonMove.h"
#include "Functions/AAPlay/Facecam.h"

namespace PlayInjections {
namespace HPlayInjections {


bool (__stdcall *loc_OriginalTickFunction)(ExtClass::HInfo* info);


//take note that these ticks might be called multiple times even after returning contScene = false
bool __stdcall TickRedirect(ExtClass::HInfo* hInfo) {
	HAi::PreTick(hInfo);
	bool contScene = loc_OriginalTickFunction(hInfo);
	HAi::PostTick(hInfo,contScene);
	HButtonMove::PostTick(hInfo,contScene);
	Facecam::PostTick(hInfo,contScene);
	return contScene;
}

void TickInjection() {
	//			bool HTick(HInfo*);
	// Returns false if this tick ends h (end button pressed). stdcall.
	//AA2Play v12 FP v1.4.0a.exe+80DB0 - 53                    - push ebx
	//AA2Play v12 FP v1.4.0a.exe+80DB1 - E8 9ADDFFFF           - call "AA2Play v12 FP v1.4.0a.exe"+7EB50{ ->AA2Play v12 FP v1.4.0a.exe+7EB50 }
	//this function returns false if h actually ends (not just if the button was pressed), but might be called multiple times
	//despite h ending
	//AA2Play v12 FP v1.4.0a.exe+3B5C7 - 50                    - push eax
	//AA2Play v12 FP v1.4.0a.exe+3B5C8 - E8 03510400           - call "AA2Play v12 FP v1.4.0a.exe"+806D0 { ->AA2Play v12 FP v1.4.0a.exe+806D0 }
	DWORD address = General::GameBase + 0x3B5C8;
	DWORD redirectAddress = (DWORD)(&TickRedirect);
	Hook((BYTE*)address,
		{ 0xE8, 0x03, 0x51, 0x04, 0x00 },						//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		(DWORD*)(&loc_OriginalTickFunction));						//save old function to call in our redirect function
}

void __stdcall FocusCameraEvent(ExtClass::Frame* bone) {
	Facecam::AdjustCamera(bone);
}

DWORD loc_OriginalFocusTickFunction;
void __declspec(naked) FocusCameraRedirect() {
	__asm {
		push [esp+4]
		call [loc_OriginalFocusTickFunction]
		pushad
		push eax
		call FocusCameraEvent
		popad
		ret 4
	}
}

void FocusCameraInjection() {
	//called when q/w/e is pressed, return value is the bone struct that will be focused. one stack parameter
	//AA2Play v12 FP v1.4.0a.exe+80111 - E8 0ABEF9FF           - call "AA2Play v12 FP v1.4.0a.exe"+1BF20{ this call determines where the camera goes }
	DWORD address = General::GameBase + 0x80111;
	DWORD redirectAddress = (DWORD)(&FocusCameraRedirect);
	Hook((BYTE*)address,
		{ 0xE8, 0x9A, 0xDD, 0xFF, 0xFF },						//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		(DWORD*)(&loc_OriginalFocusTickFunction));

}

}
}