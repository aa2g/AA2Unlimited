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
		HWND m_edValue;
		HWND m_listBones;
		HWND m_listOperation;
		HWND m_listAxis;
		HWND m_sliderValue;
		UINT_PTR m_timer;
		int m_curBone;

		void InitBones();
		void ApplySlider();
		void SyncEdit();
		void SyncList();
		void SyncSlider();

		static INT_PTR CALLBACK DialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam);
	};

	extern PoserWindow g_PoserWindow;
};