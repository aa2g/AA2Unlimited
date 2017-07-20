#include <Windows.h>

#include "MemAlloc.h"
#include "Files\Config.h"
#include "MemMods\Hook.h"
#include "General\ModuleInfo.h"


namespace SharedInjections {
namespace MemAlloc {
static DWORD *iat_HeapAlloc;
static DWORD *iat_HeapFree;
static DWORD *iat_HeapSize;

static SIZE_T (__stdcall *orig_HeapSize)(HANDLE hHeap, DWORD dwFlags, LPCVOID lpMem);
static SIZE_T __stdcall InjectedHeapSize(HANDLE hHeap, DWORD dwFlags, LPCVOID lpMem) {
	return orig_HeapSize(hHeap, dwFlags, lpMem);
}

static LPVOID (__stdcall *orig_HeapAlloc)(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
static LPVOID __stdcall InjectedHeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes) {
	return orig_HeapAlloc(hHeap, dwFlags, dwBytes);
}

static BOOL (__stdcall *orig_HeapFree)(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem);
static BOOL __stdcall InjectedHeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem) {
	return orig_HeapFree(hHeap, dwFlags, lpMem);
}

void MemAllocInject() {
	if (General::IsAAEdit) {
		iat_HeapSize = (DWORD*)(General::GameBase + 0x4C423C);
		iat_HeapAlloc = (DWORD*)(General::GameBase + 0x4C4244);
		iat_HeapFree = (DWORD*)(General::GameBase + 0x4C4248);
	}
	else if (General::IsAAPlay) {
		iat_HeapSize = (DWORD*)(General::GameBase + 0x4E324C);
		iat_HeapAlloc = (DWORD*)(General::GameBase + 0x4E3244);
		iat_HeapFree = (DWORD*)(General::GameBase + 0x4E3248);
	}
	orig_HeapSize = (decltype(orig_HeapSize))*iat_HeapSize;
	*iat_HeapSize = (DWORD)&InjectedHeapSize;


	orig_HeapAlloc = (decltype(orig_HeapAlloc))*iat_HeapAlloc;
	*iat_HeapAlloc = (DWORD)&InjectedHeapAlloc;
	
	orig_HeapFree = (decltype(orig_HeapFree)) *iat_HeapFree;
	*iat_HeapFree = (DWORD)&InjectedHeapFree;
}
}
}
