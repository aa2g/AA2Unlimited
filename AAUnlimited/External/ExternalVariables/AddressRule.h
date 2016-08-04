#pragma once

#include <Windows.h>

#include "General\ModuleInfo.h"

namespace ExtVars {


inline void* ApplyRule(const DWORD* offsetArr, unsigned int size) {
	DWORD start = General::GameBase + offsetArr[0];
	for (unsigned int i = 1; i < size; i++) {
		start = *(DWORD*)(start);
		if (start == NULL) return NULL;
		start += offsetArr[i];
	}
	return (void*)(start);
}


}