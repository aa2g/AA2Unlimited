#pragma once

#include <Windows.h>

#include "Bone.h"
#include "Animation.h"


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
	BYTE m_unknown3[0x138];
	DWORD m_animArraySize;
	Animation* m_animArray;

	template<class Callback>
	void EnumBonesPreOrder_sub(Callback& callback, Bone* bone);
public:
	XXFile() = delete;
	~XXFile() = delete;

	//finds a bone belonging to this xx file in depth-first-search
	//with a maximum depth of maxDepth (or infinity if maxDepth < 0)
	Bone* FindBone(const char* name, int maxDepth = -1);


	//for each function
	template<class Callback>
	void EnumBonesPreOrder(Callback& callback);

	

};
#pragma pack(pop)

static_assert(sizeof(XXFile) == 0x35C,"XXFile size missmatch; must be 0x35C bytes");


template<class Callback>
void XXFile::EnumBonesPreOrder(Callback& callback) {
	if (!m_root) return;
	EnumBonesPreOrder_sub(callback,m_root);
}

template<class Callback>
void XXFile::EnumBonesPreOrder_sub(Callback& callback,Bone* bone) {
	callback(bone);
	for(DWORD i = 0; i < bone->m_arrSize; i++) {
		EnumBonesPreOrder_sub(callback,&bone->m_boneArray[i]);
	}
}


}