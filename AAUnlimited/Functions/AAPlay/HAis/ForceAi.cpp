#include "ForceAi.h"

#include "External\ExternalVariables\AAPlay\GameGlobals.h"
#include "Files\Logger.h"

using namespace ExtClass;

ForceAi::ForceAi() {
}


ForceAi::~ForceAi() {
}

//note that certain positions may swap passive and active automatically depending on the priority member
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
		for (DWORD i = start; i <= end; i++) {
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
		}
		return 0;
	};
	
	if (allowForeplay) nNormalPoses += countPoses(HCAT_FOREPLAY_FEMALE, HCAT_FOREPLAY_MUTUAL);
	if (allowNormal) nNormalPoses += countPoses(HCAT_FRONTAL, HCAT_BACK);
	if (climaxChance > 0) {
		if (allowForeplay) nClimaxPoses += countPoses(HCAT_CLIMAX_FOREPLAY_FEMALE, HCAT_CLIMAX_FOREPLAY_MALE);
		if (allowNormal ) nClimaxPoses += countPoses(HCAT_CLIMAX_FRONTAL, HCAT_CLIMAX_BACK);
	}

	LOGPRIO(Logger::Priority::SPAM) << "Picking dom position from " << nNormalPoses << " normal and " << nClimaxPoses <<
		" climax poses, climax chance " << climaxChance << "\r\n";

	float f = General::GetRandomFloat(0, 1);
	if (nClimaxPoses > 0 && f <= climaxChance) {
		size_t randChoice = rand() % nClimaxPoses;
		LOGPRIO(Logger::Priority::SPAM) << "chose random climax position " << randChoice << "\r\n";
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
		LOGPRIO(Logger::Priority::SPAM) << "chose random position " << randChoice << "\r\n";
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
		LOGPRIO(Logger::Priority::WARN) << "no dominant positions for this type!\r\n";
		return;
	}
	LOGPRIO(Logger::Priority::WARN) << "did not find corresponding position. This means the algorithm is wrong. shit.\r\n";
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
	
	LOGPRIO(Logger::Priority::SPAM) << "Picking pref position from " << nNormalPoses << " normal and " << nClimaxPoses <<
		" climax poses, climax chance " << climaxChance << "\r\n";

	//choose pose
	float f = General::GetRandomFloat(0, 1);
	if (nClimaxPoses > 0 && f <= climaxChance) {
		//choose cliamx
		size_t randChoice = rand() % nClimaxPoses;
		LOGPRIO(Logger::Priority::SPAM) << "chose random climax position " << randChoice << "\r\n";
		if (allowForeplay) {
			for (int i = HCAT_CLIMAX_FOREPLAY_FEMALE; i <= HCAT_CLIMAX_FOREPLAY_MALE; i++) {
				size_t size = m_preferencePositions[i].size();
				if (randChoice < size) {
					auto& pos = m_preferencePositions[i][randChoice];
					info->m_hPosButtons[pos.first].m_arrButtonList[pos.second]->Press();
					return;
				}
				randChoice -= size;
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
				randChoice -= size;
			}
		}
	}
	else if(nNormalPoses > 0) {
		//choose normal
		size_t randChoice = rand() % nNormalPoses;
		LOGPRIO(Logger::Priority::SPAM) << "chose random position " << randChoice << "\r\n";
		if (allowForeplay) {
			for (int i = HCAT_FOREPLAY_FEMALE; i <= HCAT_FOREPLAY_MUTUAL; i++) {
				size_t size = m_preferencePositions[i].size();
				if (randChoice < size) {
					auto& pos = m_preferencePositions[i][randChoice];
					info->m_hPosButtons[pos.first].m_arrButtonList[pos.second]->Press();
					return;
				}
				randChoice -= size;
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
				randChoice -= size;
			}
		}
	}
	else {
		//none were available
		LOGPRIO(Logger::Priority::SPAM) << "no preference positions were avaiable, so a dominant is chosen instead.\r\n";
		PickRandomDomPosition(info, !this->m_isActive, this->m_isActive, allowForeplay, allowNormal, climaxChance);
		return;
	}
	LOGPRIO(Logger::Priority::WARN) << "did not find corresponding position. This means the algorithm is wrong. shit.\r\n";
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
			thisPtr->PickRandomDomPosition(info, !thisPtr->m_isActive, thisPtr->m_isActive, true, false, 0 );
			thisPtr->StartTimerRandom(0, 15, 20);
			if (state->m_customValue == 0) {
				info->m_speed = 1;
				thisPtr->SetSpeedChangeLinear(1, 2, 10);
				thisPtr->SetRepeatParams(0, 1.0f, 0.5f);
				if (thisPtr->m_forcee->m_shoesOffState == 0) {
					info->m_btnShoe->Press(); //slip of shoes. they suck.
				}
			}
			else if (state->m_customValue == 1 && thisPtr->m_forcee->m_bClothesSlipped == 0) {
				info->m_btnUnderwear->Press();
				thisPtr->m_forcee->m_bClothesSlipped = 1;
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
			thisPtr->PickRandomDomPosition(info, !thisPtr->m_isActive, thisPtr->m_isActive, false, true, 0.1f);
			if (state->m_customValue == 0) {
				thisPtr->StartTimerRandom(0,15,20);
				thisPtr->SetSpeedChangeFluctuate(info->m_speed, 2, 3);
				thisPtr->SetRepeatParams(0, 0.8f, 0.5f);
				if (thisPtr->m_forcee->m_clothesState == 0) {
					info->m_btnOutfit->Press();
					thisPtr->m_forcee->m_clothesState = 1;
				}
				if (thisPtr->m_forcee->m_bClothesSlipped == 0) {
					info->m_btnUnderwear->Press(); //slip now if not done yet
					thisPtr->m_forcee->m_bClothesSlipped = 1;
				}
			}
			else {
				thisPtr->StartTimerRandom(0,5,10);
			}
		},
		[](State* state, ForceAi* thisPtr, HInfo* info) {
			info->m_speed = thisPtr->GetSpeed();
		},
		[](State* state, ForceAi* thisPtr, HInfo* info) -> DWORD {
			if (!thisPtr->TimerPassed(0)) return INVALID;
			if (thisPtr->WantRepeat(0)) { state->m_customValue++;  return DOMINANCE; }

			//reset the repeat counter before moving on so the repeat params reset when we come back to this state
			state->m_customValue = 0; 
			return PREFERENCES;
		}
	},
	//step 4: pick some position according to the rapist's preference. Include cumming position in the list
	{ PREFERENCES,
		[](State* state, ForceAi* thisPtr, HInfo* info) {
			thisPtr->PickRandomPrefPosition(info, true, true, 0.1f);
			if (state->m_customValue == 0) {
				thisPtr->StartTimerRandom(0,15.0f,20.0f);
				thisPtr->SetSpeedChangeFluctuate(info->m_speed, 2.5, 3.5);
				thisPtr->SetRepeatParams(0, 1.0f, 0.5f);
			}
			else {
				thisPtr->StartTimerRandom(0,5.0f,10.0f);
			}
		},
		[](State* state, ForceAi* thisPtr, HInfo* info) {
			info->m_speed = thisPtr->GetSpeed();
		},
		[](State* state, ForceAi* thisPtr, HInfo* info) -> DWORD {
			if (!thisPtr->TimerPassed(0)) return INVALID;
			if (thisPtr->WantRepeat(0)) { state->m_customValue++;  return PREFERENCES; }

			//reset the repeat counter before moving on so the repeat params reset when we come back to this state
			state->m_customValue = 0;
			return CLIMAX;
		}
	},
	//step 5: pick some cumming position
	{ CLIMAX,
		[](State* state, ForceAi* thisPtr, HInfo* info) {
			thisPtr->PickRandomPrefPosition(info, true, true, 1);
			thisPtr->StartTimerRandom(0, 20.0f, 20.0f);
			if (state->m_customValue == 0) {
				thisPtr->SetRepeatParams(1, 1.0f, 0.1f);
			}
		},
		[](State* state, ForceAi* thisPtr, HInfo* info) {
			if (thisPtr->m_forcer->m_charPtr->m_charData->m_traitBools[TRAIT_VIOLENT]) {
				info->m_speed = 2;
			}
			else {
				info->m_speed = 1;
			}
		},
		[](State* state, ForceAi* thisPtr, HInfo* info) -> DWORD {
			if (!thisPtr->TimerPassed(0)) return INVALID;
			if (thisPtr->WantRepeat(1)) { 
				if (state->m_customValue == 0) {
					//if repeat the first time, unress fully
					if (thisPtr->m_forcee->m_clothesState != 2) {
						info->m_btnOutfit->Press();
						thisPtr->m_forcee->m_clothesState = 2;
					}
				}
				state->m_customValue++;  
				return DOMINANCE; 
			}

			//need to reset clothes data here or the next run may not undress correctly
			thisPtr->m_forcee->m_clothesState = 0;
			thisPtr->m_forcee->m_bClothesSlipped = 0;

			return END;
		}
	}
};

void ForceAi::Initialize(ExtClass::HInfo* info) {
	LOGPRIO(Logger::Priority::SPAM) << "initializing ForceAi\r\n";

	//seed our random number generator
	srand((unsigned int)time(NULL));

	m_aiState = 0;
	for (int i = 0; i < INVALID; i++) {
		states[i].m_customValue = 0;
	}
	for (int i = 0; i < 9; i++) {
		m_dominantPositionsActive[i].clear();
		m_dominantPositionsPassive[i].clear();
		m_preferencePositions[i].clear();
	}
	//find out who we and our forcer are
	if (info->m_activeParticipant->m_charPtr == *ExtVars::AAPlay::PlayerCharacterPtr()) {
		//we are the active, so forcer is the passive
		m_forcee = info->m_activeParticipant;
		m_forcer = info->m_passiveParticipant;
		m_isActive = false;
	}
	else {
		//we are the passive, so forcer is the active
		m_forcer = info->m_activeParticipant;
		m_forcee = info->m_passiveParticipant;
		m_isActive = true;
	}
	m_forcerGender = m_forcer->m_charPtr->m_charData->m_gender;
	m_forceeGender = m_forcee->m_charPtr->m_charData->m_gender;
	
	m_isYuri = m_forceeGender == m_forcerGender;
	if (g_Logger.FilterPriority(Logger::Priority::SPAM)) {
		LOGPRIO(Logger::Priority::SPAM) << "Force information:\r\n";
		g_Logger << "Forcer: \r\n\tPointer: " << m_forcer << "\r\n\tGender: " << m_forcerGender << "\r\n";
		g_Logger << "Forcee: \r\n\tPointer: " << m_forcee << "\r\n\tGender: " << m_forceeGender << "\r\n";
	}
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
			if (data->m_preferenceFlags & m_forcer->m_charPtr->m_charData->GetPreferenceFlags()) {
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
	if (g_Logger.FilterPriority(Logger::Priority::WARN)) {
		if (m_dominantPositionsActive.size() == 0) {
			LOGPRIO(Logger::Priority::WARN) << "no dominant active positions found\r\n";
		}
		if (m_dominantPositionsPassive.size() == 0) {
			LOGPRIO(Logger::Priority::WARN) << "no dominant passive positions found\r\n";
		}
		size_t size = 0;
		for (int i = HCAT_FOREPLAY_FEMALE; i <= HCAT_BACK; i++) size += m_preferencePositions[i].size();
		if (size == 0) {
			LOGPRIO(Logger::Priority::WARN) << "no preference positions found\r\n";
		}
		size = 0;
		for (int i = HCAT_CLIMAX_FOREPLAY_FEMALE; i <= HCAT_CLIMAX_BACK; i++) size += m_preferencePositions[i].size();
		if (size == 0) {
			LOGPRIO(Logger::Priority::WARN) << "no climax preference positions found\r\n";
		}
	}
	LOGPRIO(Logger::Priority::SPAM) << "initialized ForceAi\r\n";
}

void ForceAi::Tick(ExtClass::HInfo* info) {
	DisableAllButtons(info);
	if (states[m_aiState].m_tickFunc != NULL) states[m_aiState].m_tickFunc(&states[m_aiState], this, info);
	DWORD nextState = states[m_aiState].m_endFunc(&states[m_aiState], this, info);
	if (nextState == END) {
		info->m_btnExit->Press();
	}
	else if (nextState != INVALID) {
		LOGPRIO(Logger::Priority::SPAM) << "Switching from state " << m_aiState << " to " << nextState << "\r\n";
		m_aiState = nextState;
		if (states[m_aiState].m_initFunc != NULL) states[m_aiState].m_initFunc(&states[m_aiState], this, info);
	}

}