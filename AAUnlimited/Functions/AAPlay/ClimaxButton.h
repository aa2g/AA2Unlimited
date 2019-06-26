#pragma once


struct CLIMAX_BUTTON {
	int categoryNode = 0;
	int buttonNode = -1;
	bool anywayAdd = false;
	int climaxPoses[3] = { -1, -1 , -1 };
	int climaxCount = 0;
};

struct CLIMAX_NORMAL_BUTTON {
	int categoryNode = 0;
	int buttonNode = -1;
	int normalPoses[3] = { -1, -1 , -1 };
	int normalCount = 0;
};

struct CLIMAX_CAT_SCENARIO {
	bool isShift = false;
	int shiftVal = 0;
	int catId = 0;		// if !isOffset
	int buttonPos = -1;	// if !isOffset
	int climaxPoses[3][2] = { {-1, -1}, { -1, -1 }, { -1, -1 } };
	int climaxCount = 0;
};

namespace ClimaxButton {
	extern bool initialized[2];

	extern void StartClimaxPose();
	extern void StartNormalPose();
	extern void InitCfg();
	extern void Init();
}
