#include "XXFile.h"


DWORD FileFormats::XXFile::ReadObjectLength(BYTE * file) {
	int xxversion = 8;

	BYTE* it = file;

	//name
	DWORD nameLength;
	nameLength = *(DWORD*)(it);
	it += 4 + nameLength;

	//n children
	DWORD nChildren;
	nChildren = *(DWORD*)(it);
	it += 4;
	//trans matrix
	it += 16*4;

	//skip versiondependent bytes
	it += xxversion < 8 ? 0x10 : 0x20;

	DWORD meshCount;
	meshCount = *(DWORD*)(it);
	it += 4;

	//skip rest
	it += 3*4 + 3*4;
	it += xxversion < 8 ? 0x10 : 0x40;
	if (xxversion >= 6) {
		DWORD nameLength;
		nameLength = *(DWORD*)(it);
		it += 4 + nameLength;
	}

	if (meshCount > 0) {
		//mesh info
		it++;
		for (DWORD i = 0; i < meshCount; i++) {
			it += xxversion < 8 ? 0x10 : 0x40;
			it += 4;


			DWORD faceCount;
			faceCount = *(DWORD*)(it);
			it += 4;

			it += faceCount*2;


			DWORD vertexCount;
			vertexCount = *(DWORD*)(it);
			it += 4;

			it += vertexCount*70;


			if (xxversion >= 5) {
				it += 4*5;
			}
			if (xxversion >= 6) {
				it += 0x60 + 4;
			}
			if (xxversion < 6) {
				it += 0x40;
			}
			else {
				it += 0x100;
			}
			if (xxversion >= 6) {
				it += 0x1C;
			}
			if (xxversion >= 8) {
				it += 1;

				nameLength = *(DWORD*)(it);
				it += 4 + nameLength;

				it += 4 + 3*4;
			}
		}

		WORD mysteryCount;
		mysteryCount = *(DWORD*)(it);
		it += 2;
		it += 8;
		if (xxversion >= 6) {
			it += 70*mysteryCount;
		}
		else {
			
		}

		//bone info
		DWORD boneCount;
		boneCount = *(DWORD*)(it);
		it += 4;

		for (DWORD i = 0; i < boneCount; i++) {
			nameLength = *(DWORD*)(it);
			it += 4;
			it += nameLength;
			it += 4 + 64;
		}
	}

	if (nChildren != 0) {
		for (DWORD i = 0; i < nChildren; i++) {
			it += ReadObjectLength(it);
		}
	}

	DWORD size = it - file;

	return size;
}