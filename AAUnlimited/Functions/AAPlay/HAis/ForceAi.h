#pragma once

#include <vector>
#include <string>
#include <array>
#include <ctime>

#include "External\ExternalClasses\HClasses\HGUIButton.h"
#include "External\ExternalClasses\HClasses\HInfo.h"
#include "BaseAi.h"

class ForceAi : public BaseAi  {
public:
	ForceAi();
	~ForceAi();


	void Initialize(ExtClass::HInfo* info) override;
	void Tick(ExtClass::HInfo* info) override;

	//constants (some of them)
	//https://docs.google.com/spreadsheets/d/1gwmoVpKuSuF0PtEPLEB17eK_dexPaKU106ShZEpBLhg/edit?pref=2&pli=1#gid=453712916 for names
	//in normal hetero h, these are dominant for the passive (female) one
	const std::vector<std::wstring> m_passiveDomNames{
		L"h_00_02", L"h_00_03", L"h_00_06", L"h_00_51" //standing Cunnil, face sitting, spread licking, standing cunnil,
		L"h_01_01", L"h_01_03", L"h_01_06", L"h_01_09", L"h_01_10", L"h_01_11", L"h_01_17",//lying down hand, palzuri, legs up, foot,sack,blindfolded,chestjob
		L"h_02_00", L"h_02_03", L"h_02_07", //mutual mas, 69, straddle, 
		L"h_03_15", L"h_03_14", L"h_03_02", L"h_03_11", L"h_03_50", //cowgirl, hugging cowgirl, legs fron cowgirl, cowgirl handjob, sitting in lap
		L"h_04_02", L"h_04_05" //reverse cowgirl, sitting in lap

		L"h_10_01",
		L"h_11_04",
		L"h_13_00"
	};
	//in normal hetero h, these are dominant for the active (male) one. Note that in yuri force, the forcer will start as dominant, but can be switched.
	const std::vector<std::wstring> m_activeDomNames{
		L"h_00_00", L"h_00_01", L"h_00_04", L"h_00_05", L"h_00_07", L"h_01_15", //groping, nip lick, piledriveranal teasing, anilingus, fingering, stand blow 2
		L"h_01_00", L"h_01_02", L"h_01_12", L"h_01_13", L"h_01_50",//lick stand, stand blow, sitting blow
		L"h_02_05", //thighjob
		L"h_03_00", L"h_03_12", L"h_03_01", L"h_03_13", L"h_03_09", L"h_03_07", L"h_03_10", L"h_03_04", L"h_03_06", L"h_03_05", L"h_03_16",
		//missionary, rasied hips, doggy, top down bottom up, on side, against wall, sitting on lap, doggy surface, doggy surface leg up, mission surface, 16=piledriver
		L"h_04_00", L"h_04_01", L"h_04_04", L"h_04_03", L"h_04_09", L"h_04_51", L"h_04_50",
		//mission, doggy, double V, against wall, top down bottom, on side, piledriver,

		L"h_10_00",
		L"h_11_00", L"h_11_01", L"h_11_03",
		L"h_13_03", //legs spread
		L"h_14_00", L"h_14_02"
		//facing each other, doggy style
	};

	enum STATE {
		BEGIN = 0, WATCH_INTRO = 1, FOREPLAY = 2, DOMINANCE = 3, PREFERENCES = 4, CLIMAX = 5, END = 6,

		INVALID = END+1
	};

	//variables
	std::array<std::vector<std::pair<DWORD, DWORD>>, 9> m_dominantPositionsActive; //for each category
	std::array<std::vector<std::pair<DWORD, DWORD>>, 9> m_dominantPositionsPassive; //for each category
	std::array<std::vector<std::pair<DWORD, DWORD>>, 9> m_preferencePositions;

	bool m_isYuri;
	bool m_isActive;
	DWORD m_forcerGender;
	DWORD m_forceeGender;
	ExtClass::HParticipant* m_forcer;
	ExtClass::HParticipant* m_forcee;
	DWORD m_aiState;

	void PickRandomDomPosition(ExtClass::HInfo* info, bool passive, bool active, bool allowForeplay, bool allowNormal, float climaxChance);
	void PickRandomPrefPosition(ExtClass::HInfo* info, bool allowForeplay, bool allowNormal, float climaxChance);

	struct State {
		DWORD m_nr;
		void(*m_initFunc)(State* state, ForceAi* thisPtr, ExtClass::HInfo* info);
		void(*m_tickFunc)(State* state, ForceAi* thisPtr, ExtClass::HInfo* info);
		DWORD(*m_endFunc)(State* state, ForceAi* thisPtr, ExtClass::HInfo* info);

		DWORD m_customValue; //is going to be set to 0 at init
	};

	friend struct State;

	static State states[INVALID];
};

