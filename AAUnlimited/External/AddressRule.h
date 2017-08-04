#pragma once

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

//https://www.unknowncheats.me/forum/c-and-c/58694-address-logger.html

inline bool Match(const BYTE* pData, const BYTE* bMask, const char* szMask){
	for (;*szMask;++szMask, ++pData, ++bMask)
		if (*szMask == 'x' && *pData != *bMask)
			return false;
	return (*szMask) == NULL;
}


inline DWORD FindPattern(DWORD dwAddress, DWORD dwLen, BYTE *bMask, char * szMask){
	for (DWORD i = 0; i < dwLen; i++)
		if (Match((BYTE*)(dwAddress + i), bMask, szMask))
			return (DWORD)(dwAddress + i);

	return 0;
}

}