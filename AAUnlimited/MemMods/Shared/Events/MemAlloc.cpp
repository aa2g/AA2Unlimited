#include "StdAfx.h"


namespace SharedInjections {
namespace MemAlloc {
static DWORD *iat_HeapAlloc;
static DWORD *iat_HeapFree;
static DWORD *iat_HeapSize;

static SIZE_T __stdcall InjectedHeapSize(HANDLE hHeap, DWORD dwFlags, LPCVOID lpMem) {
	return HeapSize(hHeap, dwFlags, lpMem);
}

static LPVOID __stdcall InjectedHeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes) {
	return HeapAlloc(hHeap, dwFlags, dwBytes);
}

static BOOL __stdcall InjectedHeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem) {
	return HeapFree(hHeap, dwFlags, lpMem);
}

void MemAllocInject() {
	if (General::IsAAEdit) {
		iat_HeapSize = (DWORD*)(General::GameBase + 0x4C423C);
		iat_HeapAlloc = (DWORD*)(General::GameBase + 0x4C4244);
		iat_HeapFree = (DWORD*)(General::GameBase + 0x4C4248);
	}
	else if (General::IsAAPlay) {
		iat_HeapSize = (DWORD*)(General::GameBase + 0x2E324C);
		iat_HeapAlloc = (DWORD*)(General::GameBase + 0x2E3254);
		iat_HeapFree = (DWORD*)(General::GameBase + 0x2E3258);
	}

	PatchIAT(iat_HeapSize, &InjectedHeapSize);
	PatchIAT(iat_HeapAlloc, &InjectedHeapAlloc);
	PatchIAT(iat_HeapFree, &InjectedHeapFree);

}
}
}
