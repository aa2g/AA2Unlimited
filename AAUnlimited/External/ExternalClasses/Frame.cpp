#include "StdAfx.h"

namespace ExtClass {

	std::vector<DWORD> Frame::GetSubmeshOutlineColorArray(int idxSubmesh) {

		union {
			DWORD i;
			float f;
		} uOutlineT;

		auto colorHex = this->m_subMeshes[idxSubmesh].m_submeshOutline;

		float fOutlineR = colorHex[0];
		float fOutlineG = colorHex[1];
		float fOutlineB = colorHex[2];
		float fOutlineA = colorHex[3];
		uOutlineT.f = colorHex[4];

		DWORD iOutlineR = 255 * fOutlineR;
		DWORD iOutlineG = 255 * fOutlineG;
		DWORD iOutlineB = 255 * fOutlineB;
		DWORD iOutlineT = uOutlineT.i;

		std::vector<DWORD> color{ iOutlineR , iOutlineG , iOutlineB , iOutlineT };

		return color;

	}

	void Frame::SetSubmeshOutlineColorArray(int idxSubmesh, std::vector<DWORD> color) {

		union {
			DWORD i;
			float f;
		} uOutlineT;

		auto colorHex = this->m_subMeshes[idxSubmesh].m_submeshOutline;

		float fOutlineR = color[0] * 1.0f / 255.0f;
		float fOutlineG = color[1] * 1.0f / 255.0f;
		float fOutlineB = color[2] * 1.0f / 255.0f;
		float fOutlineA = 1.0f;
		uOutlineT.i = color[3];
		float fOutlineT = uOutlineT.f;

		colorHex[0] = fOutlineR;
		colorHex[1] = fOutlineG;
		colorHex[2] = fOutlineB;
		colorHex[3] = fOutlineA;
		colorHex[4] = fOutlineT;

	}

	std::vector<DWORD> Frame::GetSubmeshShadowColorArray(int idxSubmesh) {

		union {
			DWORD i;
			float f;
		} uShadowA;

		auto colorHex = this->m_subMeshes[idxSubmesh].m_submeshShadow;

		float fShadowR = colorHex[0];
		float fShadowG = colorHex[1];
		float fShadowB = colorHex[2];
		uShadowA.f = colorHex[3];

		DWORD iShadowR = 255 * fShadowR;
		DWORD iShadowG = 255 * fShadowG;
		DWORD iShadowB = 255 * fShadowB;
		DWORD iShadowA = uShadowA.i;

		std::vector<DWORD> color{ iShadowR , iShadowG , iShadowB , iShadowA };

		return color;

	}

	void Frame::SetSubmeshShadowColorArray(int idxSubmesh, std::vector<DWORD> color) {

		union {
			DWORD i;
			float f;
		} uShadowA;

		auto colorHex = this->m_subMeshes[idxSubmesh].m_submeshShadow;

		float fShadowR = color[0] * 1.0f / 255.0f;
		float fShadowG = color[1] * 1.0f / 255.0f;
		float fShadowB = color[2] * 1.0f / 255.0f;
		uShadowA.i = color[3];
		float fShadowA = uShadowA.f;

		colorHex[0] = fShadowR;
		colorHex[1] = fShadowG;
		colorHex[2] = fShadowB;
		colorHex[3] = fShadowA;

	}


}