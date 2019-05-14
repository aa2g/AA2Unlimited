#pragma once

#include <Windows.h>
#include <d3d9.h>
#include "Script/ScriptLua.h"

namespace ExtClass {

class HInfo;
class Frame;

class Camera {
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

#define LUA_CLASS ExtClass::Camera
	static inline void bindLua() {
	LUA_BIND(m_xShift)
	LUA_BIND(m_yShift)
	LUA_BIND(m_zShift)
	LUA_BIND(m_distToMid)
	LUA_BIND(m_fov)
	LUA_BIND(m_zRotRad)
	LUA_BIND(m_yRotRad)
	LUA_BIND(m_xRotRad)
	LUA_BINDARRE(m_matrix, .m[0], 16)
	}
#undef LUA_CLASS
	inline static Camera* GetCamera() {
		DWORD rule[] { (General::IsAAPlay?0x376164:0x35315C), 0x38, 0x24, 0x4, 0x198, 0 };
		return (Camera*)ExtVars::ApplyRule(rule);
	}
	static void PostTick(ExtClass::HInfo* hInfo, bool tickRetVal);
	static int SetFocusBone(ExtClass::Frame* bone, double x, double y, double z, bool);
	static void InitPovParams(int stabilize_percents);
	static void ResetPovToNormal();

	D3DMATRIX m_matrix; //used, but typically identity matrix. can be used to distort view
};

}