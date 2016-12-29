#include "PoseMods.h"

#include <fstream>
#include <algorithm>

#include "Logger.h"

PoseMods::PoseMods(std::wstring path) {
	std::ifstream in(path);
	int line = 0;
	while(in.good()) {
		line++;
		std::string line, frameName, frameDesc;
		std::getline(in,line);

		auto it = line.cbegin();
		while (it != line.cend() && isspace(*it)) it++;
		if (it == line.cend()) continue;
		if (*it == ';') continue;

		auto begin = it;
		while (it != line.cend() && !isspace(*it)) it++;
		if (it == line.cend()) continue;

		frameName = std::string(begin, it);

		while (it != line.cend() && isspace(*it)) it++;

		frameDesc = (it != line.cend()) ? std::string(it, line.cend()) : frameName;

		auto tuple = std::make_tuple(frameName,frameDesc);
		m_data.push_back(std::move(tuple));
	}
}


PoseMods::~PoseMods() {
}
