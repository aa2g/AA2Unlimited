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

	FrameMod mod;
	while(in.good()) {
		in >> mod.frameName;
		for (int i = 0; i < 9 && in.good(); i++) {
			in >> mod.matrix[i];
		}
		mods.push_back(mod);
	}
}

void PoseFile::DumpToFile(std::wstring path) {
	std::ofstream out(path);

	if (!out.good()) return;
	out << pose << " " << frame << std::endl;
	for(FrameMod& mod : mods) {
		out << mod.frameName;
		for (float m : mod.matrix) {
			out << " " << m;
		}
		out << std::endl;;
	}
}
