#include "PoseMods.h"

#include <fstream>
#include <algorithm>

#include "Logger.h"

PoseMods::PoseMods(std::wstring path) {
	std::ifstream in(path);
	int line = 0;
	while(in.good()) {
		line++;
		std::string line;
		std::getline(in,line);
		const char* it = line.c_str();

		std::string frame;
		std::string mod;
		std::string name;

		const char* start;

		while (*it && isspace(*it)) it++;
		if (!*it) continue;
		if (*it == ';') continue; //line is commented out

		//frame name
		start = it;
		while (*it && !isspace(*it)) frame.push_back(*it++);
		frame = std::string(start, it);

		while (*it && isspace(*it)) it++;
		if (!*it) continue;

		//mod
		start = it;
		while (*it && !isspace(*it)) mod.push_back(*it++);
		mod = std::string(start,it);

		while (*it && isspace(*it)) it++;
		if (*it) {
			//name (rest of the line)
			start = it;
			name = std::string(start, line.c_str() + line.size());
		}

		

		int imod = 0;
		if (mod == "YAW" || mod == "X") imod = 0;
		else if (mod == "PITCH" || mod == "Y") imod = 1;
		else if (mod == "ROLL" || mod == "Z") imod = 2;
		else {
			LOGPRIO(Logger::Priority::WARN) << "PoseMod file in line " << line << ": unknown mod " << mod << ". The line was ignored.\r\n";
		}

		auto triple = std::make_tuple(frame,imod,name);
		m_data.push_back(std::move(triple));

	}
}


PoseMods::~PoseMods() {
}
