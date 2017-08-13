#include "StdAfx.h"


namespace MemAlloc {
static DWORD *iat_HeapAlloc;
static DWORD *iat_HeapFree;
static DWORD *iat_HeapSize;

static void dumpheap1(HANDLE h, const char *hn) {
	size_t commited = 0, uncommited = 0, busy = 0, overhead = 0, entries = 0;
	if (!h) return;
	PROCESS_HEAP_ENTRY e;
	e.lpData = 0;
	HeapLock(h);
	while (HeapWalk(h, &e)) {
		if (e.wFlags & PROCESS_HEAP_REGION) {
			commited += e.Region.dwCommittedSize;
			uncommited += e.Region.dwUnCommittedSize;
		}
		else if (e.wFlags & PROCESS_HEAP_ENTRY_BUSY) {
			busy += e.cbData;
			overhead += e.cbOverhead;
			entries++;
		}
	}
	HeapUnlock(h);
	LOGPRIONC(Logger::Priority::SPAM) std::dec << hn << ": "
		<< entries << " entries, "
		<< (busy / 1024 / 1024) << "MiB busy, "
		<< (overhead / 1024 / 1024) << "MiB overhead, "
		<< (commited / 1024 / 1024) << "MiB commited, "
		<< (uncommited / 1024 / 1024) << "MiB uncommited\n";
}

void dumpheap() {
	LOGPRIONC(Logger::Priority::SPAM) "----- DUMPING HEAP -----\n";

	dumpheap1(*Shared::IllusionMemAllocHeap, "IllusionHeap");
	dumpheap1(GetProcessHeap(), "ProcessHeap");
	if (g_Lua_p) {
		LUA_SCOPE;
		int luakb = lua_gc(g_Lua.L(), LUA_GCCOUNT, 0);
		int luam = int(g_Lua["_alloced"].get()) / 1024;
		LOGPRIONC(Logger::Priority::SPAM) std::dec << luakb << "KiB in use by Lua objects, " << luam << "KiB by malloc\n";
	}
	//	dumpheap1((HANDLE)_get_heap_handle(), "CRTHeap");
}

static SIZE_T __stdcall InjectedHeapSize(HANDLE hHeap, DWORD dwFlags, LPCVOID lpMem) {
	return HeapSize(hHeap, dwFlags, lpMem);
}

static LPVOID __stdcall InjectedHeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes) {
	dwFlags &= ~1;
	return HeapAlloc(hHeap, dwFlags, dwBytes);
}

static BOOL __stdcall InjectedHeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem) {
	return HeapFree(hHeap, dwFlags, lpMem);
}

void Init() {
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

	// Illusion heap is forced to process one to reduce fragmentation
	// of address space. Comment this out to use separate heaps
	// for debugging.
#if 1
	*Shared::IllusionMemAllocHeap = (HANDLE)_get_heap_handle();
#endif
}
}
