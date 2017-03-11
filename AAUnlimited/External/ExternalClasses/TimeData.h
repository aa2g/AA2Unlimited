#pragma once

#include <Windows.h>

namespace ExtClass {

#pragma pack(push, 1)

class TimeData {
public:
	BYTE m_unknown[0x20];
	int currentPeriod; //(0 = sleep, 1 = day, 2 = "nothing to talk", 3 = first lesson,
						//4 = first break, 5 = sports, 6 = second break, 7 = club, 8 = end, 9 = home again)
	int day;			//0=sunday, 1=monday ... 6 = saturday
	int nDays;			//total amount of days passed
};

#pragma pack(pop)


}