#pragma once

#include <Windows.h>

#include "Bone.h"


namespace ExtClass {

#pragma pack(push, 1)
/*
* Represents a texture.
*/
class XXFile
{
public:
	DWORD m_unknown;
	char m_name[512];
	BYTE m_unknown2[0x14];
	Bone* m_root;
public:
	XXFile() = delete;
	~XXFile() = delete;

	//finds a bone belonging to this xx file in depth-first-search
	//with a maximum depth of maxDepth (or infinity if maxDepth < 0)
	Bone* FindBone(const char* name, int maxDepth = -1);


};
#pragma pack(pop)

static_assert(sizeof(XXFile) == 0x21C,"XXFile size missmatch; must be 0x21C bytes");


}