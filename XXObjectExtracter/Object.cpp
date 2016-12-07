#include "Object.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

Object::Object() : children(NULL) {

}

#define INREAD(x) in.read((char*)(&x), sizeof(x))

Object::Object(std::ifstream & in, uint32_t xxversion) : Object() {
	in.exceptions(std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit);
	
	try {
		this->startOffset = in.tellg();

		//name
		size_t nameLength;
		INREAD(nameLength);
		if(nameLength > 0) {
			char name[256];
			in.read(name,nameLength);
			for (int i = 0; i < nameLength; i++) name[i] = ~name[i];
			this->name = name;
		}
		
		//n children
		INREAD(this->nChildren);
		//trans matrix
		for(int i = 0; i < 16; i++) INREAD(matrix[i]);

		//skip versiondependent bytes
		in.seekg(xxversion < 8 ? 0x10 : 0x20,std::ios_base::cur);
		INREAD(meshCount);
		//skip rest
		in.seekg(3*4+3*4,std::ios_base::cur);
		in.seekg(xxversion < 8 ? 0x10 : 0x40,std::ios_base::cur);
		if(xxversion >= 6) {
			uint32_t nameLength;
			INREAD(nameLength);
			if(nameLength > 0) {
				char name[256];
				in.read(name,nameLength);
				for (int i = 0; i < nameLength; i++) name[i] = ~name[i];
				this->someName = name;
			}
		}

		if(meshCount > 0) {
			//mesh info
			isMesh = true;
			uint8_t meshType;
			INREAD(meshType);
			for(uint32_t i = 0; i < meshCount; i++) {
				in.seekg(xxversion < 8 ? 0x10 : 0x40,std::ios_base::cur);
				in.seekg(4,std::ios_base::cur);


				uint32_t faceCount;
				INREAD(faceCount);

				in.seekg(faceCount*2,std::ios_base::cur);


				uint32_t vertexCount;
				INREAD(vertexCount);

				in.seekg(vertexCount*70,std::ios_base::cur);


				if(xxversion >= 5) {
					in.seekg(4*5,std::ios_base::cur);
				}
				if (xxversion >= 6) {
					in.seekg(0x60 + 4,std::ios_base::cur);
				}
				if(xxversion < 6) {
					in.seekg(0x40,std::ios_base::cur);
				}
				else {
					in.seekg(0x100,std::ios_base::cur);
				}
				if (xxversion >= 6) {
					in.seekg(0x1C,std::ios_base::cur);
				}
				if (xxversion >= 8) {
					in.seekg(1,std::ios_base::cur);
					
					uint32_t nameLength;
					INREAD(nameLength);
					if (nameLength > 0) {
						in.seekg(nameLength,std::ios_base::cur);
					}
					in.seekg(4 + 3*4,std::ios_base::cur);
				}
			}

			uint16_t mysteryCount;
			INREAD(mysteryCount);
			in.seekg(8,std::ios_base::cur);
			if(xxversion >= 6) {
				in.seekg(70*mysteryCount,std::ios_base::cur);
			}
			else {
				std::cout << "dont know what to do; throw exception\r\n";
			}

			//bone info
			uint32_t boneCount;
			INREAD(boneCount);

			for (uint32_t i = 0; i < boneCount; i++) {
				uint32_t nameLength;
				INREAD(nameLength);
				in.seekg(nameLength,std::ios_base::cur);
				in.seekg(4 + 64,std::ios_base::cur);
			}
		}

		if(nChildren != 0) {
			children.resize(nChildren);
			for(uint32_t i = 0; i < nChildren; i++) {
				children[i] = Object(in, xxversion);
			}
		}

		this->size = in.tellg(); 
		this->size -= this->startOffset;
	}
	catch(std::ifstream::failure e) {
		std::cout << "Error while reading object: " << e.code() << " - " << e.what() << std::endl;
	}
}

void loc_RecursiveFunction(int depth, std::stringstream& ss, Object& obj) {
	
	for (int i = 0; i < depth; i++) ss << "  ";
	ss << obj.name << "(" << obj.someName << ")";
	for(auto& child : obj.children) {
		ss << "\r\n";
		loc_RecursiveFunction(depth+1,ss,child);
	}
}

Object::operator std::string() {
	std::stringstream ss;
	loc_RecursiveFunction(0,ss,*this);
	return ss.str();
}

Object* Object::Find(std::string name) {
	Object* retVal = nullptr;
	if (this->name == name) return this;
	else for(Object& child : children) {
		if(retVal = child.Find(name)) {
			return retVal;
		}
	}
	return nullptr;
}
