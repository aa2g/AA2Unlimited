#pragma once
#include <Windows.h>
#include <vector>

#include "General\GuiSlider.h"
#include "External\ExternalClasses\CharacterStruct.h"

namespace Poser {

	void StartEvent();
	void EndEvent();
	void SetTargetCharacter(ExtClass::CharacterStruct* c);
	void FrameModEvent(ExtClass::XXFile* xxFile);

	class PoserWindow {
	public:
		void Init();
		void Hide();
	private:
		HWND m_dialog;
		HWND m_edPose;
		HWND m_edFrame;
		UINT_PTR m_timer;

		std::vector<GuiSlider> m_sliders;

		void InitSliders();
		void ApplySlider(int i);

		static INT_PTR CALLBACK DialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam);
	};

	extern PoserWindow g_PoserWindow;
};