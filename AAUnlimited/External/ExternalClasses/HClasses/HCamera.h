#pragma once

#include <Windows.h>
#include <d3d9.h>
#include "Script/ScriptLua.h"

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

#define LUA_CLASS HCamera
	static inline void bindLua() {
		LUA_EXTCLASS(HCamera,
			LUA_FIELD(m_xShift),
			LUA_FIELD(m_yShift),
			LUA_FIELD(m_zShift),
			LUA_FIELD(m_fov),
			LUA_FIELD(m_zRotRad),
			LUA_FIELD(m_yRotRad),
			LUA_FIELD(m_xRotRad)
		);
	}
#undef LUA_CLASS

	D3DMATRIX m_matrix; //used, but typically identity matrix. can be used to distort view
};

}