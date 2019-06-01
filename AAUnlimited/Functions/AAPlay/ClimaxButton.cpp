#include "StdAfx.h"
#include <random>

namespace ClimaxButton {

	bool initialized[2] = { false, false }; // 0 - hetero | 1 - homo
	bool initializedCfg = false;
	bool debugMode = false;
	bool verifyRegister = false;
	ExtClass::HInfo* hInfo;
	std::default_random_engine rand_gen;

	const UINT maxPoses = 2000;
	const UINT maxPosesInCat = 200;
	CLIMAX_BUTTON hButtonToClimax[2][maxPoses];
		// hButtonToClimax[0 - hetero|1 - homo][hInfo->m_currPosition] == {.categoryNode, .buttonNode}
		// .categoryNode in hInfo->m_hPosButtons[<0..8>]
		// .buttonNode in hInfo->m_hPosButtons[0..8].m_arrButtonList[<0..n>]
		// .anywayAdd == true, if need to add anyway for current gender
	CLIMAX_NORMAL_BUTTON hButtonToNormal[2][maxPoses];
	int hBtnPosToIndex[2][9][maxPosesInCat]; // Can get pose id
		// hBtnPosToIndex[0 - hetero|1 - homo][category_id][0..n button_pos_in_category] == pose_id;

	// Scenario for fix buttons attachment between normal and climax version/versions
	std::list< CLIMAX_CAT_SCENARIO > hCategoriesScenario[2]; 

	void InitGenderPoses(int gender_id) {
		int pose_id = -1;
		int button_pos_in_cat = 0;

		for (int cat_i = 0; cat_i < 9; cat_i++) {
			button_pos_in_cat = 0;
			ExtClass::HPosButtonList* list = &(hInfo->m_hPosButtons[cat_i]);
			ExtClass::HGUIButton** arrIt = list->m_arrButtonList;
			for (int button_i = 0; arrIt + button_i < list->m_pLastButton; button_i++) {
				pose_id++;

				ExtClass::HGUIButton* btn = arrIt[button_i];
				HPosData* data = hInfo->GetHPosData(hInfo->GetHPosition(cat_i, button_i));

				if (!hButtonToClimax[gender_id][pose_id].anywayAdd) {
					//if (btn->m_posTop == 0 && btn->m_posLeft == 0) continue;//////////////
					if (gender_id == 0 && btn->m_posTop == 0 && btn->m_posLeft == 0) continue;////////////// for HETERO great all
					if (data->m_yuriAllowance == GENDERALLOW_HETERO_ONLY && gender_id == 1) continue; //not allowed
					if (data->m_yuriAllowance == GENDERALLOW_HOMO_ONLY && gender_id == 0) continue;
					//check if it fullfills preferences
					//if (
						//!data->m_preferenceFlags 
						//|| 
						//!hInfo->m_activeParticipant->m_charPtr->m_charData->GetPreferenceFlags()////////////////
						//)
						//continue;
				}
				// Register button
				hButtonToClimax[gender_id][pose_id].categoryNode = cat_i;
				hButtonToClimax[gender_id][pose_id].buttonNode = button_i;
				hButtonToNormal[gender_id][pose_id].categoryNode = cat_i;
				hButtonToNormal[gender_id][pose_id].buttonNode = button_i;

				hBtnPosToIndex[gender_id][cat_i][button_pos_in_cat] = pose_id;

				// Normal pose version for normal pose will be same
				if (cat_i < 5) {
					hButtonToNormal[gender_id][pose_id].normalPoses[0] = pose_id;
					hButtonToNormal[gender_id][pose_id].normalCount = 1;
				}
				// For Climax categories Climax pose will be same
				else {
					hButtonToClimax[gender_id][pose_id].climaxPoses[0] = pose_id;
					hButtonToClimax[gender_id][pose_id].climaxCount = 1;
				}

				button_pos_in_cat++;
			}
			if (debugMode) {
				const char * gender_name = "Hetero";
				if (gender_id == 1)
					gender_name = "Homo";
				LOGPRIO(Logger::Priority::INFO) << "[ClimaxBtn] Registered " << button_pos_in_cat << " poses in " 
					<< gender_name << " Category id: " << cat_i << "\r\n";
			}
		}
	}

	// Attaching normal poses to their climax version (with cfg fixes)
	void AttachGenderPoses(int gender_id) {

		int btnClimaxVerCategory = 5;// 0 & 2 => 5, 1 => 6, 3 => 7, 4 => 8
		int btnClimaxVerButton = 0;
		int btnClimaxTouchesSavedPos = 0; // Saving Last button pos of 'Climax touches' for category 2 (masturbate)

		for (int cat_i = 0; cat_i < 5; cat_i++) // For each Category of normal poses
		{
			for (int button_i = 0; button_i < maxPosesInCat; button_i++)
			{
				int pose_id = hBtnPosToIndex[gender_id][cat_i][button_i];
				if (pose_id == -1) 
					break;
				// For each Button of normal poses

				// If need apply fix from config
				if (!hCategoriesScenario[gender_id].empty() 
					&& !hCategoriesScenario[gender_id].front().isShift
					&& hCategoriesScenario[gender_id].front().catId == cat_i
					&& hCategoriesScenario[gender_id].front().buttonPos == button_i) {

					hButtonToClimax[gender_id][pose_id].climaxCount = hCategoriesScenario[gender_id].front().climaxCount;
					
					for (int climax_i = 0; climax_i < 3; climax_i++) {
						int climax_pose_cat = hCategoriesScenario[gender_id].front().climaxPoses[climax_i][0];
						int climax_pose_btn_pos = hCategoriesScenario[gender_id].front().climaxPoses[climax_i][1];
						if (climax_pose_btn_pos != -1) {
							int climax_pos_id = hBtnPosToIndex[gender_id][climax_pose_cat][climax_pose_btn_pos];
							hButtonToClimax[gender_id][pose_id].climaxPoses[climax_i] = climax_pos_id;
							// Normal version for this climax pose
							if (hButtonToNormal[gender_id][climax_pos_id].normalCount < 3) {
								hButtonToNormal[gender_id][climax_pos_id].normalPoses[hButtonToNormal[gender_id][climax_pos_id].normalCount] = pose_id;
								hButtonToNormal[gender_id][climax_pos_id].normalCount++;
							}
						}
					}

					hCategoriesScenario[gender_id].pop_front(); // remove this scenario command
					
					// If need shift `climax button_i` also
					if (!hCategoriesScenario[gender_id].empty() 
						&& hCategoriesScenario[gender_id].front().isShift)
					{
						btnClimaxVerButton += hCategoriesScenario[gender_id].front().shiftVal;
						if (btnClimaxVerButton < -1)
							btnClimaxVerButton = -1;
						hCategoriesScenario[gender_id].pop_front(); // remove this scenario command
					}
				}
				else {
					// Climax 1st version for normal pose (by default)
					int climax_pos_id = hBtnPosToIndex[gender_id][btnClimaxVerCategory][btnClimaxVerButton];
					hButtonToClimax[gender_id][pose_id].climaxPoses[0] = climax_pos_id;
					hButtonToClimax[gender_id][pose_id].climaxCount = 1;
					// Normal version for this climax pose
					if (hButtonToNormal[gender_id][climax_pos_id].normalCount < 3) {
						hButtonToNormal[gender_id][climax_pos_id].normalPoses[hButtonToNormal[gender_id][climax_pos_id].normalCount] = pose_id;
						hButtonToNormal[gender_id][climax_pos_id].normalCount++;
					}
				}

				btnClimaxVerButton++;
			}
			// If next Category need for Climax Version of normal button
			if (cat_i == 0)
				btnClimaxTouchesSavedPos = btnClimaxVerButton; // Save for future category 2
			
			if (cat_i == 1) {									// When need restore button pos for category 2
				btnClimaxVerButton = btnClimaxTouchesSavedPos;
				btnClimaxVerCategory = 5;
			}
			else {
				if (cat_i == 2)
					btnClimaxVerCategory = 7;
				else 
					btnClimaxVerCategory++;
				btnClimaxVerButton = 0;
			}
		}
	}

	void StartClimaxPose() { // Starting Climax version of current pose
		hInfo = Shared::GameState::getHInfo(); // Not H scene
		if (!hInfo)
			return;

		int gender_i = 0; // hetero
		if (hInfo->m_activeParticipant->m_charPtr->m_charData->m_gender
			== hInfo->m_passiveParticipant->m_charPtr->m_charData->m_gender)
			gender_i = 1; // homo

		if (!initialized[gender_i])
			Init();
		if (!initialized[gender_i]) { // If Init not success
			LOGPRIO(Logger::Priority::WARN) << "[ClimaxBtn] Can't initialize ClimaxButton params\r\n";
			return;
		}
		
		if (debugMode)
			LOGPRIO(Logger::Priority::WARN) << "[ClimaxBtn] Current pose id: " << hInfo->m_currPosition << "\r\n";

		if (hInfo->m_currPosition < 0 || hInfo->m_currPosition >= maxPoses)
			return;

		if (hButtonToClimax[gender_i][hInfo->m_currPosition].buttonNode == -1) {
			LOGPRIO(Logger::Priority::WARN) << "[ClimaxBtn] Current pose is unregistered\r\n";
			return;
		}

		// Get Climax version
		int climax_pose_node = 0;
		if (hButtonToClimax[gender_i][hInfo->m_currPosition].climaxCount > 1) { // If need random pick climax pose
			std::uniform_int_distribution<int> distribution(0, (hButtonToClimax[gender_i][hInfo->m_currPosition].climaxCount - 1));
			climax_pose_node = distribution(rand_gen);
		}

		int climax_id = hButtonToClimax[gender_i][hInfo->m_currPosition].climaxPoses[climax_pose_node];
		if (verifyRegister) // If need verify only registered status of current pose
			climax_id = hInfo->m_currPosition;
		if (climax_id == -1) {
			LOGPRIO(Logger::Priority::WARN) << "[ClimaxBtn] Current pose haven't climax version\r\n"; 
			return;
		}

		int climaxBtnCategory = hButtonToClimax[gender_i][climax_id].categoryNode;
		int climaxBtnNode = hButtonToClimax[gender_i][climax_id].buttonNode;

		if (hInfo->m_hPosButtons[climaxBtnCategory].m_arrButtonList[climaxBtnNode] != NULL) {
			// If button Showed for current H scene
			if (!hInfo->m_hPosButtons[climaxBtnCategory].m_arrButtonList[climaxBtnNode]->m_bInvalid || debugMode)
				hInfo->m_hPosButtons[climaxBtnCategory].m_arrButtonList[climaxBtnNode]->Press();
		}
	}

	void StartNormalPose() { // Starting Normal version of current pose
		hInfo = Shared::GameState::getHInfo(); // Not H scene
		if (!hInfo)
			return;

		int gender_i = 0; // hetero
		if (hInfo->m_activeParticipant->m_charPtr->m_charData->m_gender
			== hInfo->m_passiveParticipant->m_charPtr->m_charData->m_gender)
			gender_i = 1; // homo

		if (!initialized[gender_i])
			Init();
		if (!initialized[gender_i]) { // If Init not success
			LOGPRIO(Logger::Priority::WARN) << "[ClimaxBtn] Can't initialize ClimaxButton params\r\n";
			return;
		}

		if (debugMode)
			LOGPRIO(Logger::Priority::WARN) << "[ClimaxBtn] Current pose id: " << hInfo->m_currPosition << "\r\n";

		if (hInfo->m_currPosition < 0 || hInfo->m_currPosition >= maxPoses)
			return;

		if (hButtonToNormal[gender_i][hInfo->m_currPosition].buttonNode == -1) {
			LOGPRIO(Logger::Priority::WARN) << "[ClimaxBtn] Current pose is unregistered\r\n";
			return;
		}

		// Get Normal version
		int normal_pose_node = 0;
		if (hButtonToNormal[gender_i][hInfo->m_currPosition].normalCount > 1) { // If need random pick normal pose
			std::uniform_int_distribution<int> distribution(0, (hButtonToNormal[gender_i][hInfo->m_currPosition].normalCount - 1));
			normal_pose_node = distribution(rand_gen);
		}

		int normal_id = hButtonToNormal[gender_i][hInfo->m_currPosition].normalPoses[normal_pose_node];
		if (verifyRegister) // If need verify only registered status of current pose
			normal_id = hInfo->m_currPosition;
		if (normal_id == -1) {
			LOGPRIO(Logger::Priority::WARN) << "[ClimaxBtn] Current pose haven't Normal version\r\n";
			return;
		}

		int normalBtnCategory = hButtonToNormal[gender_i][normal_id].categoryNode;
		int normalBtnNode = hButtonToNormal[gender_i][normal_id].buttonNode;

		if (hInfo->m_hPosButtons[normalBtnCategory].m_arrButtonList[normalBtnNode] != NULL) {
			// If button Showed for current H scene
			if (!hInfo->m_hPosButtons[normalBtnCategory].m_arrButtonList[normalBtnNode]->m_bInvalid || debugMode)
				hInfo->m_hPosButtons[normalBtnCategory].m_arrButtonList[normalBtnNode]->Press();
		}
	}

	void InitCfg() {
		if (initializedCfg)
			return;

		// Starting data for buttons
		for (int gender_i = 0; gender_i < 2; gender_i++)
			for (int categ_i = 0; categ_i < 9; categ_i++)
				for (int pose_id = 0; pose_id < maxPosesInCat; pose_id++)
					hBtnPosToIndex[gender_i][categ_i][pose_id] = -1;


		// Get the config file for current poses pack
		std::ifstream infile(General::AAUPath + L"configs\\climax_button_poses.txt");
		std::string line;
		while (std::getline(infile, line))
		{
			if (std::regex_match(line, std::regex("\\s*\\/\\/.*"))) // if comment
				continue;

			std::smatch matches;
			std::smatch matchesIn;
			// Debug param
			if (std::regex_search(line, matches, std::regex("Debug\s*\=\s*([a-zA-Z]+)")))
			{
				if (matches[1].str() == "true") {
					debugMode = true;
					LOGPRIO(Logger::Priority::INFO) << "[ClimaxBtn] Debug Activated\r\n";
				}
			}
			else if (std::regex_search(line, matches, std::regex("VerifyRegister\s*\=\s*([a-zA-Z]+)")))
			{
				if (matches[1].str() == "true") {
					verifyRegister = true;
					if (debugMode)
						LOGPRIO(Logger::Priority::INFO) << "[ClimaxBtn] VerifyRegister Activated\r\n";
				}
			}
			// Fix register poses id
			else if (std::regex_search(line, matches,
				std::regex("\\[(FIX_HETERO|FIX_HOMO)\\][^0-9]*(\\d+)")))
			{
				int gender_id = matches[1].str() == "FIX_HETERO" ? 0 : 1;
				int pose_id = std::stoi(matches[2].str());
				if (pose_id > -1 && pose_id < maxPoses) {
					hButtonToClimax[gender_id][pose_id].anywayAdd = true;
					if (debugMode)
						LOGPRIO(Logger::Priority::INFO) << "[ClimaxBtn] Detected fix register for " << matches[1].str()
						<< " (id: " << gender_id << ") the pose id: " << pose_id << "\r\n";
				}
			}
			// hetero and homo poses attachment fixes
			else if (std::regex_search(line, matches,
				std::regex("\\[(HETERO_LINK|HOMO_LINK)\\][^{]*\\{([^{]*)\\}[^{]*\\{([^{]*)\\}[^{]*\\{([^{]*)\\}[^{]*\\{([^{]*)\\}")))
			{
				int gender_id = matches[1].str() == "HETERO_LINK" ? 0 : 1;
				// Fill the climax scenario command for current gender
				CLIMAX_CAT_SCENARIO climaxScenario;

				std::string match_str = matches[2].str();
				if (std::regex_search(match_str, matchesIn, std::regex("(\\d+)[^0-9]+(\\d+)")))
				{
					climaxScenario.catId = std::stoi(matchesIn[1].str());
					climaxScenario.buttonPos = std::stoi(matchesIn[2].str());
				}
				int climaxCount = 0;
				match_str = matches[3].str();	// 1st climax ver.
				if (std::regex_search(match_str, matchesIn, std::regex("(\\d+)[^0-9]+(\\d+)")))
				{
					climaxScenario.climaxPoses[0][0] = std::stoi(matchesIn[1].str());
					climaxScenario.climaxPoses[0][1] = std::stoi(matchesIn[2].str());
					climaxCount++;
				}
				match_str = matches[4].str();	// 2nd climax ver.
				if (std::regex_search(match_str, matchesIn, std::regex("(\\d+)[^0-9]+(\\d+)")))
				{
					int pos_i = 0;
					if (climaxScenario.climaxPoses[0][0] != -1) { pos_i++; }
					climaxScenario.climaxPoses[pos_i][0] = std::stoi(matchesIn[1].str());
					climaxScenario.climaxPoses[pos_i][1] = std::stoi(matchesIn[2].str());
					climaxCount++;
				}
				match_str = matches[5].str();	// 3rd climax ver.
				if (std::regex_search(match_str, matchesIn, std::regex("(\\d+)[^0-9]+(\\d+)")))
				{
					int pos_i = 0;
					if (climaxScenario.climaxPoses[0][0] != -1) { pos_i++; }
					if (climaxScenario.climaxPoses[1][0] != -1) { pos_i++; }
					climaxScenario.climaxPoses[pos_i][0] = std::stoi(matchesIn[1].str());
					climaxScenario.climaxPoses[pos_i][1] = std::stoi(matchesIn[2].str());
					climaxCount++;
				}
				climaxScenario.climaxCount = climaxCount;
				// Add Scenario command to result scenario
				hCategoriesScenario[gender_id].push_back(climaxScenario);
				if (debugMode)
					LOGPRIO(Logger::Priority::INFO) << "[ClimaxBtn] Detected Link " << matches[0].str() << "\r\n";
			}
			// hetero and homo poses attachment fixes (Shifts)
			else if (std::regex_search(line, matches,
				std::regex("\\[(HETERO_SHIFT|HOMO_SHIFT)\\][^\\+\\-]([\\+\\-])(\\d+)"))) 
			{
				int gender_id = matches[1].str() == "HETERO_SHIFT" ? 0 : 1;
				int singPlusMinus = matches[2].str() == "+" ? 1 : -1;
				int shiftVal = std::stoi(matches[3].str());
				// Fill the climax scenario command for current gender
				CLIMAX_CAT_SCENARIO climaxScenario;
				climaxScenario.isShift = true;
				climaxScenario.shiftVal = shiftVal * singPlusMinus;
				// Add Scenario command to result scenario
				hCategoriesScenario[gender_id].push_back(climaxScenario);
				if (debugMode)
					LOGPRIO(Logger::Priority::INFO) << "[ClimaxBtn] Detected Shift " << matches[0].str() << "\r\n";
			}
		}

		if (debugMode)
			LOGPRIO(Logger::Priority::INFO) << "[ClimaxBtn] Climax InitCfg() OK\r\n";

		initializedCfg = true;
	}

	void Init() {
		if (!initializedCfg)
			InitCfg();
		if (!initializedCfg) { // If Init of config file not success
			LOGPRIO(Logger::Priority::WARN) << "[ClimaxBtn] Can't initialize CFG txt file for ClimaxButton\r\n";
			return;
		}

		hInfo = Shared::GameState::getHInfo(); // Not H scene yet
		if (!hInfo)
			return;

		int gender_i = 0; // hetero
		if (hInfo->m_activeParticipant->m_charPtr->m_charData->m_gender
			== hInfo->m_passiveParticipant->m_charPtr->m_charData->m_gender)
			gender_i = 1; // homo

		if (initialized[gender_i]) 
			return;

		if (debugMode)
			LOGPRIO(Logger::Priority::INFO) << "[ClimaxBtn] Climax Init() START\r\n";

		InitGenderPoses(gender_i);
		// Gotten hButtonToClimax (not all params) and  hBtnPosToIndex
		// Now we need to attach normal poses to their climax version (using also fixes from hCategoriesScenario)
		AttachGenderPoses(gender_i);

		if (debugMode)
			LOGPRIO(Logger::Priority::INFO) << "[ClimaxBtn] Climax Init() END (success)\r\n";

		initialized[gender_i] = true;
	}
}
