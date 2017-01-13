#pragma once

#include <Windows.h>
#include <d3d9.h>

#include "Bone.h"


namespace ExtClass {

class XXFile;

class Frame
{
public:
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
	BYTE m_unknown1[0x24];
	DWORD m_type; //type i think. usually 0, but 1 for meshes.
	DWORD m_readStuff[6]; //usually 0, something else on meshes
	BYTE m_unknown2[0x18];
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

}