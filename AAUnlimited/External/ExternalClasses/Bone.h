#pragma once

#include <Windows.h>
#include <d3d9.h>

namespace ExtClass {

class Bone
{
public:
	DWORD m_nameBufferSize;
	char* m_name;
	DWORD m_arrSize;
	Bone* m_boneArray; //probably children
	Bone* m_parent;

	D3DMATRIX m_matrix1; //constantly renewed using different matrices; contains offset of bone to parent when read in from xx file
	D3DMATRIX m_matrix2; //same, also contains matrix1

	D3DMATRIX m_matrix3; //not sure, usually identity matrix
	D3DMATRIX m_matrix4; //not sure, usually almost identity matrix (0.999999994 or some shit cause its inverted for some reason)

	D3DMATRIX m_matrix5; //constant translation matrix, copy of matrixx1 when read in;
						 //used by neck3 to build matrix1, maybe attachment matrix for other body parts?
};

}