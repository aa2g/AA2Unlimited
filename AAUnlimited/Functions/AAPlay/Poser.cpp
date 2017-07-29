#include "Poser.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <CommCtrl.h>
#include <Strsafe.h>
#include <fstream>
#include <Commctrl.h>

#include "External\AddressRule.h"
#include "External\ExternalClasses\Frame.h"
#include "External\ExternalClasses\XXFileFace.h"
#include "General\IllusionUtil.h"
#include "General\Util.h"
#include "Functions\Shared\Globals.h"
#include "Functions\AAPlay\GameState.h"
#include "Files\PoseMods.h"
#include "Files\Config.h"
#include "Files\PoseFile.h"
#include "Files\ClothFile.h"
#include "resource.h"
#include "config.h"

#include "PoserController.h"
#include "3rdparty\picojson\picojson.h"

namespace Poser {

#define X 0
#define Y 1
#define Z 2
#define W 3

	PoserWindow g_PoserWindow;
	PoserController g_PoserController;

	bool loc_syncing;

	void StartEvent(EventType type) {
		if (!g_Config.GetKeyValue(Config::USE_POSER_CLOTHES).bVal && type == ClothingScene) return;
		if (!g_Config.GetKeyValue(Config::USE_POSER_DIALOGUE).bVal
			&& (type == NpcInteraction || type == HMode)) return;
		g_PoserController.GenSliderInfo();
		g_PoserWindow.Init();
	}

	void EndEvent() {
		g_PoserController.StopPoser();
		g_PoserWindow.Hide();
	}

	void LoadCharacter(ExtClass::CharacterStruct* charStruct) {
		if (g_Config.GetKeyValue(Config::USE_POSER_DIALOGUE).bVal || g_Config.GetKeyValue(Config::USE_POSER_CLOTHES).bVal) {
			g_PoserWindow.Init();
			g_PoserController.StartPoser();
			g_PoserController.SetTargetCharacter(charStruct);
			for (auto& s : g_PoserController.CurrentCharacter()->m_sliders) {
				s.guide = nullptr;
			}
		}
	}

	void LoadCharacterEnd() {
		//g_PoserController.SetHidden(ExtClass::CharacterStruct::Models::SKELETON, "guide_", true);
		if (!g_PoserController.IsActive()) return;
		g_PoserWindow.SyncBones();
		g_PoserWindow.SyncOperation();
	}

	bool OverrideFile(wchar_t** paramArchive, wchar_t** paramFile, DWORD* readBytes, BYTE** outBuffer) {
		if (!g_PoserController.IsActive()) {
			return false;
		}
		std::wstring& override = g_PoserController.GetOverride(std::wstring(*paramFile));
		override = General::BuildOverridePath(override.c_str());
		HANDLE hFile = CreateFile(override.c_str(), FILE_GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) {
			return false;
		}
		DWORD lo, hi;
		lo = GetFileSize(hFile, &hi);
		void* fileBuffer = Shared::IllusionMemAlloc(lo);
		ReadFile(hFile, fileBuffer, lo, &hi, NULL);
		CloseHandle(hFile);
		*outBuffer = (BYTE*)fileBuffer;
		*readBytes = hi;
		return true;
	}

	inline PoserController::SliderInfo* CurrentSlider() {
		return g_PoserController.CurrentSlider();
	}

	void PoserWindow::Init() {
		if (g_PoserController.IsActive())
			return;
		if (m_dialog == NULL) {
			CreateDialogParam(General::DllInst, MAKEINTRESOURCE(IDD_PLAY_POSE),
				NULL, DialogProc, (LPARAM)this);
		}
		if (!m_timer)
			m_timer = SetTimer(m_dialog, 1, 2000, NULL);
		EnableWindow(g_PoserWindow.m_dialog, TRUE);
		ShowWindow(m_dialog, SW_SHOW);
		if (!g_PoserController.m_sliderCategories[PoseMods::Other].empty()) {
			SendMessage(m_listCategories, LB_SETCURSEL, PoseMods::Other, 0);
		}
	}

	void PoserWindow::Hide() {
		KillTimer(m_dialog, m_timer);
		m_timer = 0;
		EnableWindow(g_PoserWindow.m_dialog, FALSE);
		if (SendMessage(g_PoserWindow.m_chkAlwaysOnTop, BM_GETCHECK, 0, 0) == BST_CHECKED)
			ShowWindow(m_dialog, SW_HIDE);
	}

	INT_PTR CALLBACK PoserWindow::DialogProc(_In_ HWND hwndDlg, _In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam) {
		static bool ignoreNextSlider = false;

		//set hotkeys		
		auto hkTranslate = g_Config.GetKeyValue(Config::HKEY_POSER_TRANSLATE).bVal;	//W
		auto hkRotate = g_Config.GetKeyValue(Config::HKEY_POSER_ROTATE).bVal;	//E
		auto hkScale = g_Config.GetKeyValue(Config::HKEY_POSER_SCALE).bVal;	//R

		switch (msg) {
		case WM_INITDIALOG: {
			PoserWindow* thisPtr = (PoserWindow*)lparam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lparam); //register class to this hwnd
			thisPtr->m_dialog = hwndDlg;
			thisPtr->m_edPose = GetDlgItem(hwndDlg, IDC_PPS_EDPOSE);
			thisPtr->m_edCharacter = GetDlgItem(hwndDlg, IDC_PPS_EDCHARACTER);
			thisPtr->m_edFrame = GetDlgItem(hwndDlg, IDC_PPS_EDFRAME);
			thisPtr->m_edValueX = GetDlgItem(hwndDlg, IDC_PPS_EDVALUEX);
			thisPtr->m_edValueY = GetDlgItem(hwndDlg, IDC_PPS_EDVALUEY);
			thisPtr->m_edValueZ = GetDlgItem(hwndDlg, IDC_PPS_EDVALUEZ);
			thisPtr->m_edMouth = GetDlgItem(hwndDlg, IDC_PPS_EDMOUTH);
			thisPtr->m_edMouthOpen = GetDlgItem(hwndDlg, IDC_PPS_EDMOUTHOPEN);
			thisPtr->m_edEye = GetDlgItem(hwndDlg, IDC_PPS_EDEYE);
			thisPtr->m_edEyeOpen = GetDlgItem(hwndDlg, IDC_PPS_EDEYEOPEN);
			thisPtr->m_edEyebrow = GetDlgItem(hwndDlg, IDC_PPS_EDEYEBROW);
			thisPtr->m_edBlush = GetDlgItem(hwndDlg, IDC_PPS_EDBLUSH);
			thisPtr->m_edBlushLines = GetDlgItem(hwndDlg, IDC_PPS_EDBLUSH2);
			thisPtr->m_spinCharacter = GetDlgItem(hwndDlg, IDC_PPS_SPINCHARACTER);
			thisPtr->m_spinPose = GetDlgItem(hwndDlg, IDC_PPS_SPINPOSE);
			thisPtr->m_spinFrame = GetDlgItem(hwndDlg, IDC_PPS_SPINFRAME);
			thisPtr->m_spinMouth = GetDlgItem(hwndDlg, IDC_PPS_SPINMOUTH);
			thisPtr->m_spinMouthOpen = GetDlgItem(hwndDlg, IDC_PPS_SPINMOUTHOPEN);
			thisPtr->m_spinEye = GetDlgItem(hwndDlg, IDC_PPS_SPINEYE);
			thisPtr->m_spinEyeOpen = GetDlgItem(hwndDlg, IDC_PPS_SPINEYEOPEN);
			thisPtr->m_spinEyebrow = GetDlgItem(hwndDlg, IDC_PPS_SPINEYEBROW);
			thisPtr->m_spinBlush = GetDlgItem(hwndDlg, IDC_PPS_SPINBLUSH);
			thisPtr->m_spinBlushLines = GetDlgItem(hwndDlg, IDC_PPS_SPINBLUSH2);
			thisPtr->m_listCategories = GetDlgItem(hwndDlg, IDC_PPS_LISTCATEGORIES);
			thisPtr->m_listBones = GetDlgItem(hwndDlg, IDC_PPS_LISTBONES);
			thisPtr->m_listOperation = GetDlgItem(hwndDlg, IDC_PPS_LISTOP);
			thisPtr->m_sliderValueX = GetDlgItem(hwndDlg, IDC_PPS_SLIDERVALUEX);
			thisPtr->m_sliderValueY = GetDlgItem(hwndDlg, IDC_PPS_SLIDERVALUEY);
			thisPtr->m_sliderValueZ = GetDlgItem(hwndDlg, IDC_PPS_SLIDERVALUEZ);
			thisPtr->m_chkEyeTrack = GetDlgItem(hwndDlg, IDC_PPS_CHKEYETRACK);
			thisPtr->m_chkAlwaysOnTop = GetDlgItem(hwndDlg, IDC_PPS_CHKALWAYSONTOP);
			thisPtr->m_chkTears = GetDlgItem(hwndDlg, IDC_PPS_CHKTEARS);
			thisPtr->m_chkDimEyes = GetDlgItem(hwndDlg, IDC_PPS_CHKDIMEYES);
			thisPtr->m_chkTongueJuice = GetDlgItem(hwndDlg, IDC_PPS_CHKTONGUEJUICE);
			thisPtr->m_chkShowGuides = GetDlgItem(hwndDlg, IDC_PPS_CHKSHOWGUIDES);
			thisPtr->m_tabModifiers = GetDlgItem(hwndDlg, IDC_PPS_TABMODIFIERS);
			thisPtr->m_tabShowHide = GetDlgItem(hwndDlg, IDC_PPS_TABSHOWHIDE);
			SetWindowPos(thisPtr->m_dialog, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);


			loc_syncing = true;
			SendMessage(thisPtr->m_listCategories, LB_ADDSTRING, 0, LPARAM(L"Torso"));
			SendMessage(thisPtr->m_listCategories, LB_ADDSTRING, 0, LPARAM(L"Left Arm"));
			SendMessage(thisPtr->m_listCategories, LB_ADDSTRING, 0, LPARAM(L"Right Arm"));
			SendMessage(thisPtr->m_listCategories, LB_ADDSTRING, 0, LPARAM(L"Left Hand"));
			SendMessage(thisPtr->m_listCategories, LB_ADDSTRING, 0, LPARAM(L"Right Hand"));
			SendMessage(thisPtr->m_listCategories, LB_ADDSTRING, 0, LPARAM(L"LeftLeg"));
			SendMessage(thisPtr->m_listCategories, LB_ADDSTRING, 0, LPARAM(L"RightLeg"));
			SendMessage(thisPtr->m_listCategories, LB_ADDSTRING, 0, LPARAM(L"Breasts"));
			SendMessage(thisPtr->m_listCategories, LB_ADDSTRING, 0, LPARAM(L"Face"));
			SendMessage(thisPtr->m_listCategories, LB_ADDSTRING, 0, LPARAM(L"Skirt"));
			SendMessage(thisPtr->m_listCategories, LB_ADDSTRING, 0, LPARAM(L"Room"));
			SendMessage(thisPtr->m_listCategories, LB_ADDSTRING, 0, LPARAM(L"Other"));
			SendMessage(thisPtr->m_listCategories, LB_ADDSTRING, 0, LPARAM(L"Props"));

			SendMessage(thisPtr->m_listOperation, LB_ADDSTRING, 0, LPARAM(TEXT("Rotate")));
			SendMessage(thisPtr->m_listOperation, LB_ADDSTRING, 0, LPARAM(TEXT("Translate")));
			SendMessage(thisPtr->m_listOperation, LB_ADDSTRING, 0, LPARAM(TEXT("Scale")));

			SendMessage(thisPtr->m_sliderValueX, TBM_SETRANGEMIN, TRUE, 0);
			SendMessage(thisPtr->m_sliderValueX, TBM_SETRANGEMAX, TRUE, 0x10000);
			SendMessage(thisPtr->m_sliderValueY, TBM_SETRANGEMIN, TRUE, 0);
			SendMessage(thisPtr->m_sliderValueY, TBM_SETRANGEMAX, TRUE, 0x10000);
			SendMessage(thisPtr->m_sliderValueZ, TBM_SETRANGEMIN, TRUE, 0);
			SendMessage(thisPtr->m_sliderValueZ, TBM_SETRANGEMAX, TRUE, 0x10000);
			SendMessage(thisPtr->m_listCategories, LB_SETCURSEL, 0, 0);
			SendMessage(thisPtr->m_listBones, LB_SETCURSEL, 0, 0);
			SendMessage(thisPtr->m_listOperation, LB_SETCURSEL, 0, 0);

			SendMessage(thisPtr->m_spinCharacter, UDM_SETRANGE, 0, MAKELPARAM(1, 0));
			SendMessage(thisPtr->m_spinPose, UDM_SETRANGE, 0, MAKELPARAM(32767, 0));
			SendMessage(thisPtr->m_spinFrame, UDM_SETRANGE, 0, MAKELPARAM(32767, 0));
			SendMessage(thisPtr->m_spinMouth, UDM_SETRANGE, 0, MAKELPARAM(200, 0)); //fix max
			SendMessage(thisPtr->m_spinMouthOpen, UDM_SETRANGE, 0, MAKELPARAM(9, 0));
			SendMessage(thisPtr->m_spinEye, UDM_SETRANGE, 0, MAKELPARAM(200, 0)); //fix max
			SendMessage(thisPtr->m_spinEyeOpen, UDM_SETRANGE, 0, MAKELPARAM(9, 0));
			SendMessage(thisPtr->m_spinEyebrow, UDM_SETRANGE, 0, MAKELPARAM(200, 0)); //fix max
			SendMessage(thisPtr->m_spinBlush, UDM_SETRANGE, 0, MAKELPARAM(12, 0));
			SendMessage(thisPtr->m_spinBlushLines, UDM_SETRANGE, 0, MAKELPARAM(12, 0));

			SendMessage(thisPtr->m_chkShowGuides, BM_SETCHECK, g_PoserController.ShowGuides() ? BST_CHECKED : BST_UNCHECKED, 0);

			TCITEM tab;
#define makeTab(data,text) tab.mask = TCIF_TEXT | TCIF_PARAM; tab.pszText = L#text; tab.lParam = data;
			makeTab(1, x1);
			SendMessage(thisPtr->m_tabModifiers, TCM_INSERTITEM, 0, (LPARAM)(LPTCITEM)&tab);
			makeTab(10, x10);
			SendMessage(thisPtr->m_tabModifiers, TCM_INSERTITEM, 1, (LPARAM)(LPTCITEM)&tab);
			makeTab(100, x100);
			SendMessage(thisPtr->m_tabModifiers, TCM_INSERTITEM, 2, (LPARAM)(LPTCITEM)&tab);
			makeTab(1, Show);
			SendMessage(thisPtr->m_tabShowHide, TCM_INSERTITEM, 0, (LPARAM)(LPTCITEM)&tab);
			makeTab(0, Hide);
			SendMessage(thisPtr->m_tabShowHide, TCM_INSERTITEM, 1, (LPARAM)(LPTCITEM)&tab);
#undef makeTab
			thisPtr->SyncList();

			loc_syncing = false;

			//register hotkeys
			if (g_Config.GetKeyValue(Config::USE_POSER_HOTKEYS).bVal) RegisterHotKey(
				hwndDlg,
				hkTranslate,
				MOD_NOREPEAT,
				hkTranslate) &&
				RegisterHotKey(
					hwndDlg,
					hkRotate,
					MOD_NOREPEAT,
					hkRotate) &&
				RegisterHotKey(
					hwndDlg,
					hkScale,
					MOD_NOREPEAT,
					hkScale);

			return TRUE;
			break; }
		case WM_VSCROLL: {
			PoserWindow* thisPtr = (PoserWindow*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (thisPtr == NULL) return FALSE;
			HWND wnd = (HWND)lparam;
			if (wnd == NULL) {
				General::ScrollWindow(hwndDlg, wparam);
				return TRUE;
			}
			break; }
		case WM_HSCROLL: {
			PoserWindow* thisPtr = (PoserWindow*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (thisPtr == NULL) return FALSE;
			HWND wnd = (HWND)lparam;
			if (wnd == NULL) break; //not slider control, but automatic scroll
			if (!loc_syncing) {
				thisPtr->SyncSlider();
				PoserController::SliderInfo* current = g_PoserController.CurrentSlider();
				if (current)
					current->Apply();
			}
			break; }
		case WM_TIMER: {
			PoserWindow* thisPtr = (PoserWindow*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (thisPtr == NULL) return FALSE;
			PoserController::PoserCharacter* c = nullptr;
			ExtClass::XXFile* skeleton = nullptr;
			c = g_PoserController.CurrentCharacter();
			if (c)
				skeleton = c->m_character->m_xxSkeleton;
			if (skeleton == NULL) return TRUE;

			loc_syncing = true;

			int currPose = General::GetEditInt(thisPtr->m_edPose);
			if (currPose != skeleton->m_poseNumber) {
				std::wstring pose;
				pose = std::to_wstring(skeleton->m_poseNumber);
				SendMessage(thisPtr->m_edPose, WM_SETTEXT, 0, (LPARAM)pose.c_str());
			}

			float currFrame = General::GetEditFloat(thisPtr->m_edFrame);
			if (currFrame != skeleton->m_animFrame) {
				TCHAR frame[16];
				_snwprintf_s(frame, 16, 15, TEXT("%.1f"), skeleton->m_animFrame);
				SendMessage(thisPtr->m_edFrame, WM_SETTEXT, 0, (LPARAM)frame);
			}
			TCHAR value[16];
			StringCbPrintf(value, 15, TEXT("%d"), g_PoserController.CurrentCharacter()->GetFace().GetMouthShape());
			SendMessage(thisPtr->m_edMouth, WM_SETTEXT, 0, (LPARAM)value);
			StringCbPrintf(value, 15, TEXT("%.0f"), g_PoserController.CurrentCharacter()->GetFace().GetMouthOpen());
			SendMessage(thisPtr->m_edMouthOpen, WM_SETTEXT, 0, (LPARAM)value);
			StringCbPrintf(value, 15, TEXT("%d"), g_PoserController.CurrentCharacter()->GetFace().GetEyeShape());
			SendMessage(thisPtr->m_edEye, WM_SETTEXT, 0, (LPARAM)value);
			StringCbPrintf(value, 15, TEXT("%.0f"), g_PoserController.CurrentCharacter()->GetFace().GetEyeOpen());
			SendMessage(thisPtr->m_edEyeOpen, WM_SETTEXT, 0, (LPARAM)value);
			StringCbPrintf(value, 15, TEXT("%d"), g_PoserController.CurrentCharacter()->GetFace().GetEyebrows());
			SendMessage(thisPtr->m_edEyebrow, WM_SETTEXT, 0, (LPARAM)value);
			StringCbPrintf(value, 15, TEXT("%d"), g_PoserController.CurrentCharacter()->GetFace().GetBlush());
			SendMessage(thisPtr->m_edBlush, WM_SETTEXT, 0, (LPARAM)value);
			StringCbPrintf(value, 15, TEXT("%d"), g_PoserController.CurrentCharacter()->GetFace().GetBlushLines());
			SendMessage(thisPtr->m_edBlushLines, WM_SETTEXT, 0, (LPARAM)value);
			SendMessage(thisPtr->m_chkEyeTrack, BM_SETCHECK, g_PoserController.CurrentCharacter()->GetFace().GetEyeTracking() ? BST_CHECKED : BST_UNCHECKED, 0);
			loc_syncing = false;
			break; }
		case WM_COMMAND: {
			PoserWindow* thisPtr = (PoserWindow*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (thisPtr == NULL) return FALSE;
			switch (HIWORD(wparam)) {
			case EN_CHANGE: {
				HWND ed = (HWND)lparam;
				if (LOWORD(wparam) == IDC_PPS_EDPOSE) {
					if (loc_syncing) return false;
					int val = General::GetEditInt(ed);
					ExtClass::XXFile* skeleton = g_PoserController.CurrentCharacter()->m_character->m_xxSkeleton;
					if (skeleton == NULL) return TRUE;
					skeleton->m_poseNumber = val;
					skeleton->m_animFrame = 30000.0f; // "fix" for pose change error
				}
				else if (LOWORD(wparam) == IDC_PPS_EDFRAME) {
					if (loc_syncing) return false;
					float val = General::GetEditFloat(ed);
					ExtClass::XXFile* skeleton = g_PoserController.CurrentCharacter()->m_character->m_xxSkeleton;
					if (skeleton == NULL) return TRUE;
					skeleton->m_animFrame = val;
				}
				else if (LOWORD(wparam) == IDC_PPS_EDVALUEX || LOWORD(wparam) == IDC_PPS_EDVALUEY || LOWORD(wparam) == IDC_PPS_EDVALUEZ) {
					if (!loc_syncing) {
						thisPtr->SyncEdit();
						PoserController::SliderInfo* current = g_PoserController.CurrentSlider();
						if (current)
							current->Apply();
					}
				}
				else if (LOWORD(wparam) == IDC_PPS_EDMOUTH) {
					if (loc_syncing) return false;
					int val = General::GetEditInt(ed);
					g_PoserController.CurrentCharacter()->GetFace().SetMouthShape(val);
				}
				else if (LOWORD(wparam) == IDC_PPS_EDMOUTHOPEN) {
					if (loc_syncing) return false;
					float val = General::GetEditFloat(ed);
					g_PoserController.CurrentCharacter()->GetFace().SetMouthOpen(val);
				}
				else if (LOWORD(wparam) == IDC_PPS_EDEYE) {
					if (loc_syncing) return false;
					int val = General::GetEditInt(ed);
					g_PoserController.CurrentCharacter()->GetFace().SetEyeShape(val);
				}
				else if (LOWORD(wparam) == IDC_PPS_EDEYEOPEN) {
					if (loc_syncing) return false;
					float val = General::GetEditFloat(ed);
					g_PoserController.CurrentCharacter()->GetFace().SetEyeOpen(val);
				}
				else if (LOWORD(wparam) == IDC_PPS_EDEYEBROW) {
					if (loc_syncing) return false;
					int val = General::GetEditInt(ed);
					g_PoserController.CurrentCharacter()->GetFace().SetEyebrows(val);
				}
				else if (LOWORD(wparam) == IDC_PPS_EDBLUSH) {
					if (loc_syncing) return false;
					int val = General::GetEditInt(ed);
					g_PoserController.CurrentCharacter()->GetFace().SetBlush(val);
				}
				else if (LOWORD(wparam) == IDC_PPS_EDBLUSH2) {
					if (loc_syncing) return false;
					int val = General::GetEditInt(ed);
					g_PoserController.CurrentCharacter()->GetFace().SetBlushLines(val);
				}
				else if (LOWORD(wparam) == IDC_PPS_EDCHARACTER) {
					int val = General::GetEditInt(ed);
					g_PoserController.SetCurrentCharacter(val);
					//SendMessage(thisPtr->m_listOperation, LB_SETCURSEL, CurrentSlider()->curOperation, 0);
					//SendMessage(thisPtr->m_listAxis, LB_SETCURSEL, CurrentSlider()->curAxis, 0);
					LRESULT cat = SendMessage(thisPtr->m_listCategories, LB_GETCURSEL, 0, 0);
					LRESULT bon = SendMessage(thisPtr->m_listBones, LB_GETCURSEL, 0, 0);
					if (cat != LB_ERR && bon != LB_ERR) {
						if (!g_PoserController.m_sliderCategories[(PoseMods::FrameCategory)cat].empty()) {
							g_PoserController.SetCurrentSlider((PoseMods::FrameCategory)cat, bon);
						}
					}
					thisPtr->SyncList();
				}

				return TRUE; }
			case BN_CLICKED: {
				PoserWindow* thisPtr = (PoserWindow*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				if (thisPtr == NULL) return FALSE;
				int id = LOWORD(wparam);
				if (id == IDC_PPS_BTNSAVE) {
					const TCHAR* path = General::SaveFileDialog(NULL);
					if (path != NULL) {
						g_PoserController.SavePose(path);
					}
				}
				else if (id == IDC_PPS_BTNLOAD) {
					const TCHAR* path = General::OpenFileDialog(NULL);
					if (path != NULL) {
						g_PoserController.LoadPose(path);
					}
				}
				else if (id == IDC_PPS_BTNRESET) {
					g_PoserController.CurrentCharacter()->ResetSliders();
					thisPtr->SyncList();
				}
				else if (id == IDC_PPS_BTNMODLLX) {
					thisPtr->ApplyIncrement(X, -1);
				}
				else if (id == IDC_PPS_BTNMODLLY) {
					thisPtr->ApplyIncrement(Y, -1);
				}
				else if (id == IDC_PPS_BTNMODLLZ) {
					thisPtr->ApplyIncrement(Z, -1);
				}
				else if (id == IDC_PPS_BTNMODZEROX) {
					thisPtr->ApplyIncrement(X, 0);
				}
				else if (id == IDC_PPS_BTNMODZEROY) {
					thisPtr->ApplyIncrement(Y, 0);
				}
				else if (id == IDC_PPS_BTNMODZEROZ) {
					thisPtr->ApplyIncrement(Z, 0);
				}
				else if (id == IDC_PPS_BTNMODPPX) {
					thisPtr->ApplyIncrement(X, 1);
				}
				else if (id == IDC_PPS_BTNMODPPY) {
					thisPtr->ApplyIncrement(Y, 1);
				}
				else if (id == IDC_PPS_BTNMODPPZ) {
					thisPtr->ApplyIncrement(Z, 1);
				}
				else if (id == IDC_PPS_BTNMODFLIP) {
					if (CurrentSlider()->currentOperation != PoserController::SliderInfo::Scale) {
						CurrentSlider()->setValue(X, -CurrentSlider()->getValue(X));
						thisPtr->SyncList();
						PoserController::SliderInfo* current = g_PoserController.CurrentSlider();
						if (current)
							current->Apply();
					}
				}
				else if (id == IDC_PPS_CHKEYETRACK) {
					LRESULT res = SendMessage(thisPtr->m_chkEyeTrack, BM_GETCHECK, 0, 0);
					g_PoserController.CurrentCharacter()->GetFace().SetEyeTracking(res == BST_CHECKED);
				}
				else if (id == IDC_PPS_CHKTEARS) {
					LRESULT res = SendMessage(thisPtr->m_chkTears, BM_GETCHECK, 0, 0);
					g_PoserController.SetTears(res == BST_CHECKED);
				}
				else if (id == IDC_PPS_CHKTONGUEJUICE) {
					LRESULT res = SendMessage(thisPtr->m_chkTongueJuice, BM_GETCHECK, 0, 0);
					g_PoserController.SetTongueJuice(res == BST_CHECKED);
				}
				else if (id == IDC_PPS_CHKDIMEYES) {
					LRESULT res = SendMessage(thisPtr->m_chkDimEyes, BM_GETCHECK, 0, 0);
					g_PoserController.SetDimEyes(res == BST_CHECKED);
				}
				else if (id == IDC_PPS_BTNCLOTHES) {
					const TCHAR* path = General::SaveFileDialog(General::BuildPlayPath(TEXT("data\\save\\cloth")).c_str());
					if (path != NULL) {
						g_PoserController.LoadCloth(General::FileToBuffer(path));
					}
				}
				else if (id == IDC_PPS_CHKALWAYSONTOP) {
					LRESULT res = SendMessage(thisPtr->m_chkAlwaysOnTop, BM_GETCHECK, 0, 0);
					bool isChecked = res == BST_CHECKED;
					const static DWORD rule[]{ 0x368274 };
					SetParent(thisPtr->m_dialog, isChecked ? *(HWND*)ExtVars::ApplyRule(rule) : NULL);
				}
				else if (id == IDC_PPS_BTNGUIDES) {
					g_PoserController.SetUseGuides(!g_PoserController.IsUseGuidesEnabled());
				}
				else if (id == IDC_PPS_CHKSHOWGUIDES) {
					LRESULT res = SendMessage(thisPtr->m_chkShowGuides, BM_GETCHECK, 0, 0);
					g_PoserController.SetShowGuides((bool)res);
					if (res) {
						PoserController::SliderInfo* s = CurrentSlider();
						if (s && s->guide) {
							g_PoserController.SetHiddenFrame(s->guide, false);
						}
					}
					else {
						for (PoserController::SliderInfo& s : g_PoserController.CurrentCharacter()->m_sliders) {
							if (s.guide) {
								g_PoserController.SetHiddenFrame(s.guide, true);
							}
						}
					}
					bool isChecked = res == BST_CHECKED;
				}
				break; }
			case LBN_SELCHANGE: {
				PoserWindow* thisPtr = (PoserWindow*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				if (thisPtr == NULL) return FALSE;
				int id = LOWORD(wparam);
				switch (id) {
				case(IDC_PPS_LISTCATEGORIES): {
					thisPtr->SyncBones();
					thisPtr->SyncOperation();
					break; }
				case(IDC_PPS_LISTBONES): {
					thisPtr->SyncOperation();
					break; }
				case (IDC_PPS_LISTOP): {
					LRESULT res = SendMessage(thisPtr->m_listOperation, LB_GETCURSEL, 0, 0);
					if (res != LB_ERR) {
						CurrentSlider()->setCurrentOperation(PoserController::SliderInfo::Operation(res));
						thisPtr->SyncList();
					}
					break; }
				}
				break; }
			};
			break; }
		case WM_NOTIFY: {
			PoserWindow* thisPtr = (PoserWindow*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			NMHDR* param = (NMHDR*)(lparam);
			DWORD code = param->code;
			if (code == TCN_SELCHANGE && LOWORD(wparam) == IDC_PPS_TABSHOWHIDE) {
				int idx = TabCtrl_GetCurSel(thisPtr->m_tabShowHide);
				ExtClass::Frame* frame = CurrentSlider()->xxFrame;
				if (frame)
					g_PoserController.SetHiddenFrame(frame, idx != 0);
			}
			break; }
		case WM_HOTKEY: {
			PoserWindow* thisPtr = (PoserWindow*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			SetWindowText(hwndDlg, (LPWSTR)wparam);
			if (thisPtr == NULL) return FALSE;
			int id = wparam;
			if (id == hkTranslate) {
				SendMessage(thisPtr->m_listOperation, LB_SETCURSEL, PoserController::SliderInfo::Translate, 0);
				LRESULT res = SendMessage(thisPtr->m_listOperation, LB_GETCURSEL, 0, 0);
				if (res != LB_ERR) {
					CurrentSlider()->setCurrentOperation(PoserController::SliderInfo::Operation(res));
					thisPtr->SyncList();
				}
			}
			else if (id == hkRotate) {
				SendMessage(thisPtr->m_listOperation, LB_SETCURSEL, PoserController::SliderInfo::Rotate, 0);
				LRESULT res = SendMessage(thisPtr->m_listOperation, LB_GETCURSEL, 0, 0);
				if (res != LB_ERR) {
					CurrentSlider()->setCurrentOperation(PoserController::SliderInfo::Operation(res));
					thisPtr->SyncList();
				}
			}
			else if (id == hkScale) {
				SendMessage(thisPtr->m_listOperation, LB_SETCURSEL, PoserController::SliderInfo::Scale, 0);
				LRESULT res = SendMessage(thisPtr->m_listOperation, LB_GETCURSEL, 0, 0);
				if (res != LB_ERR) {
					CurrentSlider()->setCurrentOperation(PoserController::SliderInfo::Operation(res));
					thisPtr->SyncList();
				}
			}
			break; }
		}

		return FALSE;
	}

	void PoserWindow::ApplyIncrement(int axis, int sign) {
		float increment = 0;
		if (sign != 0) {
			TCITEM tab;
			TCHAR text[10];
			tab.mask = TCIF_PARAM | TCIF_TEXT;
			tab.pszText = text;
			int index = TabCtrl_GetCurSel(m_tabModifiers);
			TabCtrl_GetItem(m_tabModifiers, index, &tab);
			g_PoserController.ApplyIncrement(axis, float(sign * (int)tab.lParam));
		}
		else {
			g_PoserController.ApplyIncrement(axis, 0);
		}
		SyncList();
		g_PoserController.CurrentSlider()->Apply();
	}

	void PoserWindow::SyncBones() {
		LRESULT res = SendMessage(m_listCategories, LB_GETCURSEL, 0, 0);
		if (res != LB_ERR) {
			g_PoserController.SetCurrentCategory((PoseMods::FrameCategory)res);
			if ((PoseMods::FrameCategory)res == PoseMods::FrameCategory::Room) {
				LRESULT cur = SendMessage(m_listBones, LB_GETCURSEL, 0, 0);
				SendMessage(m_listBones, LB_RESETCONTENT, 0, 0);
				for (auto s = g_PoserController.m_roomSliders.begin(); s != g_PoserController.m_roomSliders.end(); s++) {
					SendMessage(this->m_listBones, LB_ADDSTRING, 0, LPARAM(General::CastToWString(s->first).c_str()));
				}
				SendMessage(m_listBones, LB_SETCURSEL, cur, 0);
			}
			else if ((PoseMods::FrameCategory)res == PoseMods::FrameCategory::Prop) {
				LRESULT cur = SendMessage(m_listBones, LB_GETCURSEL, 0, 0);
				SendMessage(m_listBones, LB_RESETCONTENT, 0, 0);
				auto sliders = g_PoserController.CurrentCharacter()->m_propSliders;
				std::string currentProp = g_PoserController.CurrentCharacter()->GetCurrentProp();
				std::wstring propName;
				for (auto s = sliders.cbegin(); s != sliders.cend(); s++) {
					propName = General::CastToWString(s->first).c_str();
					if (currentProp.empty()) {
						currentProp = General::CastToString(propName);
						g_PoserController.CurrentCharacter()->SetCurrentProp(currentProp);
					}
					SendMessage(this->m_listBones, LB_ADDSTRING, 0, LPARAM(propName.c_str()));
				}
				SendMessage(m_listBones, LB_SETCURSEL, cur, 0);
			}
			else {
				SendMessage(m_listBones, LB_RESETCONTENT, 0, 0);
				int idx = 0;
				for (auto s : g_PoserController.m_sliderCategories[(PoseMods::FrameCategory)res]) {
					SendMessage(this->m_listBones, LB_ADDSTRING, 0, LPARAM(g_PoserController.m_sliders.at(s).descr.c_str()));
				}
				SendMessage(m_listBones, LB_SETCURSEL, g_PoserController.CurrentCharacter()->m_CategoryCurrentSlider[PoseMods::FrameCategory(res)], 0);
			}
		}
	}

	void PoserWindow::SyncOperation() {
		LRESULT cat = SendMessage(m_listCategories, LB_GETCURSEL, 0, 0);
		LRESULT res = SendMessage(m_listBones, LB_GETCURSEL, 0, 0);
		if (res != LB_ERR) {
			PoserController::SliderInfo* slider = nullptr;
			if ((PoseMods::FrameCategory)cat == PoseMods::FrameCategory::Room) {
				WCHAR name[255];
				SendMessage(m_listBones, LB_GETTEXT, res, (LPARAM)name);
				g_PoserController.SetCurrentRoomSlider(General::CastToString(std::wstring(name)));
				slider = g_PoserController.CurrentSlider();
				if (slider && !slider->category == PoseMods::Room)
					slider = nullptr;
			}
			else if ((PoseMods::FrameCategory)cat == PoseMods::FrameCategory::Prop) {
				WCHAR name[255];
				SendMessage(m_listBones, LB_GETTEXT, res, (LPARAM)name);
				g_PoserController.CurrentCharacter()->SetCurrentProp(General::CastToString(std::wstring(name)));
				slider = g_PoserController.GetPropSlider(General::CastToString(std::wstring(name)));
			}
			else {
				g_PoserController.SetCurrentSlider((PoseMods::FrameCategory)cat, res);
				slider = CurrentSlider();
			}
			if (slider) {
				SendMessage(m_listOperation, LB_SETCURSEL, slider->currentOperation, 0);
				if (slider->xxFrame) {
					std::string guide = General::CastToString(slider->frameName);
					bool showGuide = g_PoserController.ShowGuides();
					for (PoserController::SliderInfo& s : g_PoserController.CurrentCharacter()->m_sliders) {
						if (s.guide) {
							g_PoserController.SetHiddenFrame(s.guide, !(showGuide && (guide.compare(s.guide->m_name + 6) == 0)));
						}
					}
					TabCtrl_SetCurSel(m_tabShowHide, g_PoserController.GetIsHiddenFrame(slider->xxFrame) ? 1 : 0);
				}
			}
			SyncList();
		}
	}

	void PoserWindow::SyncEdit() {
		if (!g_PoserController.CurrentCharacter()) return;
		loc_syncing = true;
		float valueX = General::GetEditFloat(m_edValueX);
		float valueY = General::GetEditFloat(m_edValueY);
		float valueZ = General::GetEditFloat(m_edValueZ);
		CurrentSlider()->setValue(valueX, valueY, valueZ);
		loc_syncing = false;
	}

	void PoserWindow::SyncList() {
		PoserController::SliderInfo* slider = g_PoserController.CurrentSlider();
		if (!slider) return;
		loc_syncing = true;
		SendMessage(m_sliderValueX, TBM_SETPOS, TRUE, slider->toSlider(X));
		SendMessage(m_sliderValueY, TBM_SETPOS, TRUE, slider->toSlider(Y));
		SendMessage(m_sliderValueZ, TBM_SETPOS, TRUE, slider->toSlider(Z));
		TCHAR number[52];
		_snwprintf_s(number, 52, 16, TEXT("%.3f"), slider->getValue(X));
		SendMessage(m_edValueX, WM_SETTEXT, 0, (LPARAM)number);
		_snwprintf_s(number, 52, 16, TEXT("%.3f"), slider->getValue(Y));
		SendMessage(m_edValueY, WM_SETTEXT, 0, (LPARAM)number);
		_snwprintf_s(number, 52, 16, TEXT("%.3f"), slider->getValue(Z));
		SendMessage(m_edValueZ, WM_SETTEXT, 0, (LPARAM)number);
		loc_syncing = false;
	}

	void PoserWindow::SyncSlider() {
		PoserController::SliderInfo* slider = g_PoserController.CurrentSlider();
		if (!slider) return;
		loc_syncing = true;
		slider->setValueFromSlider(SendMessage(m_sliderValueX, TBM_GETPOS, 0, 0), X);
		slider->setValueFromSlider(SendMessage(m_sliderValueY, TBM_GETPOS, 0, 0), Y);
		slider->setValueFromSlider(SendMessage(m_sliderValueZ, TBM_GETPOS, 0, 0), Z);
		TCHAR number[52];
		float x, y, z;
		if (slider->currentOperation == PoserController::SliderInfo::Rotate) {
			float angle[3];
			slider->rotation.getEulerAngles(angle);
			x = angle[0];
			y = angle[1];
			z = angle[2];
		}
		else {
			x = CurrentSlider()->getValue(X);
			y = CurrentSlider()->getValue(Y);
			z = CurrentSlider()->getValue(Z);
		}
		_snwprintf_s(number, 52, 16, TEXT("%.3f"), x);
		SendMessage(m_edValueX, WM_SETTEXT, 0, (LPARAM)number);
		_snwprintf_s(number, 52, 16, TEXT("%.3f"), y);
		SendMessage(m_edValueY, WM_SETTEXT, 0, (LPARAM)number);
		_snwprintf_s(number, 52, 16, TEXT("%.3f"), z);
		SendMessage(m_edValueZ, WM_SETTEXT, 0, (LPARAM)number);
		loc_syncing = false;
	}

	void FrameModEvent(ExtClass::XXFile* xxFile) {
		g_PoserController.FrameModEvent(xxFile);
	}
}
