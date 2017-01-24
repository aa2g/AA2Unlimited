#pragma once

#include <Windows.h>
#include <d3d9.h>

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
	BYTE m_unknown3[0x44];
	XXFile* m_xxPartOf; //xx file that this frame belongs to
	BYTE m_unknown4[0x9]; //there are several flags here. dont know what they do. some crash if changed.
	BYTE m_renderFlag; //0: show, 2: dont show?
	BYTE m_unknown5[0x40EA];
};

static_assert(sizeof(Frame) == 0x42F4,"size mismatch");
static_assert(sizeof(Frame::SubmeshFlags) == 0x248, "size mismatch");

}