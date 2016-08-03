#include "ForceAi.h"

using namespace ExtClass;

ForceAi::ForceAi() {
}


ForceAi::~ForceAi() {
}

void ForceAi::PickRandomDomPosition(ExtClass::HInfo* info, bool passive, bool active, bool allowForeplay, bool allowNormal, float climaxChance) {
	int chosenPassiveActive = 0;
	size_t nNormalPoses = 0;
	size_t nClimaxPoses = 0;


	auto countPoses = [this, passive, active](DWORD start, DWORD end) -> size_t {
		size_t cnt = 0;
		for (DWORD i = start; i <= end; i++) {
			if(passive) cnt += m_dominantPositionsPassive[i].size();
			if(active)  cnt += m_dominantPositionsActive[i].size();
		}
		return cnt;
	};
	auto findPose = [this, passive, active, info](size_t& chosen, DWORD start, DWORD end) -> int {
		for (int i = start; i <= end; i++) {
			if (passive) {
				size_t size = m_dominantPositionsPassive[i].size();
				if (chosen < size) {
					auto& pos = m_dominantPositionsPassive[i][chosen];
					info->m_hPosButtons[pos.first].m_arrButtonList[pos.second]->Press();
					return 1;
				}
				chosen -= size;
			}
			if (active) {
				size_t size = m_dominantPositionsActive[i].size();
				if (chosen < size) {
					auto& pos = m_dominantPositionsActive[i][chosen];
					info->m_hPosButtons[pos.first].m_arrButtonList[pos.second]->Press();
					return 2;
				}
				chosen -= size;
			}
			return 0;
		}
	};
	
	
	if (allowForeplay) nNormalPoses += countPoses(HCAT_FOREPLAY_FEMALE, HCAT_FOREPLAY_MUTUAL);
	if (allowNormal) nNormalPoses += countPoses(HCAT_FRONTAL, HCAT_BACK);
	if (climaxChance > 0) {
		if (allowForeplay) nClimaxPoses += countPoses(HCAT_CLIMAX_FOREPLAY_FEMALE, HCAT_CLIMAX_FOREPLAY_MALE);
		if (allowNormal ) nClimaxPoses += countPoses(HCAT_CLIMAX_FRONTAL, HCAT_CLIMAX_BACK);
	}
	float f = General::GetRandomFloat(0, 1);
	if (nClimaxPoses > 0 && f <= climaxChance) {
		size_t randChoice = rand() % nClimaxPoses;
		if (allowForeplay && (chosenPassiveActive = findPose(randChoice, HCAT_CLIMAX_FOREPLAY_FEMALE, HCAT_CLIMAX_FOREPLAY_MALE))) {
			//TODO: if active->passive swap required, do that
			return;
		}
		if (allowNormal && (chosenPassiveActive = findPose(randChoice, HCAT_CLIMAX_FRONTAL, HCAT_CLIMAX_BACK))) {
			//TODO: if active->passive swap required, do that
			return;
		}
	}
	else if (nNormalPoses > 0) {
		size_t randChoice = rand() % nNormalPoses;
		if (allowForeplay && (chosenPassiveActive = findPose(randChoice, HCAT_FOREPLAY_FEMALE, HCAT_FOREPLAY_MUTUAL))) {
			//TODO: if active->passive swap required, do that
			return;
		}
		if (allowNormal && (chosenPassiveActive = findPose(randChoice, HCAT_FRONTAL, HCAT_BACK))) {
			//TODO: if active->passive swap required, do that
			return;
		}
	}
	else {
		
	}
}

void ForceAi::PickRandomPrefPosition(ExtClass::HInfo* info, bool allowForeplay, bool allowNormal, float climaxChance) {
	//count poses
	size_t nNormalPoses = 0;
	if (allowForeplay) {
		for (int i = HCAT_FOREPLAY_FEMALE; i <= HCAT_FOREPLAY_MUTUAL; i++) {
			nNormalPoses += m_preferencePositions[i].size();
		}
	}
	if (allowNormal) {
		for (int i = HCAT_FRONTAL; i <= HCAT_BACK; i++) {
			nNormalPoses += m_preferencePositions[i].size();
		}
	}
	size_t nClimaxPoses = 0;
	if (climaxChance > 0) {
		if (allowForeplay) {
			for (int i = HCAT_CLIMAX_FOREPLAY_FEMALE; i <= HCAT_CLIMAX_FOREPLAY_MALE; i++) {
				nClimaxPoses += m_preferencePositions[i].size();
			}
		}
		if (allowNormal) {
			for (int i = HCAT_CLIMAX_FRONTAL; i <= HCAT_CLIMAX_BACK; i++) {
				nClimaxPoses += m_preferencePositions[i].size();
			}
		}
	}
	
	//choose pose
	float f = General::GetRandomFloat(0, 1);
	if (nClimaxPoses > 0 && f <= climaxChance) {
		//choose cliamx
		size_t randChoice = rand() % nClimaxPoses;
		if (allowForeplay) {
			for (int i = HCAT_CLIMAX_FOREPLAY_FEMALE; i <= HCAT_CLIMAX_FOREPLAY_MALE; i++) {
				size_t size = m_preferencePositions[i].size();
				if (randChoice < size) {
					auto& pos = m_preferencePositions[i][randChoice];
					info->m_hPosButtons[pos.first].m_arrButtonList[pos.second]->Press();
					return;
				}
				nClimaxPoses -= size;
			}
		}
		if (allowNormal) {
			for (int i = HCAT_CLIMAX_FRONTAL; i <= HCAT_CLIMAX_BACK; i++) {
				size_t size = m_preferencePositions[i].size();
				if (randChoice < size) {
					auto& pos = m_preferencePositions[i][randChoice];
					info->m_hPosButtons[pos.first].m_arrButtonList[pos.second]->Press();
					return;
				}
				nClimaxPoses -= m_preferencePositions[i].size();
			}
		}
	}
	else if(nNormalPoses > 0) {
		//choose normal
		size_t randChoice = rand() % nNormalPoses;
		if (allowForeplay) {
			for (int i = HCAT_FOREPLAY_FEMALE; i <= HCAT_FOREPLAY_MUTUAL; i++) {
				size_t size = m_preferencePositions[i].size();
				if (randChoice < size) {
					auto& pos = m_preferencePositions[i][randChoice];
					info->m_hPosButtons[pos.first].m_arrButtonList[pos.second]->Press();
					return;
				}
				nNormalPoses -= m_preferencePositions[i].size();
			}
		}
		if (allowNormal) {
			for (int i = HCAT_FRONTAL; i <= HCAT_BACK; i++) {
				size_t size = m_preferencePositions[i].size();
				if (randChoice < size) {
					auto& pos = m_preferencePositions[i][randChoice];
					info->m_hPosButtons[pos.first].m_arrButtonList[pos.second]->Press();
					return;
				}
				nNormalPoses -= m_preferencePositions[i].size();
			}
		}
	}
	else {
		//none were available
		PickRandomDomPosition(info, true, true, allowForeplay, allowNormal, climaxChance);
	}
	
}

/*plan:
step 1: pick some foreplay position from the more dom ones.
step 2: pick a foreplay position according to the rapists' sexual preferences.
step 3: pick some rapey position("rapey" is defined by the sex of the rapist)
step 4: pick some position according to the rapist's preference. Include cumming position in the list
step 5: pick some cumming position
repeat steps 3-5 a few times
if rapee has ahegao repeat 3-5 with all positions a few times
maybe add another cumming position and end the scene.
*/

ForceAi::State ForceAi::states[] = {
	{BEGIN,
		NULL, NULL, [](State* state, ForceAi* thisPtr, HInfo* info) -> DWORD { return WATCH_INTRO; }
	},
	//step 1: pick some foreplay position from the more dom ones.
	{ WATCH_INTRO,
		[](State* state, ForceAi* thisPtr, HInfo* info) {
			thisPtr->StartTimerRandom(0, 8, 15);
		},
		NULL,
		[](State* state, ForceAi* thisPtr, HInfo* info) -> DWORD {
			if (!thisPtr->TimerPassed(0)) return INVALID;
			return FOREPLAY;
		}
	},
	//step 2: pick a foreplay position according to the rapists' sexual preferences.
	{ FOREPLAY,
		[](State* state, ForceAi* thisPtr, HInfo* info) {
			thisPtr->PickRandomDomPosition(info, false, true, true, false, 0 );
			thisPtr->StartTimerRandom(0, 15, 20);
			if (state->m_customValue == 0) {
				info->m_speed = 1;
				thisPtr->SetSpeedChangeLinear(1, 2, 10);
				thisPtr->SetRepeatParams(0, 1.0f, 0.8f);
			}
		},
		[](State* state, ForceAi* thisPtr, HInfo* info) {
			info->m_speed = thisPtr->GetSpeed();
		},
		[](State* state, ForceAi* thisPtr, HInfo* info) -> DWORD {
			if (!thisPtr->TimerPassed(0)) return INVALID;
			if (thisPtr->WantRepeat(0)) { state->m_customValue++;  return FOREPLAY; }
			return DOMINANCE;
		}
	},
	//step 3: pick some rapey position("rapey" is defined by the sex of the rapist)
	{ DOMINANCE,
		[](State* state, ForceAi* thisPtr, HInfo* info) {
			thisPtr->PickRandomDomPosition(info, false, true, false, true, 0.1);
			thisPtr->StartTimerRandom(0, 15, 20);
			if (state->m_customValue == 0) {
				thisPtr->SetSpeedChangeFluctuate(info->m_speed, 2, 3);
				thisPtr->SetRepeatParams(0, 1.0f, 0.9f);
			}
		},
		[](State* state, ForceAi* thisPtr, HInfo* info) {
			info->m_speed = thisPtr->GetSpeed();
		},
		[](State* state, ForceAi* thisPtr, HInfo* info) -> DWORD {
			if (!thisPtr->TimerPassed(0)) return INVALID;
			if (thisPtr->WantRepeat(0)) { state->m_customValue++;  return DOMINANCE; }
			return PREFERENCES;
		}
	},
	//step 4: pick some position according to the rapist's preference. Include cumming position in the list
	{ PREFERENCES,
		[](State* state, ForceAi* thisPtr, HInfo* info) {
			thisPtr->PickRandomPrefPosition(info, true, true, 0.1f);
			thisPtr->StartTimerRandom(0, 15.0f, 20.0f);
			if (state->m_customValue == 0) {
				thisPtr->SetSpeedChangeFluctuate(info->m_speed, 2.5, 3.5);
				thisPtr->SetRepeatParams(0, 1.0f, 0.9f);
			}
		},
		[](State* state, ForceAi* thisPtr, HInfo* info) {
			info->m_speed = thisPtr->GetSpeed();
		},
		[](State* state, ForceAi* thisPtr, HInfo* info) -> DWORD {
			if (!thisPtr->TimerPassed(0)) return INVALID;
			if (thisPtr->WantRepeat(0)) { state->m_customValue++;  return PREFERENCES; }
			return CLIMAX;
		}
	},
	//step 5: pick some cumming position
	{ CLIMAX,
		[](State* state, ForceAi* thisPtr, HInfo* info) {
			thisPtr->PickRandomPrefPosition(info, true, true, 1);
			thisPtr->StartTimerRandom(0, 10.0f, 10.0f);
			info->m_speed = 4;
			if (state->m_customValue == 0) {
				thisPtr->SetRepeatParams(0, 1.0f, 0.9f);
			}
		},
		NULL,
		[](State* state, ForceAi* thisPtr, HInfo* info) -> DWORD {
			if (!thisPtr->TimerPassed(0)) return INVALID;
			if (thisPtr->WantRepeat(0)) { state->m_customValue++;  return PREFERENCES; }
			return END;
		}
	}
};

void ForceAi::Initialize(ExtClass::HInfo* info) {
	m_aiState = 0;
	for (int i = 0; i < INVALID; i++) {
		states[i].m_customValue = 0;
	}
	for (int i = 0; i < 9; i++) {
		m_dominantPositionsActive[i].clear();
		m_dominantPositionsPassive[i].clear();
		m_preferencePositions[i].clear();
	}

	m_forcerGender = info->m_activeParticipant->m_charPtr->m_charData->m_gender;
	m_forceeGender = info->m_passiveParticipant->m_charPtr->m_charData->m_gender;
	m_isYuri = m_forceeGender == m_forcerGender;
	for (int i = 0; i < 9; i++) {
		HPosButtonList* list = &(info->m_hPosButtons[i]);
		HGUIButton** arrIt = list->m_arrButtonList;
		for (int j = 0; arrIt + j < list->m_pLastButton; j++) {
			HGUIButton* btn = arrIt[j];
			HPosData* data = info->GetHPosData(info->GetHPosition(i, j));
			if (btn->m_posTop == 0 && btn->m_posLeft == 0) continue;
			if (data->m_yuriAllowance == GENDERALLOW_HETERO_ONLY && m_isYuri) continue; //not allowed
			if (data->m_yuriAllowance == GENDERALLOW_HOMO_ONLY && !m_isYuri) continue;
			//check if it fullfills preferences
			if (data->m_preferenceFlags & info->m_activeParticipant->m_charPtr->m_charData->GetPreferenceFlags()) {
				m_preferencePositions[i].emplace_back(i, j);
			}
			//check if its dominant
			const wchar_t* name = &(data->m_fileName[0]);
			for (const auto& it : m_activeDomNames) {
				if (it == name) {
					m_dominantPositionsActive[i].emplace_back(i, j);
				}
			}
			for (const auto& it : m_passiveDomNames) {
				if (it == name) {
					m_dominantPositionsPassive[i].emplace_back(i, j);
				}
			}

		}
	}


}

void ForceAi::Tick(ExtClass::HInfo* info) {
	DisableAllButtons(info);
	if (states[m_aiState].m_tickFunc != NULL) states[m_aiState].m_tickFunc(&states[m_aiState], this, info);
	DWORD nextState = states[m_aiState].m_endFunc(&states[m_aiState], this, info);
	if (nextState == END) {
		info->m_exitButton->Press();
	}
	else if (nextState != INVALID) {
		m_aiState = nextState;
		if (states[m_aiState].m_initFunc != NULL) states[m_aiState].m_initFunc(&states[m_aiState], this, info);
	}

}