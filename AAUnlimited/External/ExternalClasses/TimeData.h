#pragma once

#include <Windows.h>

namespace ExtClass {

	enum PeriodId {
		MORNING_ROOM = 1,
		MORNING_SCHOOL = 2,
		FIRST_LESSON = 3,
		FIRST_BREAK = 4,
		SPORTS = 5,
		SECOND_BREAK = 6,
		CLUB = 7,
		EVENING_SCHOOL = 8,
		EVENING_ROOM = 9,
		SLEEP = 10
	};

	enum DayName {
		SUNDAY = 0,
		MONDAY = 1,
		TUESDAY = 2,
		WEDNESDAY = 3,
		THURSDAY = 4,
		FRIDAY = 5,
		SATURDAY = 6
	};

#pragma pack(push, 1)

class TimeData {
public:
	//BYTE m_unknown[0x20];
	int currentPeriod; //(10 = sleep, 1 = day, 2 = "nothing to talk", 3 = first lesson,
						//4 = first break, 5 = sports, 6 = second break, 7 = club, 8 = end, 9 = home again)
	int day;			//0=sunday, 1=monday ... 6 = saturday
	BYTE nDays;			//total amount of days passed

#define LUA_CLASS ExtClass::TimeData
	static inline void bindLua() {
		LUA_BIND(currentPeriod)
		LUA_BIND(day)
		LUA_BIND(nDays)
	}
#undef LUA_CLASS
};

#pragma pack(pop)


}