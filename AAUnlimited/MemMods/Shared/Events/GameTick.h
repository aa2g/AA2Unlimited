#include <windows.h>

namespace GameTick {
void Initialize();

typedef bool(*MsgFilterFunc)(MSG *);
void RegisterMsgFilter(MsgFilterFunc);
extern DWORD now;  // real time in ms since first tick in the game
extern DWORD tick; // game time in ticks
extern HANDLE *hwnd;

/*
#include <set>
extern std::set<uint64_t> timers;
High precision if SetTimer/KillTimer proves to be too heavy 
template<typename T>
class Timer {
public:;
	T fun;
	uint64_t handle;
	inline Timer(T fun) {
		this->fun = fun;
		handle = 0;
	}

	inline Timer(int when, T fun) {
		this->fun = fun;
		Schedule(when);
	}

	inline void Cancel() {
		if (handle) {
			timers.erase(handle);
		}
		handle = 0;
	}

	inline void Schedule(int when) {
		Cancel();
		handle = AddTimer(when, fun);
	}

	inline ~Timer() {
		Cancel();
	}
};
*/

}