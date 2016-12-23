#include <fstream>
#include <iostream>

#include "Object.h"

int main(int argc, char* argv[]) {
	std::string file("test.xx");
	std::string toExtract("A00_O_kao");
	std::string extractTarget("extractedObject.xxo");
	if(argc >= 2) {
		file = argv[1];
	}
	if (argc >= 3) {
		toExtract = argv[2];
	}
	if (argc >= 4) {
		extractTarget = argv[3];
	}
	
	std::ifstream in(file,std::ios::binary);
	if (!in.good()) return 0;
	
	//skip header
	uint32_t version;
	in.read((char*)&version,4);
	in.seekg(1 + 4 + 17,std::ios::cur);
	auto test = in.tellg();

	Object root(in, version);
	
	std::cout << (std::string)root;

	std::cout << "\r\n\r\n";
	
	//search toExtract in the object tree
	Object* match = root.Find(toExtract);
	if(match == nullptr) {
		std::cout << "object " << toExtract << " was not found.\r\n";
		return 0;
	}

	std::ofstream out(extractTarget, std::ios::binary);

	in.seekg(match->startOffset,std::ios::beg);
	uint8_t* buffer = new uint8_t[match->size];
	in.read((char*)buffer,match->size);
	out.write((char*)buffer,match->size);

	return 0;
}