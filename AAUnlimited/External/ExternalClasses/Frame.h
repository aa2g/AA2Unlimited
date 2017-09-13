#pragma once

#include <Windows.h>
#include <d3d9.h>
#include <queue>
#include "Script/ScriptLua.h"

#include "Bone.h"


namespace ExtClass {

class XXFile;

class Frame
{
public:
	struct SubmeshFlags {
		BYTE m_flags[64]; //SB3U submesh flags
		BYTE m_unknown1[48]; //unknown
		BYTE m_flagsUnknown2[20]; //SB3U submesh unknown2 flags
		BYTE m_unknown2[136]; //unknown
		//BYTE m_flagsUnknown3[]; //unsure about the location of this one
		BYTE m_flagsUnknown4[284];
		BYTE m_unknown3[32];
		//BYTE m_flagsUnknown5[]; //unsure about the location of this one
	};
	DWORD m_nameBufferSize;
	char* m_name;
	DWORD m_nChildren;
	Frame* m_children; //probably children
	Frame* m_parent;

	D3DMATRIX m_matrix1; //constantly renewed using different matrices; 
						 //contains offset of bone to parent when read in from xx file
	D3DMATRIX m_matrix2; //final generated matrix. used as a base to calculate matrix of sub-bones.
						 //some bones (base bones, i suppose, such as A00_kao) do NOT constantly modify this matrix.
						 //changing it has a permanent effect on the model in these cases

	D3DMATRIX m_matrix3; //not sure, usually identity matrix
	D3DMATRIX m_matrix4; //not sure, usually almost identity matrix (0.999999994 or some shit cause its inverted for some reason)

	D3DMATRIX m_matrix5; //constant translation matrix, copy of matrixx1 when read in;
						 //used by neck3 to build matrix1, maybe attachment matrix for other body parts?
	DWORD m_unknown1;
	BYTE m_frameFlags[0x20]; //SB3U Frame Flags
	DWORD m_nSubmeshes; //number of submeshes
	DWORD m_readStuff[6]; //usually 0, something else on meshes
	SubmeshFlags* m_subMeshFlags; //SB3U Submesh Flags array
	BYTE m_unknown2[0x14];
	DWORD m_nBones;		//only meshes have bones. else, this is 0
	Bone* m_bones;		//looks like this is not actually an array.
	float m_someXXCopy; //some value copied from the xx file. usually 1

	DWORD m_unkint;
	BYTE m_renderFlag2; // used by skirt hiding
	BYTE m_lightData; // sets up light, used by fixed light for custom props
	BYTE m_unkflags[0x40-2];

	XXFile* m_xxPartOf; //xx file that this frame belongs to
	BYTE m_unknown4[0x9]; //there are several flags here. dont know what they do. some crash if changed.
	BYTE m_renderFlag; //0: show, 2: dont show?
	BYTE m_unknown5[0x40EA];

	inline Frame* GetChild(int n) {
		if (n >= m_nChildren) return NULL;
		return &m_children[n];
	}

	inline Frame* FindFrame(const char* name) {
		Frame* child = nullptr;
		EnumTreeLevelOrder([&child, &name](Frame* frame) {
			int idx = strcmp(frame->m_name, name);
			if (!idx) {
				child = frame;
				return false; //stop
			}
			return true; //continue
		});
		return child;
	}

	template<class Callback>
	void EnumTreeLevelOrder(Callback& callback);

	static inline void bindLua() {
#define LUA_CLASS Frame
		LUA_NAME;
		LUA_BINDSTRP(m_name)
		LUA_BIND(m_nChildren)
		LUA_BINDARREP(m_children,, _self->m_nChildren)
		LUA_BIND(m_parent)
		LUA_BINDARRE(m_matrix1,.m[0], 16)
		LUA_BINDARRE(m_matrix2,.m[0], 16)
		LUA_BINDARRE(m_matrix3,.m[0], 16)
		LUA_BINDARRE(m_matrix4,.m[0], 16)
		LUA_BINDARRE(m_matrix5,.m[0], 16)

		LUA_BINDARR(m_frameFlags)
		LUA_BIND(m_nSubmeshes)
		LUA_BIND(m_subMeshFlags)

		LUA_BIND(m_nBones)
		LUA_BINDARREP(m_bones,,_self->m_nBones)
		LUA_BIND(m_xxPartOf)
		LUA_BIND(m_renderFlag)
		LUA_BIND(m_renderFlag2)
		LUA_BIND(m_lightData)
		LUA_BINDP(m_unkflags)
		LUA_METHOD(FindFrame, {
			Frame* child = nullptr;
			if (_gl.top() == 2) {
				child = _self->FindFrame(_gl.get(2));
			}
			_gl.push(child);
			return 1;
		})
#undef LUA_CLASS
	};
};

template<class Callback>
void Frame::EnumTreeLevelOrder(Callback& callback) {
	std::queue<Frame*> q;
	q.push(this);

	bool ret;
	Frame* it;
	while (!q.empty()) {
		it = q.front();
		q.pop();
		for (int i = 0; i < it->m_nChildren; i++) {
			q.push(&it->m_children[i]);
		}
		ret = callback(it);
		if (!ret) break;
	}
}

static_assert(sizeof(Frame) == 0x42F4,"size mismatch");
static_assert(sizeof(Frame::SubmeshFlags) == 0x248, "size mismatch");

}
