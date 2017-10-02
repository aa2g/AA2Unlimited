#pragma once

#include <Windows.h>

#include "Animation.h"
#include "Script/ScriptLua.h"
#include "Script/glua_stl.h"
#include "External/ExternalClasses/Material.h"


namespace ExtClass {

class Frame;

#pragma pack(push, 1)
/*
* Represents an in-game XX file structure
*/
class XXFile
{
public:
	DWORD m_unknown;
	char m_name[0x200];
	BYTE m_unknown2[0x8];
	Frame* m_attachmentFrame; // 0x20C
	BYTE m_unknown3[0x8];
	Frame* m_root; // 0x218

	BYTE m_unknown4[0x8];
	void* m_morphsPointer1; // 0x224
	DWORD m_morphsUnknown1;
	void* m_morphShapesPointer;
	DWORD m_morphsUnknown2;
	void* m_morphsPointer3;

	BYTE m_unknown5[0x4];
	int m_mouth; // 0x23C
	BYTE m_unknown6[0x18];
	int m_eye; // 0x258
	BYTE m_unknown7[0x18];
	int m_eyebrow; // 0x274
	BYTE m_unknown8[0x30];
	float m_eyeOpenMax;
	float m_eyeOpenMin;
	float m_mouthOpenMax;
	float m_mouthOpenMin;

	BYTE m_unknown10[0x78];
	uint32_t m_materialCount; // 0x330
	Material* m_materialArray;
	BYTE m_unknown11[0x1C];
	DWORD m_animArraySize; // 0x354
	Animation* m_animArray;
	BYTE m_unknown12[0x1C018]; // 0x35C
	DWORD m_poseNumber;
	BYTE m_unknown13[0x8];
	float m_animFrame; // 0c1C380
	BYTE m_unknown14[0x69C];
	DWORD m_eyeTracking; //0x1CA20

	template<class Callback>
	void EnumBonesPreOrder_sub(Callback& callback, Frame* bone);
	template<class Callback>
	void EnumBonesPostOrder_sub(Callback& callback,Frame* bone);
public:
	static inline void bindLua() {
#define LUA_CLASS ExtClass::XXFile
			LUA_NAME;
			LUA_BINDSTR(m_name)
			LUA_BIND(m_attachmentFrame);
			LUA_BIND(m_root);
			LUA_BINDARREP(m_animArray,,_self->m_animArraySize)
			LUA_BIND(m_poseNumber);
			LUA_BIND(m_animFrame);

			LUA_MGETTER1(FindBone);
			LUA_MGETTER1(FindMaterial);
			LUA_MGETTER1(Unload);

			LUA_BIND(m_mouth);
			LUA_BIND(m_eye);
			LUA_BIND(m_eyebrow);
			LUA_BIND(m_eyeOpenMin);
			LUA_BIND(m_eyeOpenMax);
			LUA_BIND(m_mouthOpenMin);
			LUA_BIND(m_mouthOpenMax);
			LUA_BIND(m_eyeTracking);

			LUA_ARRAYITERATOR(materials, m_materialArray, _self->m_materialCount);
#undef LUA_CLASS
	};

	XXFile() = delete;
	~XXFile() = delete;

	inline bool Unload(DWORD kls) {
		bool(__cdecl *unload)(DWORD , XXFile *);
		if (General::IsAAEdit) {
			unload = decltype(unload)(General::GameBase + 0x1E5530);
		}
		else {
			unload = decltype(unload)(General::GameBase + 0x202930);
		}
		return unload(kls, this);
	}

	//finds a bone belonging to this xx file in depth-first-search
	//with a maximum depth of maxDepth (or infinity if maxDepth < 0)
	Frame* FindBone(const char* name, int maxDepth = -1);
	Material* FindMaterial(const char* name);


	//for each function
	template<class Callback>
	void EnumBonesPreOrder(Callback& callback);
	template<class Callback>
	void EnumBonesPostOrder(Callback& callback);

	

};
#pragma pack(pop)

static_assert(sizeof(XXFile) == 0x1CA24, "XXFile size mismatch; must be 0x1CA24 bytes");


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