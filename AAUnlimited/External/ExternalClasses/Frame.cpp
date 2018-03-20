#include "StdAfx.h"

namespace ExtClass {

	std::vector<DWORD> Frame::GetSubmeshOutlineColorArray(int idxSubmesh) {

		auto colorHex = this->m_subMeshes[idxSubmesh].m_submeshOutline;

		float fOutlineR = colorHex[0];
		float fOutlineG = colorHex[1];
		float fOutlineB = colorHex[2];
		float fOutlineA = colorHex[3];
		float fOutlineT = colorHex[4];

		DWORD iOutlineR = 255 * fOutlineR;
		DWORD iOutlineG = 255 * fOutlineG;
		DWORD iOutlineB = 255 * fOutlineB;
		DWORD iOutlineA = 255 * fOutlineA;
		DWORD iOutlineT = (DWORD)fOutlineT;

		std::vector<DWORD> color{ iOutlineR , iOutlineG , iOutlineB , iOutlineT };

		return color;

	}

	void Frame::SetSubmeshOutlineColorArray(int idxSubmesh, std::vector<DWORD> color) {

		auto colorHex = this->m_subMeshes[idxSubmesh].m_submeshOutline;

		float fOutlineR = color[0] * 1.0f / 255.0f;
		float fOutlineG = color[1] * 1.0f / 255.0f;
		float fOutlineB = color[2] * 1.0f / 255.0f;
		float fOutlineA = 1.0f;
		float fOutlineT = (float)color[3];

		colorHex[0] = fOutlineR;
		colorHex[1] = fOutlineG;
		colorHex[2] = fOutlineB;
		colorHex[3] = fOutlineA;
		colorHex[4] = fOutlineT;

	}

}