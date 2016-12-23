#pragma once

#include <Windows.h>

#include "Frame.h"
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
	BYTE m_unknown2[0x8];
	Frame* m_attachmentFrame;
	BYTE m_unknown3[0x8];
	Frame* m_root;
	BYTE m_unknown4[0x138];
	DWORD m_animArraySize;
	Animation* m_animArray;
	BYTE m_unknown5[0x1C018];
	DWORD m_poseNumber;
	BYTE m_unknown6[8];
	float m_animFrame;

	template<class Callback>
	void EnumBonesPreOrder_sub(Callback& callback, Frame* bone);
	template<class Callback>
	void EnumBonesPostOrder_sub(Callback& callback,Frame* bone);
public:
	XXFile() = delete;
	~XXFile() = delete;

	//finds a bone belonging to this xx file in depth-first-search
	//with a maximum depth of maxDepth (or infinity if maxDepth < 0)
	Frame* FindBone(const char* name, int maxDepth = -1);


	//for each function
	template<class Callback>
	void EnumBonesPreOrder(Callback& callback);
	template<class Callback>
	void EnumBonesPostOrder(Callback& callback);

	

};
#pragma pack(pop)

static_assert(sizeof(XXFile) == 0x1C384,"XXFile size missmatch; must be 0x35C bytes");


template<class Callback>
void XXFile::EnumBonesPreOrder(Callback& callback) {
	if (!m_root) return;
	EnumBonesPreOrder_sub(callback,m_root);
}

template<class Callback>
void XXFile::EnumBonesPostOrder(Callback& callback) {
	if (!m_root) return;
	EnumBonesPostOrder_sub(callback,m_root);
}

template<class Callback>
void XXFile::EnumBonesPreOrder_sub(Callback& callback,Frame* bone) {
	callback(bone);
	for(DWORD i = 0; i < bone->m_nChildren; i++) {
		EnumBonesPreOrder_sub(callback,&bone->m_children[i]);
	}
}

template<class Callback>
void XXFile::EnumBonesPostOrder_sub(Callback& callback,Frame* bone) {
	for (DWORD i = 0; i < bone->m_nChildren; i++) {
		EnumBonesPostOrder_sub(callback,&bone->m_children[i]);
	}
	callback(bone);
}


}