#include "PoseFile.h"

#include <fstream>

PoseFile::PoseFile() : pose(0), frame(0) {}

PoseFile::PoseFile(std::wstring path) {
	pose = 0;
	frame = 0;

	std::ifstream in(path);

	if (!in.good()) return;

	in >> pose;
	in >> frame;

	while(in.good()) {
		FrameMod mod;
		in >> mod.frameName;
		in >> mod.modKind;
		in >> mod.value;
		mods.push_back(mod);
	}
}

void PoseFile::DumpToFile(std::wstring path) {
	std::ofstream out(path);

	if (!out.good()) return;
	out << pose << " " << frame << "\r\n";
	for(FrameMod& mod : mods) {
		out << mod.frameName << " " << mod.modKind << " " << mod.value << "\r\n";
	}
}