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

#include "External\ExternalClasses\Frame.h"
#include "General\IllusionUtil.h"
#include "General\Util.h"
#include "Functions\Shared\Globals.h"
#include "Files\PoseMods.h"
#include "Files\Config.h"
#include "Files\PoseFile.h"
#include "resource.h"
#include "config.h"

#include "3rdparty\picojson\picojson.h"

namespace Poser {

	PoserWindow g_PoserWindow;

	struct FaceInfo {
		FaceInfo() : xxFace(nullptr) {}

		void SetFace(ExtClass::XXFile* face) {
			xxFace = face;
			int base = (int)face;
			mouth = (int*)(base + 0x23C);
			mouthOpen = (float*)(base + 0x2B4);
			eye = (int*)(base + 0x258);
			eyeOpen = (float*)(base + 0x2A8);
			eyebrow = (int*)(base + 0x274);
			void* blushPointer = *((void**)(base + 0x234));
			blushPointer = *((void**)((int)blushPointer + 0x44 * 0x8 + 0x0C));
			blushPointer = *((void**)((int)blushPointer + 0x194));
			blushPointer = *((void**)((int)blushPointer + 0x50));
			blush = (float*)((int)blushPointer + 0x14);
			blushLines = (float*)((int)blushPointer + 0x148);
			eyeTracking = (bool*)(base + 0x1CA20);
		}
		ExtClass::XXFile* xxFace;
		int* mouth;
		int* eye;
		int* eyebrow;
		float* eyeOpen;
		float* mouthOpen;
		float* blush;
		float* blushLines;
		bool* eyeTracking;
	};

	enum Operation {
		Rotate,
		Translate,
		Scale
	};
	
	struct OperationData {
		float x, y, z, min, max;
	};

	struct SliderInfo {
		std::wstring frameName;
		std::wstring descr;
		ExtClass::Frame* xxFrame;
		enum Operation curOperation;
		int curAxis;
		struct OperationData translate, rotate, scale;

		float* curValue() {
			OperationData* data = curOperation == Rotate ? &rotate : curOperation == Translate ? &translate : &scale;
			if (curAxis == 0)
				return &data->x;
			else if (curAxis == 1)
				return &data->y;
			else
				return &data->z;
		}

		void setValue(float newValue) {
			float* value = curValue();
			*value = newValue;
		}

		void fromSlider(int sldVal) {
			OperationData data = curOperation == Rotate ? rotate : curOperation == Translate ? translate : scale;
			float val = data.min + (data.max - data.min) / 0x10000 * sldVal;
			float* value = curValue();
			*value = val;
		}

		int toSlider() {
			OperationData data = curOperation == Rotate ? rotate : curOperation == Translate ? translate : scale;
			float range = data.max - data.min; //map from [min,max] to [0,0x10000]
			float val;
			val = (*curValue()) - data.min;
			float coeff = 0x10000 / range;
			return int(coeff * val);
		}

		void reset() {
			rotate = { 0,0,0,(float)-M_PI,(float)M_PI };
			translate = { 0,0,0,-2,2 };
			scale = { 1,1,1,0,2 };
		}
	};
	std::vector<SliderInfo> loc_sliderInfos;
	std::map<std::string,unsigned int> loc_frameMap;

	class PoserCharacter {
	public:
		PoserCharacter(ExtClass::CharacterStruct* c) :
			Character(c), SliderInfos(loc_sliderInfos), FrameMap(loc_frameMap)
		{
			CurrentSlider = &SliderInfos[0];
		}

		ExtClass::CharacterStruct* Character;
		std::vector<SliderInfo> SliderInfos;
		std::map<std::string, unsigned int> FrameMap;
		FaceInfo FaceInfo;
		SliderInfo *CurrentSlider;
	};

	std::vector<PoserCharacter*> loc_targetCharacters;
	PoserCharacter* loc_targetChar;

	bool loc_syncing;
	Poser::EventType loc_eventType = NoEvent;

	void jsonToPose(PoserCharacter* c, picojson::value json);
	picojson::value poseToJson(PoserCharacter* c);

	void ApplySlider(void* slider = nullptr);
	void GenSliderInfo();

	void StartEvent(EventType type) {
		if (!g_Config.GetKeyValue(Config::USE_POSER_CLOTHES).bVal && type == ClothingScene) return;
		if (!g_Config.GetKeyValue(Config::USE_POSER_DIALOGUE).bVal
			&& (type == NpcInteraction || type == HMode )) return;
		if (loc_eventType && type != loc_eventType) {
			EndEvent();
		}
		GenSliderInfo();
		if (loc_eventType == NoEvent)
			g_PoserWindow.Init();
		loc_eventType = type;
	}

	void EndEvent() {
		for (auto c : loc_targetCharacters) {
			delete c;
		}
		loc_targetCharacters.clear();
		loc_targetChar = nullptr;
		g_PoserWindow.Hide();
		loc_eventType = NoEvent;
	}

	void SetTargetCharacter(ExtClass::CharacterStruct* c) {
		GenSliderInfo();
		if (loc_eventType != ClothingScene && g_Config.GetKeyValue(Config::USE_POSER_DIALOGUE).bVal) {
			StartEvent(HMode);
			PoserCharacter* character = new PoserCharacter(c);
			loc_targetCharacters.push_back(character);
			loc_targetChar = character; // must be set for further meshes loading such as the face
		}
		else if (loc_eventType == ClothingScene && g_Config.GetKeyValue(Config::USE_POSER_CLOTHES).bVal) {
			PoserCharacter* character = new PoserCharacter(c);
			loc_targetCharacters.push_back(character);
			loc_targetChar = character;
		}
	}

	float SliderIncrement(float order, Operation op) {
		return op == Rotate ? ((float)M_PI * order / 180.0f) : op == Translate ? order / 10.0f : order / 50.0f;
	}


	void PoserWindow::Init() {
		if (m_dialog == NULL) {
			CreateDialogParam(General::DllInst,MAKEINTRESOURCE(IDD_PLAY_POSE),
				NULL,DialogProc,(LPARAM)this);
		}
		m_timer = SetTimer(m_dialog,1,2000,NULL);
		ShowWindow(m_dialog,SW_SHOW);
	}
	void PoserWindow::Hide() {
		KillTimer(m_dialog,m_timer);
		m_timer = 0;
		ShowWindow(m_dialog,SW_HIDE);
	}

	INT_PTR CALLBACK PoserWindow::DialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam) {
		static bool ignoreNextSlider = false;

		switch (msg) {
		case WM_INITDIALOG: {
			PoserWindow* thisPtr = (PoserWindow*)lparam;
			SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lparam); //register class to this hwnd
			thisPtr->m_dialog = hwndDlg;
			thisPtr->m_edPose = GetDlgItem(hwndDlg, IDC_PPS_EDPOSE);
			thisPtr->m_edCharacter = GetDlgItem(hwndDlg, IDC_PPS_EDCHARACTER);
			thisPtr->m_edFrame = GetDlgItem(hwndDlg, IDC_PPS_EDFRAME);
			thisPtr->m_edValue = GetDlgItem(hwndDlg, IDC_PPS_EDVALUE);
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
			thisPtr->m_spinValue = GetDlgItem(hwndDlg, IDC_PPS_SPINVALUE);
			thisPtr->m_spinMouth = GetDlgItem(hwndDlg, IDC_PPS_SPINMOUTH);
			thisPtr->m_spinMouthOpen = GetDlgItem(hwndDlg, IDC_PPS_SPINMOUTHOPEN);
			thisPtr->m_spinEye = GetDlgItem(hwndDlg, IDC_PPS_SPINEYE);
			thisPtr->m_spinEyeOpen = GetDlgItem(hwndDlg, IDC_PPS_SPINEYEOPEN);
			thisPtr->m_spinEyebrow = GetDlgItem(hwndDlg, IDC_PPS_SPINEYEBROW);
			thisPtr->m_spinBlush = GetDlgItem(hwndDlg, IDC_PPS_SPINBLUSH);
			thisPtr->m_spinBlushLines = GetDlgItem(hwndDlg, IDC_PPS_SPINBLUSH2);
			thisPtr->m_listBones = GetDlgItem(hwndDlg, IDC_PPS_LISTBONES);
			thisPtr->m_listOperation = GetDlgItem(hwndDlg, IDC_PPS_LISTOP);
			thisPtr->m_listAxis = GetDlgItem(hwndDlg, IDC_PPS_LISTAXIS);
			thisPtr->m_sliderValue = GetDlgItem(hwndDlg, IDC_PPS_SLIDERVALUE);
			thisPtr->m_chkEyeTrack = GetDlgItem(hwndDlg, IDC_PPS_CHKEYETRACK);

			loc_syncing = true;
			SendMessage(thisPtr->m_listOperation, LB_ADDSTRING, 0, LPARAM(TEXT("Rotate")));
			SendMessage(thisPtr->m_listOperation, LB_ADDSTRING, 0, LPARAM(TEXT("Translate")));
			SendMessage(thisPtr->m_listOperation, LB_ADDSTRING, 0, LPARAM(TEXT("Scale")));
			SendMessage(thisPtr->m_listAxis, LB_ADDSTRING, 0, LPARAM(TEXT("X")));
			SendMessage(thisPtr->m_listAxis, LB_ADDSTRING, 0, LPARAM(TEXT("Y")));
			SendMessage(thisPtr->m_listAxis, LB_ADDSTRING, 0, LPARAM(TEXT("Z")));
			thisPtr->InitBones();

			SendMessage(thisPtr->m_sliderValue, TBM_SETRANGEMIN, TRUE, 0);
			SendMessage(thisPtr->m_sliderValue, TBM_SETRANGEMAX, TRUE, 0x10000);
			SendMessage(thisPtr->m_listBones, LB_SETCURSEL, 0, 0);
			SendMessage(thisPtr->m_listOperation, LB_SETCURSEL, 0, 0);
			SendMessage(thisPtr->m_listAxis, LB_SETCURSEL, 0, 0);

			SendMessage(thisPtr->m_spinCharacter, UDM_SETRANGE, 0, MAKELPARAM(1, 0));
			SendMessage(thisPtr->m_spinPose, UDM_SETRANGE, 0, MAKELPARAM(32767, 0));
			SendMessage(thisPtr->m_spinFrame, UDM_SETRANGE, 0, MAKELPARAM(32767, 0));
			SendMessage(thisPtr->m_spinValue, UDM_SETRANGE, 0, MAKELPARAM(-32767, 32767));
			SendMessage(thisPtr->m_spinMouth, UDM_SETRANGE, 0, MAKELPARAM(200, 0)); //fix max
			SendMessage(thisPtr->m_spinMouthOpen, UDM_SETRANGE, 0, MAKELPARAM(9, 0));
			SendMessage(thisPtr->m_spinEye, UDM_SETRANGE, 0, MAKELPARAM(200, 0)); //fix max
			SendMessage(thisPtr->m_spinEyeOpen, UDM_SETRANGE, 0, MAKELPARAM(9, 0));
			SendMessage(thisPtr->m_spinEyebrow, UDM_SETRANGE, 0, MAKELPARAM(200, 0)); //fix max
			SendMessage(thisPtr->m_spinBlush, UDM_SETRANGE, 0, MAKELPARAM(9, 0));
			SendMessage(thisPtr->m_spinBlushLines, UDM_SETRANGE, 0, MAKELPARAM(9, 0));
			thisPtr->SyncList();

			loc_syncing = false;
			return TRUE;
			break; }
		case WM_VSCROLL: {
			PoserWindow* thisPtr = (PoserWindow*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
			if (thisPtr == NULL) return FALSE;
			HWND wnd = (HWND)lparam;
			if (wnd == NULL) {
				General::ScrollWindow(hwndDlg,wparam);
				return TRUE;
			}
			break; }
		case WM_HSCROLL: {
			PoserWindow* thisPtr = (PoserWindow*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
			if (thisPtr == NULL) return FALSE;
			HWND wnd = (HWND)lparam;
			if (wnd == NULL) break; //not slider control, but automatic scroll
			if (!loc_syncing) {
				thisPtr->SyncSlider();
				ApplySlider();
			}
			break; }
		case WM_TIMER: {
			PoserWindow* thisPtr = (PoserWindow*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
			if (thisPtr == NULL) return FALSE;
			if (loc_targetChar == NULL) return TRUE;
			ExtClass::XXFile* skeleton = loc_targetChar->Character->m_xxSkeleton;
			if (skeleton == NULL) return TRUE;

			int currPose = General::GetEditInt(thisPtr->m_edPose);
			if(currPose != skeleton->m_poseNumber) {
				std::wstring pose;
				pose = std::to_wstring(skeleton->m_poseNumber);
				SendMessage(thisPtr->m_edPose,WM_SETTEXT,0,(LPARAM)pose.c_str());
			}
			
			float currFrame = General::GetEditFloat(thisPtr->m_edFrame);
			if(currFrame != skeleton->m_animFrame) {
				TCHAR frame[16];
				_snwprintf_s(frame, 16, 15, TEXT("%.1f"), skeleton->m_animFrame);
				SendMessage(thisPtr->m_edFrame,WM_SETTEXT,0,(LPARAM)frame);
			}
			TCHAR value[16];
			StringCbPrintf(value, 15, TEXT("%d"), *loc_targetChar->FaceInfo.mouth);
			SendMessage(thisPtr->m_edMouth, WM_SETTEXT, 0, (LPARAM)value);
			StringCbPrintf(value, 15, TEXT("%.0f"), *loc_targetChar->FaceInfo.mouthOpen);
			SendMessage(thisPtr->m_edMouthOpen, WM_SETTEXT, 0, (LPARAM)value);
			StringCbPrintf(value, 15, TEXT("%d"), *loc_targetChar->FaceInfo.eye);
			SendMessage(thisPtr->m_edEye, WM_SETTEXT, 0, (LPARAM)value);
			StringCbPrintf(value, 15, TEXT("%.0f"), *loc_targetChar->FaceInfo.eyeOpen);
			SendMessage(thisPtr->m_edEyeOpen, WM_SETTEXT, 0, (LPARAM)value);
			StringCbPrintf(value, 15, TEXT("%d"), *loc_targetChar->FaceInfo.eyebrow);
			SendMessage(thisPtr->m_edEyebrow, WM_SETTEXT, 0, (LPARAM)value);
			StringCbPrintf(value, 15, TEXT("%d"), (int)(*loc_targetChar->FaceInfo.blush * 9.0f));
			SendMessage(thisPtr->m_edBlush, WM_SETTEXT, 0, (LPARAM)value);
			StringCbPrintf(value, 15, TEXT("%d"), (int)(*loc_targetChar->FaceInfo.blushLines * 9.0f));
			SendMessage(thisPtr->m_edBlushLines, WM_SETTEXT, 0, (LPARAM)value);
			SendMessage(thisPtr->m_chkEyeTrack, BM_SETCHECK, *loc_targetChar->FaceInfo.eyeTracking ? BST_CHECKED : BST_UNCHECKED, 0);

			break; }
		case WM_COMMAND: {
			PoserWindow* thisPtr = (PoserWindow*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
			if (thisPtr == NULL) return FALSE;
			switch (HIWORD(wparam)) {
			case EN_CHANGE: {
				HWND ed = (HWND)lparam;
				if (LOWORD(wparam) == IDC_PPS_EDPOSE) {
					int val = General::GetEditInt(ed);
					ExtClass::XXFile* skeleton = loc_targetChar->Character->m_xxSkeleton;
					if (skeleton == NULL) return TRUE;
					skeleton->m_poseNumber = val;
					skeleton->m_animFrame = 30000.0f; // "fix" for pose change error
				}
				else if(LOWORD(wparam) == IDC_PPS_EDFRAME) {
					float val = General::GetEditFloat(ed);
					ExtClass::XXFile* skeleton = loc_targetChar->Character->m_xxSkeleton;
					if (skeleton == NULL) return TRUE;
					skeleton->m_animFrame = val;
				}
				else if (LOWORD(wparam) == IDC_PPS_EDVALUE) {
					if (!loc_syncing) {
						thisPtr->SyncEdit();
						ApplySlider();
					}
				}
				else if (LOWORD(wparam) == IDC_PPS_EDMOUTH) {
					int val = General::GetEditInt(ed);
					*loc_targetChar->FaceInfo.mouth = val;
				}
				else if (LOWORD(wparam) == IDC_PPS_EDMOUTHOPEN) {
					float val = General::GetEditFloat(ed);
					*loc_targetChar->FaceInfo.mouthOpen = val;
				}
				else if (LOWORD(wparam) == IDC_PPS_EDEYE) {
					int val = General::GetEditInt(ed);
					*loc_targetChar->FaceInfo.eye = val;
				}
				else if (LOWORD(wparam) == IDC_PPS_EDEYEOPEN) {
					float val = General::GetEditFloat(ed);
					*loc_targetChar->FaceInfo.eyeOpen = val;
				}
				else if (LOWORD(wparam) == IDC_PPS_EDEYEBROW) {
					int val = General::GetEditInt(ed);
					*loc_targetChar->FaceInfo.eyebrow = val;
				}
				else if (LOWORD(wparam) == IDC_PPS_EDBLUSH) {
					int val = General::GetEditInt(ed);
					*loc_targetChar->FaceInfo.blush = (float)val/9.0f;
				}
				else if (LOWORD(wparam) == IDC_PPS_EDBLUSH2) {
					int val = General::GetEditInt(ed);
					*loc_targetChar->FaceInfo.blushLines = (float)val/9.0f;
				}
				else if (LOWORD(wparam) == IDC_PPS_EDCHARACTER) {
					int val = General::GetEditInt(ed);
					loc_targetChar = loc_targetCharacters[val % loc_targetCharacters.size()];
					SendMessage(thisPtr->m_listOperation, LB_SETCURSEL, loc_targetChar->CurrentSlider->curOperation, 0);
					SendMessage(thisPtr->m_listAxis, LB_SETCURSEL, loc_targetChar->CurrentSlider->curAxis, 0);
					thisPtr->SyncList();
				}

				return TRUE; }
			case BN_CLICKED: {
				PoserWindow* thisPtr = (PoserWindow*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
				if (thisPtr == NULL) return FALSE;
				int id = LOWORD(wparam);
				if(id == IDC_PPS_BTNSAVE) {
					const TCHAR* path = General::SaveFileDialog(NULL);
					if(path != NULL) {
						thisPtr->SavePose(path);
					}
				}
				else if (id == IDC_PPS_BTNLOAD) {
					const TCHAR* path = General::OpenFileDialog(NULL);
					if(path != NULL) {
						thisPtr->LoadPose(path);
					}
				}
				else if (id == IDC_PPS_BTNRESET) {
					for (auto it = loc_sliderInfos.begin(); it != loc_sliderInfos.end(); it++) {
						it->reset();
						ApplySlider(&(*it));
					}
					thisPtr->SyncList();
				}
				else if (id == IDC_PPS_BTNMODLL10) {
					*loc_targetChar->CurrentSlider->curValue() += SliderIncrement(-10.0f, loc_targetChar->CurrentSlider->curOperation);
					thisPtr->SyncList();
					ApplySlider();
				}
				else if (id == IDC_PPS_BTNMODLL1) {
					*loc_targetChar->CurrentSlider->curValue() += SliderIncrement(-1.0f, loc_targetChar->CurrentSlider->curOperation);
					thisPtr->SyncList();
					ApplySlider();
				}
				else if (id == IDC_PPS_BTNMODZERO) {
					*loc_targetChar->CurrentSlider->curValue() = 0.0f;
					thisPtr->SyncList();
					ApplySlider();
				}
				else if (id == IDC_PPS_BTNMODPP1) {
					*loc_targetChar->CurrentSlider->curValue() += SliderIncrement(1.0f, loc_targetChar->CurrentSlider->curOperation);
					thisPtr->SyncList();
					ApplySlider();
				}
				else if (id == IDC_PPS_BTNMODPP10) {
					*loc_targetChar->CurrentSlider->curValue() += SliderIncrement(10.0f, loc_targetChar->CurrentSlider->curOperation);
					thisPtr->SyncList();
					ApplySlider();
				}
				else if (id == IDC_PPS_BTNMODFLIP) {
					if (loc_targetChar->CurrentSlider->curOperation != Scale) {
						*loc_targetChar->CurrentSlider->curValue() *= -1.0f;
						thisPtr->SyncList();
						ApplySlider();
					}
				}
				else if (id == IDC_PPS_CHKEYETRACK) {
					LRESULT res = SendMessage(thisPtr->m_chkEyeTrack, BM_GETCHECK, 0, 0);
					*loc_targetChar->FaceInfo.eyeTracking = res == BST_CHECKED;
				}
				break; }
			case LBN_SELCHANGE: {
				PoserWindow* thisPtr = (PoserWindow*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				if (thisPtr == NULL) return FALSE;
				int id = LOWORD(wparam);
				switch (id) {
				case(IDC_PPS_LISTBONES): {
					LRESULT res = SendMessage(thisPtr->m_listBones, LB_GETCURSEL, 0, 0);
					if (res != LB_ERR) {
						loc_targetChar->CurrentSlider = &loc_targetChar->SliderInfos[res];
						SendMessage(thisPtr->m_listOperation, LB_SETCURSEL, loc_targetChar->CurrentSlider->curOperation, 0);
						SendMessage(thisPtr->m_listAxis, LB_SETCURSEL, loc_targetChar->CurrentSlider->curAxis, 0);
						thisPtr->SyncList();
					}
					break; }
				case (IDC_PPS_LISTOP): {
					LRESULT res = SendMessage(thisPtr->m_listOperation, LB_GETCURSEL, 0, 0);
					if (res != LB_ERR) {
						loc_targetChar->CurrentSlider->curOperation = Operation(res);
						thisPtr->SyncList();
					}
					break; }
				case (IDC_PPS_LISTAXIS): {
					LRESULT res = SendMessage(thisPtr->m_listAxis, LB_GETCURSEL, 0, 0);
					if (res != LB_ERR) {
						loc_targetChar->CurrentSlider->curAxis = res;
						thisPtr->SyncList();
					}
					break; }
				}
				break; }

			};
			break; }
		}

		return FALSE;
	}

	void PoserWindow::SyncEdit() {
		if (!loc_targetChar) return;
		loc_syncing = true;
		float value = General::GetEditFloat(m_edValue);
		loc_targetChar->CurrentSlider->setValue(value);
		SendMessage(m_sliderValue, TBM_SETPOS, TRUE, loc_targetChar->CurrentSlider->toSlider());
		loc_syncing = false;
	}

	void PoserWindow::SyncList() {
		if (!loc_targetChar) return;
		loc_syncing = true;
		SendMessage(m_sliderValue, TBM_SETPOS, TRUE, loc_targetChar->CurrentSlider->toSlider());
		TCHAR number[52];
		_snwprintf_s(number, 52, 16, TEXT("%.3f"), *loc_targetChar->CurrentSlider->curValue());
		SendMessage(m_edValue, WM_SETTEXT, 0, (LPARAM)number);
		loc_syncing = false;
	}

	void PoserWindow::SyncSlider() {
		if (!loc_targetChar) return;
		loc_syncing = true;
		int pos = SendMessage(m_sliderValue, TBM_GETPOS, 0, 0);
		loc_targetChar->CurrentSlider->fromSlider(pos);
		TCHAR number[52];
		_snwprintf_s(number, 52, 16, TEXT("%.3f"), *loc_targetChar->CurrentSlider->curValue());
		SendMessage(m_edValue, WM_SETTEXT, 0, (LPARAM)number);
		loc_syncing = false;
	}

	void ApplySlider(void* slider) {
		SliderInfo* targetSlider = slider == NULL ? loc_targetChar->CurrentSlider : static_cast<SliderInfo*>(slider);
		ExtClass::Frame *frame = targetSlider->xxFrame;
		if(frame) {
			//note that somehow those frame manipulations dont quite work as expected;
			//by observation, rotation happens around the base of the bone whos frame got manipulated,
			//rather than the tip.
			//so to correct that, we gotta translate back

			ExtClass::Frame* origFrame = &frame->m_children[0];

			D3DMATRIX transMatrix;
			(*Shared::D3DXMatrixTranslation)(&transMatrix, targetSlider->translate.x, targetSlider->translate.y, targetSlider->translate.z);
			D3DMATRIX rotMatrix;
			(*Shared::D3DXMatrixRotationYawPitchRoll)(&rotMatrix, targetSlider->rotate.y, targetSlider->rotate.x, targetSlider->rotate.z);
			D3DMATRIX scaleMatrix;
			(*Shared::D3DXMatrixScaling)(&scaleMatrix, targetSlider->scale.x, targetSlider->scale.y, targetSlider->scale.z);
			D3DMATRIX matrix = origFrame->m_matrix5;
			(*Shared::D3DXMatrixMultiply)(&matrix, &matrix, &transMatrix);
			(*Shared::D3DXMatrixMultiply)(&matrix, &matrix, &rotMatrix);
			(*Shared::D3DXMatrixMultiply)(&rotMatrix, &scaleMatrix, &rotMatrix);

			D3DVECTOR3 translations;
			translations.x = origFrame->m_matrix5._41 - matrix._41;
			translations.y = origFrame->m_matrix5._42 - matrix._42;
			translations.z = origFrame->m_matrix5._43 - matrix._43;

			D3DMATRIX resultMatrix = rotMatrix;
			resultMatrix._41 += translations.x;
			resultMatrix._42 += translations.y;
			resultMatrix._43 += translations.z;

			frame->m_matrix1 = resultMatrix;
		}
	}

	void PoserWindow::InitBones() {
		int n = loc_sliderInfos.size();
		for (int i = 0; i < n; i++) {
			SendMessageW(this->m_listBones, LB_ADDSTRING, 0, LPARAM(loc_sliderInfos.at(i).descr.c_str()));
		}
	}



	void FrameModEvent(ExtClass::XXFile* xxFile) {
		using namespace ExtClass;
		static const char prefix[]{ "pose_" };

		if (xxFile == NULL) return;
		if (loc_targetChar == NULL) return;
		if (loc_targetChar->Character->m_xxFace && (loc_targetChar->Character->m_xxFace != loc_targetChar->FaceInfo.xxFace))
			loc_targetChar->FaceInfo.SetFace(loc_targetChar->Character->m_xxFace);
		ExtClass::CharacterStruct::Models model;
		model = General::GetModelFromName(xxFile->m_name);
		if (model != ExtClass::CharacterStruct::SKELETON) return;

		//adjust bone matrizes
		xxFile->EnumBonesPostOrder([&](ExtClass::Frame* bone) {
			
			//try to find matches in both the bone rules and slider rules
			auto match = loc_frameMap.find(bone->m_name);

			//apply matches by adding matrizes
			if (match != loc_frameMap.end()) {
				//make copy of the bone first
				Frame* newMatch = (Frame*)Shared::IllusionMemAlloc(sizeof(Frame));
				memcpy_s(newMatch,sizeof(Frame),bone,sizeof(Frame));

				//turn match into a copy of the root for now, since there are a lot of members i dont know
				memcpy_s(bone,sizeof(Frame),xxFile->m_root,sizeof(Frame));

				//change parent and child stuff
				bone->m_parent = newMatch->m_parent;
				bone->m_nChildren = 1;
				bone->m_children = newMatch;
				newMatch->m_parent = bone;
				for (unsigned int i = 0; i < newMatch->m_nChildren; i++) {
					newMatch->m_children[i].m_parent = newMatch;
				}

				//change name
				int namelength = newMatch->m_nameBufferSize + sizeof(prefix)-1;
				bone->m_name = (char*)Shared::IllusionMemAlloc(namelength);
				bone->m_nameBufferSize = namelength;
				strcpy_s(bone->m_name,bone->m_nameBufferSize,prefix);
				strcat_s(bone->m_name,bone->m_nameBufferSize,newMatch->m_name);

				loc_targetChar->SliderInfos[match->second].xxFrame = bone;
			}
		});



		//now, frames that represent a mesh have a bunch of bones; each bone has a pointer to its frame (more precisely,
		//its frames matrix2), which it uses to position its mesh. after this, those pointers will point to the artificial matrizes,
		//so we have to change that as well
		xxFile->EnumBonesPostOrder([&](ExtClass::Frame* frame) {
			for (unsigned int i = 0; i < frame->m_nBones; i++) {
				Bone* bone = &frame->m_bones[i];
				Frame* boneFrame = bone->GetFrame();
				if (boneFrame != NULL && strncmp(boneFrame->m_name,prefix,sizeof(prefix)-1) == 0) {
					bone->SetFrame(&boneFrame->m_children[0]);
				}
			}
		});
	}

	void GenSliderInfo() {
		if (!loc_sliderInfos.empty()) return;
		PoseMods mods(POSEMOD_FILE_PATH);
		auto& input = mods.GetInput();
		for(auto& elem : input) {
			SliderInfo info;
			std::string& strFrame = std::get<0>(elem);
			std::string& strDesc = std::get<1>(elem);

			std::wstring wstrFrame(strFrame.begin(), strFrame.end());
			info.frameName = wstrFrame;
			std::wstring wstrDescr(strDesc.begin(),strDesc.end());
			info.descr = wstrDescr;
			
			info.reset();

			info.curAxis = 0;
			info.curOperation = Rotate;

			info.xxFrame = NULL;

			loc_sliderInfos.push_back(info);
			loc_frameMap.insert(std::make_pair(strFrame, loc_sliderInfos.size() - 1));
		}
	}

	void PoserWindow::LoadPose(const TCHAR* path) {
		using namespace picojson;
		value json;

		for (SliderInfo& slider : loc_sliderInfos) {
			slider.reset();
			ApplySlider(&slider);
		}

		std::ifstream in(path);
		in >> json;

		if (picojson::get_last_error().empty()) {
			jsonToPose(loc_targetChar, json);
		}
		else {
			PoseFile openFile(path);
			std::wstring str;
			str = std::to_wstring(openFile.GetPose());
			SendMessage(m_edPose, WM_SETTEXT, 0, (LPARAM)str.c_str());
			str = std::to_wstring(openFile.GetFrame());
			SendMessage(m_edFrame, WM_SETTEXT, 0, (LPARAM)str.c_str());
			SliderInfo* slider = NULL;
			for (auto elem : openFile.GetMods()) {
				auto match = loc_targetChar->FrameMap.find(elem.frameName);
				if (match != loc_targetChar->FrameMap.end()) {
					slider = &loc_targetChar->SliderInfos[match->second];
					slider->rotate.x = elem.matrix[0];
					slider->rotate.y = elem.matrix[1];
					slider->rotate.z = elem.matrix[2];
					slider->translate.x = elem.matrix[3];
					slider->translate.y = elem.matrix[4];
					slider->translate.z = elem.matrix[5];
					slider->scale.x = elem.matrix[6];
					slider->scale.y = elem.matrix[7];
					slider->scale.z = elem.matrix[8];
					ApplySlider(slider);
				}
			}
		}
		SyncList();
	}

	void PoserWindow::SavePose(const TCHAR* path) {
		std::ofstream out(path);
		out << poseToJson(loc_targetChar).serialize(true);
	}

	void jsonToPose(PoserCharacter* c, picojson::value json) {
		using namespace picojson;
		
		if (json.is<object>()) {
			const object load = json.get<object>();
			SliderInfo* slider;
			try {
				c->Character->m_xxSkeleton->m_poseNumber = (int)load.at("pose").get<double>();
				c->Character->m_xxSkeleton->m_animFrame = (float)load.at("frame").get<double>();
				object sliders = load.at("sliders").get<object>();
				for (auto s = sliders.cbegin(); s != sliders.cend(); s++) {
					auto match = c->FrameMap.find((*s).first);
					if (match != loc_targetChar->FrameMap.end()) {
						array mods = (*s).second.get<array>();
						if (mods.size() == 9) {
							slider = &c->SliderInfos[match->second];
							slider->rotate.x = (float)mods[0].get<double>();
							slider->rotate.y = (float)mods[1].get<double>();
							slider->rotate.z = (float)mods[2].get<double>();
							slider->translate.x = (float)mods[3].get<double>();
							slider->translate.y = (float)mods[4].get<double>();
							slider->translate.z = (float)mods[5].get<double>();
							slider->scale.x = (float)mods[6].get<double>();
							slider->scale.y = (float)mods[7].get<double>();
							slider->scale.z = (float)mods[8].get<double>();
							ApplySlider(slider);
						}
						else {
							//invalid json data
						}
					}
				}
				object face = load.at("face").get<object>();
				*c->FaceInfo.mouth = (int)face.at("mouth").get<double>();
				*c->FaceInfo.mouthOpen = (float)face.at("mouthopen").get<double>();
				*c->FaceInfo.eye = (int)face.at("eye").get<double>();
				*c->FaceInfo.eyeOpen = (float)face.at("eyeopen").get<double>();
				*c->FaceInfo.eyebrow = (int)face.at("eyebrow").get<double>();
				*c->FaceInfo.blush = (float)face.at("blush").get<double>();
				*c->FaceInfo.blushLines = (float)face.at("blushlines").get<double>();
			}
			catch (std::out_of_range& e) {
				//key doesn't exist
			}
			catch (std::runtime_error& e) {
				//invalid json data
			}
		}

	}

	picojson::value poseToJson(PoserCharacter* c) {
		using namespace picojson;
		object json;
		json["pose"] = value((double)c->Character->m_xxSkeleton->m_poseNumber);
		json["frame"] = value((double)c->Character->m_xxSkeleton->m_animFrame);
		value::object sliders;
		for (SliderInfo& slider : c->SliderInfos) {
			value::array values(9);
			values[0] = value((double)slider.rotate.x);
			values[1] = value((double)slider.rotate.y);
			values[2] = value((double)slider.rotate.z);
			values[3] = value((double)slider.translate.x);
			values[4] = value((double)slider.translate.y);
			values[5] = value((double)slider.translate.z);
			values[6] = value((double)slider.scale.x);
			values[7] = value((double)slider.scale.y);
			values[8] = value((double)slider.scale.z);
			sliders[std::string(slider.frameName.cbegin(), slider.frameName.cend())] = value(values);
		}
		json["sliders"] = value(sliders);

		value::object face;
		face["eye"] = value((double)*c->FaceInfo.eye);
		face["eyeopen"] = value((double)*c->FaceInfo.eyeOpen);
		face["eyebrow"] = value((double)*c->FaceInfo.eyebrow);
		face["mouth"] = value((double)*c->FaceInfo.mouth);
		face["mouthopen"] = value((double)*c->FaceInfo.mouthOpen);
		face["blush"] = value((double)*c->FaceInfo.blush);
		face["blushlines"] = value((double)*c->FaceInfo.blushLines);
		json["face"] = value(face);
		return value(json);
	}
}
