#pragma once

#include <Windows.h>

#include "Frame.h"

namespace ExtClass {

#pragma pack(push, 1)
	/*
	* Represents a texture.
	*/
	class Keyframe
	{
	public:
		DWORD m_keyframeNumber;
		float m_quatX; //quaterion used for rotation
		float m_quatY;
		float m_quatZ;
		float m_quatW;
		DWORD m_unknown1;
		DWORD m_unknown2;
		float m_transX;
		float m_transY;
		float m_transZ;
		float m_scaleZ;
		float m_scaleY;
		float m_scaleX;
	public:
		//notabily not deleted, can be copied and constructed correctly


	};
#pragma pack(pop)

	static_assert(sizeof(Keyframe) == 0x34,"Animation size missmatch; must be 0x18 bytes");


}