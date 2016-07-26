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

bool Hook(BYTE* location,std::initializer_list<DWORD> expected,std::initializer_list<DWORD> newData,DWORD* hookedValue);

DWORD DWRelativeToAbsolute(BYTE* currLoc,DWORD offset);
WORD WRelativeToAbsolute(BYTE* currLoc,WORD offset);
DWORD DWAbsoluteToRelative(BYTE* currLoc,DWORD targetLoc);
WORD WAbsoluteToRelative(BYTE* currLoc,DWORD targetLoc);

void InitializeHooks();