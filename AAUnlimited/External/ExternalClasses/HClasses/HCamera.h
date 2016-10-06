#pragma once

#include <Windows.h>
#include <d3d9.h>

namespace ExtClass {

class HCamera {
public:
	DWORD m_unknown1;
	float m_xRotRad; //pitch
	float m_yRotRad; //yaw
	float m_zRotRad; //roll

	float m_distToMid;
	float m_fov;

	float m_zShift;
	float m_yShift;
	float m_xShift;

	D3DMATRIX m_matrix; //used, but typically identity matrix. can be used to distort view
};

}