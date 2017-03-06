#pragma once
#include <Windows.h>
#include <vector>

#include "General\GuiSlider.h"
#include "External\ExternalClasses\CharacterStruct.h"

namespace Poser {

	enum EventType {
		NoEvent,
		ClothingScene,
		NpcInteraction,
		DoubleNpcInteraction,
		HMode
	};

	void StartEvent(EventType type);
	void EndEvent();
	void SetTargetCharacter(ExtClass::CharacterStruct* c);
	void FrameModEvent(ExtClass::XXFile* xxFile);

	class PoserWindow {
	public:
		void Init();
		void Hide();
		void NewCharacter(int index);
	private:
		HWND m_dialog;
		HWND m_edCharacter;
		HWND m_edPose;
		HWND m_edFrame;
		HWND m_edValueX;
		HWND m_edValueY;
		HWND m_edValueZ;
		HWND m_edMouth;
		HWND m_edMouthOpen;
		HWND m_edEye;
		HWND m_edEyeOpen;
		HWND m_edEyebrow;
		HWND m_edBlush;
		HWND m_edBlushLines;
		HWND m_spinCharacter;
		HWND m_spinPose;
		HWND m_spinFrame;
		HWND m_spinMouth;
		HWND m_spinMouthOpen;
		HWND m_spinEye;
		HWND m_spinEyeOpen;
		HWND m_spinEyebrow;
		HWND m_spinBlush;
		HWND m_spinBlushLines;
		HWND m_listCategories;
		HWND m_listBones;
		HWND m_listOperation;
		HWND m_sliderValueX;
		HWND m_sliderValueY;
		HWND m_sliderValueZ;
		HWND m_chkEyeTrack;
		HWND m_chkTears;
		HWND m_chkDimEyes;
		HWND m_chkTongueJuice;
		HWND m_chkGlasses;
		HWND m_chkAlwaysOnTop;
		HWND m_tabModifiers;
		HWND m_tabShowHide;
		UINT_PTR m_timer;
		int m_curBone;

		void ApplyIncrement(int axis, int sign);
		void SyncOperation();
		void SyncBones();
		void SyncEdit();
		void SyncList();
		void SyncSlider();
		void LoadPose(const TCHAR* path);
		void SavePose(const TCHAR* path);
		void LoadCloth(std::vector<BYTE> &file);

		static INT_PTR CALLBACK DialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam);
	};

	extern PoserWindow g_PoserWindow;
};