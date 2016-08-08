#include "HInjections.h"
#include "MemMods/Hook.h"
#include "General/ModuleInfo.h"
#include "External/ExternalClasses.h"

#include "Functions/AAPlay/HAi.h"
#include "Functions/AAPlay/HButtonMove.h"

namespace PlayInjections {
namespace HPlayInjections {


bool (__stdcall *loc_OriginalTickFunction)(ExtClass::HInfo* info);

bool __stdcall TickRedirect(ExtClass::HInfo* hInfo) {
	HAi::PreTick(hInfo);
	bool retVal = loc_OriginalTickFunction(hInfo);
	HAi::PostTick(hInfo, retVal);
	HButtonMove::PostTick(hInfo, retVal);
	return retVal;
}

void TickInjection() {
	//			bool HTick(HInfo*);
	// Returns false if this tick ends h (end button pressed). stdcall.
	//AA2Play v12 FP v1.4.0a.exe+80DB0 - 53                    - push ebx
	//AA2Play v12 FP v1.4.0a.exe+80DB1 - E8 9ADDFFFF           - call "AA2Play v12 FP v1.4.0a.exe"+7EB50{ ->AA2Play v12 FP v1.4.0a.exe+7EB50 }
	DWORD address = General::GameBase + 0x80DB1;
	DWORD redirectAddress = (DWORD)(&TickRedirect);
	Hook((BYTE*)address,
		{ 0xE8, 0x9A, 0xDD, 0xFF, 0xFF },						//expected values
		{ 0xE8, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
		(DWORD*)(&loc_OriginalTickFunction));						//save old function to call in our redirect function
}


}
}