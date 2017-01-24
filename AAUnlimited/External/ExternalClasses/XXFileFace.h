#pragma once
#include <Windows.h>

#include "External/AddressRule.h"

/*
* Represents an XXFile with values specific to faces. Can be safely casted to XXFile.
*/
class XXFileFace
{
public:
	XXFileFace();
	~XXFileFace();

	inline float* GetBlush() {
		const static DWORD rule[]{ 0x234, 0x44 * 0x8 + 0x0C, 0x194, 0x50, 0x14 };
		return (float*)ExtVars::ApplyRule(this, rule);
	}
	
	inline float* GetBlushLines() {
		const static DWORD rule[]{ 0x234, 0x44 * 0x8 + 0x0C, 0x194, 0x50, 0x148 };
		return (float*)ExtVars::ApplyRule(this, rule);
	}

	BYTE m_unknown1[0x234];
	// this pointer leads to blush materials. see GetBlush() and GetBlushLines()
	void* blushPointer;
	BYTE m_unknown2[0x4];
	// mouth shape
	int m_mouth;
	BYTE m_unknown3[0x18];
	// eye shape
	int m_eye;
	BYTE m_unknown4[0x18];
	// eyebrow shape
	int m_eyebrow;
	BYTE m_unknown5[0x30];
	// max eye open status
	float m_eyeOpen;
	BYTE m_unknown6[0x8];
	// min mouth open status
	float m_mouthOpen;
	BYTE m_unknown7[0x1C768];
	// lock eye tracking
	bool m_eyeTracking;
};

