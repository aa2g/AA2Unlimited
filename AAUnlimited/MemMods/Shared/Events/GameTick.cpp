#include "General/ModuleInfo.h"
#include "MemMods/Hook.h"
#include <stdint.h>
#include <map>
#include <set>
#include <vector>
#include "GameTick.h"


namespace GameTick {

int (__stdcall *orig_MsgHandler)(void *ptr, MSG *msg);
int (__stdcall *orig_GameTick)();

DWORD now;  // real time in ms since first tick in the game
DWORD tick; // game time in ticks

DWORD first_now;

std::vector<MsgFilterFunc> msg_filters;

void __stdcall MsgFilter(void *ptr, MSG *msg) {
	for (auto it : msg_filters) {
		if (it(msg)) return;
	}
	if (!orig_MsgHandler(ptr, msg)) {
		TranslateMessage(msg);
		DispatchMessageW(msg);
	}
}

/*
std::set<uint64_t> timers;
void AddTimer(int when, void *fn) {
	uint64_t k = ((uint64_t)(now + when) << 32) | (uint64_t)fn;
	timers.insert(k);
}
*/

int __stdcall GameTick() {
	if (tick == 0)
		first_now = GetTickCount();
	now = GetTickCount() - first_now;
	tick++;
	/*
	auto it = timers.begin();
	while (it != timers.end()) {
		DWORD expire = *it >> 32;
		DWORD fptr = *it;
		int (*func)();
		func = decltype(func)(fptr);
		if (expire <= now) {
			int ret = func();
			if (ret > 0)
				AddTimer(ret, func);
			timers.erase(it);
		}
		else break;
	}*/
	return orig_GameTick();
}

void Initialize() {
	DWORD call_MsgHandler = General::GameBase + 0x429D;
	DWORD call_GameTick = General::GameBase + 0x427C;

	if (General::IsAAPlay) {
		call_MsgHandler = General::GameBase + 0x44A1;
		call_GameTick = General::GameBase + 0x44C2;
	}

	Hook((BYTE*)call_MsgHandler,
	// call msghandler; test eax, eax; jnz loop
	{ 0xe8, HookControl::ANY_DWORD, 0x85, 0xc0, 0x75 }, 
	{ 0xE8, HookControl::RELATIVE_DWORD, (DWORD)&MsgFilter, 0x90, 0x90, 0xeb },
		(DWORD*)&orig_MsgHandler);

	Hook((BYTE*)call_GameTick,
	// call gametick
	{ 0xe8, HookControl::ANY_DWORD },
	{ 0xE8, HookControl::RELATIVE_DWORD, (DWORD)&GameTick },
		(DWORD*)&orig_GameTick);
}


}