#include "PoseMods.h"

#include <map>
#include <fstream>
#include <algorithm>

#include "Logger.h"

PoseMods::PoseMods(std::wstring path) {
	std::ifstream in(path);
	std::map<std::string, FrameCategory> categoryMap;
	categoryMap["BODY"]  = Torso;
	categoryMap["LARM"] = LeftArm;
	categoryMap["RARM"] = RightArm;
	categoryMap["LHAND"] = LeftHand;
	categoryMap["RHAND"] = RightHand;
	categoryMap["LLEG"] = LeftLeg;
	categoryMap["RLEG"] = RightLeg;
	categoryMap["BREAST"] = Breasts;
	categoryMap["FACE"] = Face;
	categoryMap["SKIRT"] = Skirt;
	//categoryMap["ROOM"]  = Room;
	categoryMap["OTHER"] = Other;
	categoryMap["PROP"] = Prop;
	while(in.good()) {
		std::string line, categoryName, frameName, frameDesc, operations, axes;
		FrameCategory category = Other;
		std::getline(in,line);

		auto it = line.cbegin();
		while (it != line.cend() && isspace(*it)) it++;
		if (it == line.cend()) continue;
		if (*it == ';') continue;

		// Get category
		auto begin = it;
		while (it != line.cend() && !isspace(*it)) it++;
		if (it == line.cend()) continue;
		frameName = std::string(begin, it);

		while (it != line.cend() && isspace(*it)) it++;

#define GETWORD(iter) if (it == line.cend()) continue; begin = iter; while (iter != line.cend() && !isspace(*iter)) iter++;
#define SKIPSPACE(iter) while (iter != line.cend() && isspace(*iter)) iter++;
		GETWORD(it)
		categoryName = std::string(begin, it);
		SKIPSPACE(it)
		GETWORD(it)
		operations = std::string(begin, it);
		SKIPSPACE(it)
		GETWORD(it)
		axes = std::string(begin, it);
		SKIPSPACE(it)
		frameDesc = (it != line.cend()) ? std::string(it, line.cend()) : frameName;
#undef GETWORD
#undef SKIPSPACE
		try {
			category = categoryMap[categoryName];
		}
		catch (std::out_of_range&) {
			category = Other;
		}
		auto tuple = std::make_tuple(category,frameName,frameDesc);
		m_data.push_back(std::move(tuple));
	}
}


PoseMods::~PoseMods() {
}
