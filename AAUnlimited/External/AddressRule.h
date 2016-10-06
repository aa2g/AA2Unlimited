#pragma once

#include <Windows.h>

#include "General\ModuleInfo.h"

namespace ExtVars {

/*
 * Follows a chain of pointers with offsets defined in the parameter.
 * The first element must be the offset of the first pointer relative of the game module.
 * the last offset is added without dereferenciation.
 */
template<int size>
inline void* ApplyRule(const DWORD(&offsetArr)[size]) {
	DWORD start = General::GameBase + offsetArr[0];
	for (unsigned int i = 1; i < size; i++) {
		start = *(DWORD*)(start);
		if (start == NULL) return NULL;
		start += offsetArr[i];
	}
	return (void*)(start);
}

/*
 * Follows a chain of pointers with offsets defined in the parameter.
 * the first offset is added to the startAddress before the first dereferenciation is performed.
 * the last offset is added without dereferenciation.
 */
template<int size>
inline void* ApplyRule(void* startAddress, const DWORD(&offsetArr)[size]) {
	DWORD start = (DWORD)startAddress + offsetArr[0];
	for (unsigned int i = 1; i < size; i++) {
		start = *(DWORD*)(start);
		if (start == NULL) return NULL;
		start += offsetArr[i];
	}
	return (void*)(start);
}

}