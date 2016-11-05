#pragma once

#include <Windows.h>

#include "Frame.h"
#include "Keyframe.h"

namespace ExtClass {

#pragma pack(push, 1)
	/*
	* Represents a texture.
	*/
	class Animation
	{
	public:
		DWORD m_nameSize;
		char* m_name;
		DWORD m_nFrames;
		DWORD m_unknown;
		Keyframe* m_frameArray;
		Frame* m_bone;
	public:
		Animation() = delete;
		~Animation() = delete;


	};
#pragma pack(pop)

	static_assert(sizeof(Animation) == 0x18,"Animation size missmatch; must be 0x18 bytes");


}