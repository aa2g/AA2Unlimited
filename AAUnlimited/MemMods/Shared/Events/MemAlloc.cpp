#include "StdAfx.h"
#include "Hooks/WinAPI.h"

namespace MemAlloc {

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

	dumpheap1(GetProcessHeap(), "ProcessHeap");
	if (g_Lua_p) {
		LUA_SCOPE;
		int luakb = lua_gc(g_Lua.L(), LUA_GCCOUNT, 0);
		int luam = int(g_Lua["_alloced"].get()) / 1024;
		LOGPRIONC(Logger::Priority::SPAM) std::dec << luakb << "KiB in use by Lua objects, " << luam << "KiB by malloc\n";
	}
}

using namespace SharedInjections::WinAPI;

static LPVOID __stdcall MyHeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes) {
	dwFlags &= ~1;
	return HeapAlloc(hHeap, dwFlags, dwBytes);
}

static LPVOID __stdcall MyHeapReAlloc(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem, SIZE_T dwBytes) {
	dwFlags &= ~1;
	return HeapReAlloc(hHeap, dwFlags, lpMem, dwBytes);
}

static BOOL __stdcall MyHeapDestroy(HANDLE hHeap) {
	return true;
	//HeapDestroy(hHeap);
}

static HANDLE __stdcall MyHeapCreate(DWORD opt, size_t init, size_t max) {
	return (HANDLE)_get_heap_handle();
}

static BOOL __stdcall MyHeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem) {
	return HeapFree(hHeap, dwFlags, lpMem);
}
HookImport patches[] = {
	{ "HeapAlloc", &MyHeapAlloc, 0 },
	{ "HeapReAlloc", &MyHeapReAlloc,0 },
	{ "HeapFree",&MyHeapFree,0 },
	{ "HeapCreate",&MyHeapCreate,0 },
	{ "HeapDestroy",&MyHeapDestroy,0 },
	{0,0}
};

void Init() {
	HookImports(patches);
	ULONG HeapInformation;
	HeapInformation = 2;
	HANDLE heap = (HANDLE)_get_heap_handle();
	HeapSetInformation(heap,
		HeapCompatibilityInformation,
		&HeapInformation,
		sizeof(HeapInformation));
}

}
