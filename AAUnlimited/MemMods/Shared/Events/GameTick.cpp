#include "StdAfx.h"


namespace GameTick {

int (__stdcall *orig_MsgHandler)(void *ptr, MSG *msg);
int (__stdcall *orig_GameTick)();

DWORD now;  // real time in ms since first tick in the game
DWORD tick; // game time in ticks

DWORD first_now;

HANDLE *hwnd;

std::vector<MsgFilterFunc> msg_filters;

void RegisterMsgFilter(MsgFilterFunc f) {
	msg_filters.push_back(f);
}

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


void ResetOnTitle() {
	if (General::IsAAPlay) {
		if (*(ExtVars::AAPlay::PlayerCharacterPtr()) == nullptr) {
			if (!(Shared::GameState::getIsInMainMenu())) {
				for (int i = 0; i < 25; i++) AAPlay::g_characters[i].Reset();
				Shared::GameState::setIsInMainMenu(true);
			}
		}
	}
}


int __stdcall GameTick() {
	if (tick == 0) {
		first_now = GetTickCount();
		LOGPRIONC(Logger::Priority::INFO) "Entering main game event loop\n";
		LUA_EVENT_NORET("first_tick", (DWORD)(*hwnd));
	}
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
	auto delayedEvents = Shared::GameState::GetDelayedEvents();
	for (auto it = delayedEvents->begin(); it != delayedEvents->end() && it->delayEnd < now;) {
		ThrowEvent(&(*it));
		delayedEvents->erase(it++);
	}

	ResetOnTitle();
	return orig_GameTick();
}

void Initialize() {
	hwnd = (HANDLE*)(General::GameBase + 0x34526C);
	DWORD call_MsgHandler = General::GameBase + 0x429D;
	DWORD call_GameTick = General::GameBase + 0x42BE;

	if (General::IsAAPlay) {
		call_MsgHandler = General::GameBase + 0x44A1;
		call_GameTick = General::GameBase + 0x44C2;
		hwnd = (HANDLE*)(General::GameBase + 0x368274);
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