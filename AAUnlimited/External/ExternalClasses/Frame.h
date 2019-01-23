#pragma once

#include <Windows.h>
#include <d3d9.h>
#include <queue>
#include "Script/ScriptLua.h"

#include "Bone.h"
#include "Material.h"


namespace ExtClass {

class XXFile;
struct Material;

class Frame
{
public:
	struct Submesh {
		BYTE m_blendMode;
		BYTE m_unknown00;	//usually 0x64
		BYTE m_unknown01;	//possibly padding
		BYTE m_outlinesEnabled;	//00 - diable, 01 - enable	//When an object lacks outlines it has the side-effect of being transparent from the inside out (behind the backfaces)
		BYTE m_flags0[6];
		BYTE m_enableGloss;	//00 - diable, 01 - enable	//Activates the gloss (reflection) texture. This is the 4th texture material slot and usually named with the "Asp" prefix.
		BYTE m_unknown02;
		BYTE m_flags[52]; //SB3U submesh flags
		DWORD m_unknown0; //unknown
		uint32_t m_faceCount; //Face Count (actually count*3, counts WORDS(vertex indizes))
		void* m_pointer1;
		void* m_pointer2;
		Material* m_material;
		uint32_t m_vertexCount;
		BYTE m_unknown1[24]; //unknown
		union {
			BYTE m_flagsUnknown2[20]; //SB3U submesh unknown2 flags
			float m_submeshOutline[5];
		};
		BYTE m_unknown2[136]; //unknown
		//BYTE m_flagsUnknown3[]; //unsure about the location of this one
		union {
			BYTE m_flagsUnknown4[284];
			struct {
				BYTE m_flagsUnknown4_1[256];
				float m_submeshShadow[4];
				float m_flagsUnknown4_lastFloats[2];
				DWORD m_flagsUnknown4_lastInt;
			};
		};
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
	union {
		BYTE m_frameFlags[0x20]; //SB3U Frame Flags
		struct {
			BYTE m_frameFlagsPadding0;
			BYTE m_enablePriority;	//00 - disabled, 01 - enabled	//Priority is only considered when it is enabled. Otherwise its value is ignored.
			BYTE m_frameFlagsPadding1[0x2];
			DWORD m_framePriority;
			BYTE m_frameFlagsPadding2[0x18];
		};
	};
	DWORD m_nSubmeshes; //number of submeshes
	DWORD m_readStuff[6]; //usually 0, something else on meshes
	Submesh* m_subMeshes; //SB3U Submesh Flags array
	DWORD m_unknown6;
	BYTE m_vertexListDuplicate[0x8]; // SB3U's VertexListDuplicate field in Mesh tab
	BYTE m_unknown2[0x8];
	DWORD m_nBones;		//only meshes have bones. else, this is 0
	Bone* m_bones;		//looks like this is not actually an array.
	float m_someXXCopy; //some value copied from the xx file. usually 1

	DWORD m_unkint;
	union {
		BYTE m_meshFlags[0x40];
		struct {
			BYTE m_meshFlagHide; // 0 show, 2 - dont show, this particular mesh only.
			BYTE m_meshFlagLightData;
			BYTE m_meshFlagsPadding0[0x2];
			BYTE m_meshFlagSubmeshBlendingEnabled;	//00 - disabled, 01 - enabled	//Enables submesh texture blend modes for this mesh
		};
	};

	XXFile* m_xxPartOf; //xx file that this frame belongs to
	BYTE m_unknown4[0x9]; //there are several flags here. dont know what they do. some crash if changed.
	BYTE m_renderFlag; //0: show, 2: dont show? works recursively.
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

	inline std::vector<std::pair<Frame*, int>> ListSubmeshesByMaterial(const char* material) {
		std::vector<std::pair<Frame*, int>> accumulator;
		EnumTreeLevelOrder([&accumulator, &material](Frame* frame) {
			if (frame->m_nSubmeshes < 1) return true;
			else {
				for (int i = 0; i < frame->m_nSubmeshes; i++) {
					if (!strcmp(frame->m_subMeshes[i].m_material->m_name, material)) {
						std::pair<Frame*, int> submesh { frame, i };
						accumulator.push_back(submesh);
					}
				}
				return true;
			}
		});
		return accumulator;
	}

	std::vector<DWORD> Frame::GetSubmeshOutlineColorArray(int idxSubmesh);
	void Frame::SetSubmeshOutlineColorArray(int idxSubmesh, std::vector<DWORD> color);

	std::vector<DWORD> Frame::GetSubmeshShadowColorArray(int idxSubmesh);
	void SetSubmeshShadowColorArray(int idxSubmesh, std::vector<DWORD> color);

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
		LUA_BIND(m_subMeshes)

		LUA_BIND(m_nBones)
		LUA_BINDARREP(m_bones,,_self->m_nBones)
		LUA_BIND(m_xxPartOf)
		LUA_BIND(m_renderFlag)
		LUA_BINDARR(m_meshFlags)
		LUA_BIND(m_meshFlagHide)
		LUA_BIND(m_meshFlagLightData)
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
static_assert(sizeof(Frame::Submesh) == 0x248, "size mismatch");

}
