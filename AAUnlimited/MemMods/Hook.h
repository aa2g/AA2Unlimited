#pragma once
#include <Windows.h>
#include <initializer_list>

namespace HookControl {
	/*
	* The following DWORD will be a relative offset of given size
	*/
	const DWORD RELATIVE_DWORD = (DWORD)-1;
	const DWORD RELATIVE_WORD = (DWORD)-2;
	const DWORD ABSOLUTE_DWORD = (DWORD)-3;
	const DWORD ABSOLUTE_WORD = (DWORD)-4;
	bool IsHookControl(DWORD d);
};

/*
 * Replaces the bytes at the target location by the values in newData, temporarily changing access rights in the process.
 * newData may contain HookControl codes to do other things than just replace bytes.
 * If not null, hookedValue will be filled with the original value that was replaced by the last HookControl.
 */
bool Hook(BYTE* location,std::initializer_list<DWORD> expected,std::initializer_list<DWORD> newData,DWORD* hookedValue);

/*
 * Used _exclusively_ for MemMods/Shared. Read carefully before applying this function.
 * redirectFunction must point to the function that is to be changed (or the jumptable entry).
 * toCall must be the absolute address of a function to be called.
 * The function that is being changed must indicate the place where the call is to be inserted
 * by using 5 nops if offset = -1. Else, offset is the place where the call will be inserted.
 * If offset is being used, the Function must not start with an unconditional jump. That is because
 * The first unconditional jump will be assumed to be the jumptable entry.
 */
void InsertRedirectCall(void* redirectFunction, void* toCall, int offset = -1);

/*
* Calculates the difference between currLoc and the targetLoc in such a way
* that a call whose offset starts at currLoc, if given the return value of this
* function as an offset, will jump to the target location.
* Version for DWORD offsets
*/
DWORD DWAbsoluteToRelative(BYTE* currLoc,DWORD targetLoc);
WORD WAbsoluteToRelative(BYTE* currLoc,DWORD targetLoc);
/*
* Calculates the target location of a jump of call
* whose offset starts at currLoc and has the value offset
*/
DWORD DWRelativeToAbsolute(BYTE* currLoc, DWORD offset);
DWORD WRelativeToAbsolute(BYTE* currLoc, WORD offset);

void InitializeHooks();