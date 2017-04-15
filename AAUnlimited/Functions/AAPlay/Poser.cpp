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
#include "Files\PoseFile.h"
#include "Files\ClothFile.h"
#include "resource.h"
#include "config.h"

#include "3rdparty\picojson\picojson.h"

namespace Poser {

#define X 0
#define Y 1
#define Z 2
#define W 3

	const D3DXVECTOR3 g_unitVec[3] = { 1, 0, 0,
	                                   0, 1, 0,
	                                   0, 0, 1};

	PoserWindow g_PoserWindow;

	bool GetIsHiddenFrame(ExtClass::Frame* frame);
	void SetHiddenFrame(ExtClass::Frame* frame, bool hidden);

	enum Operation {
		Rotate,
		Translate,
		Scale
	};
	
	struct SliderInfo {
		std::wstring frameName;
		std::wstring descr;
		PoseMods::FrameCategory category;
		ExtClass::Frame* xxFrame;
		Operation currentOperation;
		struct TranslationScaleData {
			float rangeMin(int axis) {
				return min[axis] + delta[axis];
			}

			float rangeMax(int axis) {
				return max[axis] + delta[axis];
			}

			float value[3];
			float min[3];
			float max[3];
			float delta[3];
		} translate, scale;
		struct RotationData {
			D3DXQUATERNION quaternion;
			float minEuler[3];
			float maxEuler[3];
			inline void reset() {
				Shared::D3DXQuaternionIdentity(&quaternion);
				for (int i = 0; i < 3; i++) {
					rotAxes[i].set(g_unitVec[i]);
				}
			}

			struct RotationAxis {
				D3DXVECTOR3 vector;

				D3DXQUATERNION quaternion() {
					//D3DXQUATERNION q = { vector.x, vector.y, vector.z, 0 };
					return{ vector.x, vector.y, vector.z, 0 };
					//return q;
				}

				void set(const D3DXVECTOR3& v) {
					vector = v;
				}

				void set(const D3DXQUATERNION& q) {
					vector.x = q.x;
					vector.y = q.y;
					vector.z = q.z;
				}
			} rotAxes[3];

		} rotation;
		
		inline D3DXQUATERNION& getRotation() {
			return rotation.quaternion;
		}

		void setCurrentOperation(Operation operation) {
			currentOperation = operation;
		}

		inline TranslationScaleData* getCurrentOperation() {
			return currentOperation == Translate ? &translate : &scale;
		}

		float getCurrentOperationRangeMin(int axis) {
			if (currentOperation == Rotate)
				return rotation.minEuler[axis];
			else
				return getCurrentOperation()->rangeMin(axis);
		}

		float getCurrentOperationRange(int axis) {
				if (currentOperation == Rotate)
				return rotation.maxEuler[axis] - rotation.minEuler[axis];
			else
				return getCurrentOperation()->rangeMax(axis) - getCurrentOperation()->rangeMin(axis); //map from [min,max] to [0,0x10000]
		}

		float getValue(int axis) {
			if (currentOperation == Rotate) {
				float angle[3];
				EulerAnglesQuaternion(&getRotation(), angle);
				return angle[axis];
			}
			return getCurrentOperation()->value[axis];
		}

		void getEulerAngles(float *angle) {
			EulerAnglesQuaternion(&getRotation(), angle);
		}

		void setValue(int axis, float value) {
			if (currentOperation == Rotate) {
				float angle[3];
				getEulerAngles(angle);
				angle[axis] = value;
				(*Shared::D3DXQuaternionRotationYawPitchRoll)(&getRotation(), angle[1], angle[0], angle[2]);
			}
			else {
				getCurrentOperation()->value[axis] = value;
			}
		}

		void setValue(float newValueX, float newValueY, float newValueZ) {
			if (currentOperation == Rotate) {
				(*Shared::D3DXQuaternionRotationYawPitchRoll)(&getRotation(), newValueY, newValueX, newValueZ);
			}
			else {
				setValue(X, newValueX);
				setValue(Y, newValueY);
				setValue(Z, newValueZ);
			}
		}

		float fromSlider(int sldVal, int axis) {
			if (currentOperation == Rotate)
				return rotation.minEuler[axis] + (rotation.maxEuler[axis] - rotation.minEuler[axis]) / 0x10000 * sldVal;
			else
				return getCurrentOperation()->rangeMin(axis) + (getCurrentOperation()->rangeMax(axis) - getCurrentOperation()->rangeMin(axis)) / 0x10000 * sldVal;
		}

		void fromSliderX(int sldVal) {
			setValue(X, fromSlider(sldVal, X));
		}

		void fromSliderY(int sldVal) {
			setValue(Y, fromSlider(sldVal, Y));
		}

		void fromSliderZ(int sldVal) {
			setValue(Z, fromSlider(sldVal, Z));
		}

		int toSlider(float value, int axis) {
			float coeff = 0x10000 / getCurrentOperationRange(axis);
			return int(coeff * (value - getCurrentOperationRangeMin(axis)));
		}

		int toSliderX() {
			return toSlider(getValue(X), X);
		}

		int toSliderY() {
			return toSlider(getValue(Y), Y);
		}

		int toSliderZ() {
			return toSlider(getValue(Z), Z);
		}

		void increment(float order, int axis) {
			float delta;
			if (currentOperation == Rotate) {
				if (order) {
					delta = (float)M_PI * order / 180.0f;
					D3DXQUATERNION deltaRotation, deltaRotationConjugate;
					(*Shared::D3DXQuaternionRotationAxis)(&deltaRotation, &rotation.rotAxes[axis].vector, delta);
					(*Shared::D3DXQuaternionConjugate)(&deltaRotationConjugate, &deltaRotation);
					(*Shared::D3DXQuaternionMultiply)(&getRotation(), &getRotation(), &deltaRotation);
					for (int i = 0; i < 3; i++) {
						if (i != axis) {
							D3DXQUATERNION temp = rotation.rotAxes[i].quaternion();
							(*Shared::D3DXQuaternionMultiply)(&temp, &deltaRotationConjugate, &temp);
							(*Shared::D3DXQuaternionMultiply)(&temp, &temp, &deltaRotation);
							rotation.rotAxes[i].set(temp);
						}
					}
				}
				else {
					rotation.reset();
				}
			}
			else {
				if (order) {
					delta = currentOperation == Translate ? order / 10.0f : order / 50.0f;
					getCurrentOperation()->value[axis] += delta;
					getCurrentOperation()->delta[axis] += delta;
				}
				else {
					getCurrentOperation()->value[axis] = currentOperation == Scale ? 1.0f : 0.0f;
					getCurrentOperation()->delta[axis] = 0;
				}
			}
		}

		void translateSlider(float delta, int axis) {
			getCurrentOperation()->delta[axis] = delta;
		}

		void reset() {
			for (int i = 0; i < 3; i++) {
				rotation.reset();
				rotation.minEuler[i] = (float)-M_PI;
				rotation.maxEuler[i] = (float)M_PI;
				translate.value[i] = 0;
				translate.min[i] = -2;
				translate.max[i] = 2;
				translate.delta[i] = 0;
				scale.value[i] = 1;
				scale.min[i] = 0;
				scale.max[i] = 2;
				scale.delta[i] = 0;
			}
		}

		inline void EulerAnglesQuaternion(D3DXQUATERNION* pQ, FLOAT* angle) {
			float yy = pQ->y * pQ->y;

			float t0 = 2.0f * (pQ->w * pQ->x + pQ->y * pQ->z);
			float t1 = 1.0f - 2.0f * (pQ->x * pQ->x + yy);
			angle[0] = std::atan2(t0, t1);

			float t2 = 2.0f * (pQ->w * pQ->y - pQ->z * pQ->x);
			t2 = t2 > 1.0f ? 1.0f : t2;
			t2 = t2 < -1.0f ? -1.0f : t2;
			angle[1] = std::asin(t2);

			float t3 = 2.0f * (pQ->w * pQ->z + pQ->x * pQ->y);
			float t4 = 1.0f - 2.0f * (yy + pQ->z * pQ->z);
			angle[2] = std::atan2(t3, t4);
		}

	};
	std::map<PoseMods::FrameCategory,std::vector<unsigned int>> loc_sliderCategories; // holds the index of each slider in the main slider vector
																 // according the the category they reside into
																 // loc_sliderCategories[category][categoryIndex] = index
	std::vector<SliderInfo> loc_sliderInfos;
	std::map<std::string,unsigned int> loc_frameMap;
	std::map<std::string, SliderInfo> loc_sliderInfosRoom;
	std::vector<std::map<std::string, SliderInfo>> loc_sliderInfosProps;

	class PoserCharacter {
	public:
		PoserCharacter(ExtClass::CharacterStruct* c) :
			Character(c), SliderInfos(loc_sliderInfos), FrameMap(loc_frameMap)
		{
			CurrentSlider = &SliderInfos[0];
		}

		inline XXFileFace* GetFace() {
			return reinterpret_cast<XXFileFace*>(Character->m_xxFace);
		}

		void SetHidden(const char* name, bool hidden) {
			ExtClass::Frame** frame = Character->m_bonePtrArray;
			while (frame < Character->m_bonePtrArrayEnd) {
				if (*frame != nullptr) {
					if (strstr((*frame)->m_name, name)) {
						SetHiddenFrame(*frame, hidden);
					}
				}
				frame++;
			}
		}

		void SetTears(bool show) {
			SetHidden("A00_O_namida", !show);
		}

		void SetDimEyes(bool dim) {
			SetHidden("A00_O_mehi", dim);
		}

		void SetTongueJuice(bool show) {
			SetHidden("A00_O_kutisiru", !show);
			SetHidden("A00_O_sitasiru", !show);
		}

		int id;
		ExtClass::CharacterStruct* Character;
		std::vector<SliderInfo> SliderInfos;
		std::map<std::string, unsigned int> FrameMap;
		SliderInfo *CurrentSlider;
		std::map<PoseMods::FrameCategory, int> CategoryCurrentSlider;
	};

	std::vector<PoserCharacter*> loc_targetCharacters;
	PoserCharacter* loc_targetChar;
	PoserCharacter* loc_loadCharacter;

	bool loc_syncing;
	Poser::EventType loc_eventType = NoEvent;

	void jsonToPose(PoserCharacter* c, picojson::value json);
	picojson::value poseToJson(PoserCharacter* c);

	void ApplySlider(void* slider = nullptr);
	void GenSliderInfo();
	PoseMods::FrameCategory loc_currentCategory;
	std::string loc_currentRoomFrame;
	std::vector<std::string> loc_currentPropsFrame;
	SliderInfo* CurrentSlider() {
		if (loc_currentCategory == PoseMods::FrameCategory::Room) {
			return &loc_sliderInfosRoom[loc_currentRoomFrame];
		}
		else if (loc_currentCategory == PoseMods::FrameCategory::Prop) {
			return &loc_sliderInfosProps[loc_targetChar->id][loc_currentPropsFrame[loc_targetChar->id]];
		}
		else {
			return loc_targetChar->CurrentSlider;
		}
	}

	bool GetIsHiddenFrame(ExtClass::Frame* frame) {
		return frame->m_renderFlag == 2;
	}

	void SetHiddenFrame(ExtClass::Frame* frame, bool hidden) {
		frame->m_renderFlag = hidden ? 2 : 0;
	}

	void StartEvent(EventType type) {
		if (type == loc_eventType) return;
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
		loc_loadCharacter = nullptr;
		loc_sliderInfosRoom.clear();
		loc_sliderInfosProps.clear();
		loc_currentPropsFrame.clear();
		g_PoserWindow.Hide();
		loc_eventType = NoEvent;
	}

	void SetTargetCharacter(ExtClass::CharacterStruct* charStruct) {
		GenSliderInfo();
		if (loc_eventType != ClothingScene && g_Config.GetKeyValue(Config::USE_POSER_DIALOGUE).bVal) {
			if (loc_eventType != NpcInteraction)
				StartEvent(HMode);
			loc_loadCharacter = nullptr;
			for (PoserCharacter* c : loc_targetCharacters) {
				if (c->Character == charStruct)
					loc_loadCharacter = c;
			}
			if (!loc_loadCharacter) {
				loc_loadCharacter = new PoserCharacter(charStruct);
				loc_loadCharacter->id = loc_targetCharacters.size();
				loc_targetCharacters.push_back(loc_loadCharacter);
				loc_sliderInfosProps.resize(loc_targetCharacters.size());
				loc_currentPropsFrame.resize(loc_targetCharacters.size());
				g_PoserWindow.NewCharacter(loc_targetCharacters.size() - 1);
			}
			if (!loc_targetChar)
				loc_targetChar = loc_loadCharacter;
		}
		else if (loc_eventType == ClothingScene && g_Config.GetKeyValue(Config::USE_POSER_CLOTHES).bVal) {
			PoserCharacter* character = new PoserCharacter(charStruct);
			loc_targetCharacters.push_back(character);
			loc_targetChar = character;
			loc_loadCharacter = character;
			loc_loadCharacter->id = 0;
			loc_sliderInfosProps.resize(loc_targetCharacters.size());
			loc_currentPropsFrame.resize(loc_targetCharacters.size());
			g_PoserWindow.NewCharacter(0);
		}
	}

	void PoserWindow::Init() {
		if (m_dialog == NULL) {
			CreateDialogParam(General::DllInst,MAKEINTRESOURCE(IDD_PLAY_POSE),
				NULL,DialogProc,(LPARAM)this);
		}
		m_timer = SetTimer(m_dialog,1,2000,NULL);
		EnableWindow(g_PoserWindow.m_dialog, TRUE);
		ShowWindow(m_dialog,SW_SHOW);
	}
	void PoserWindow::Hide() {
		KillTimer(m_dialog,m_timer);
		m_timer = 0;
		EnableWindow(g_PoserWindow.m_dialog, FALSE);
		if(SendMessage(g_PoserWindow.m_chkAlwaysOnTop, BM_GETCHECK, 0, 0) == BST_CHECKED)
			ShowWindow(m_dialog,SW_HIDE);
	}

	INT_PTR CALLBACK PoserWindow::DialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam) {
		static bool ignoreNextSlider = false;

		//set hotkeys		
		auto hkTranslate = g_Config.GetKeyValue(Config::HKEY_POSER_TRANSLATE).bVal;	//W
		auto hkRotate = g_Config.GetKeyValue(Config::HKEY_POSER_ROTATE).bVal;	//E
		auto hkScale = g_Config.GetKeyValue(Config::HKEY_POSER_SCALE).bVal;	//R

		switch (msg) {
		case WM_INITDIALOG: {
			PoserWindow* thisPtr = (PoserWindow*)lparam;
			SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lparam); //register class to this hwnd
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
			StringCbPrintf(value, 15, TEXT("%d"), loc_targetChar->GetFace()->m_mouth);
			SendMessage(thisPtr->m_edMouth, WM_SETTEXT, 0, (LPARAM)value);
			StringCbPrintf(value, 15, TEXT("%.0f"), loc_targetChar->GetFace()->m_mouthOpen);
			SendMessage(thisPtr->m_edMouthOpen, WM_SETTEXT, 0, (LPARAM)value);
			StringCbPrintf(value, 15, TEXT("%d"), loc_targetChar->GetFace()->m_eye);
			SendMessage(thisPtr->m_edEye, WM_SETTEXT, 0, (LPARAM)value);
			StringCbPrintf(value, 15, TEXT("%.0f"), loc_targetChar->GetFace()->m_eyeOpen);
			SendMessage(thisPtr->m_edEyeOpen, WM_SETTEXT, 0, (LPARAM)value);
			StringCbPrintf(value, 15, TEXT("%d"), loc_targetChar->GetFace()->m_eyebrow);
			SendMessage(thisPtr->m_edEyebrow, WM_SETTEXT, 0, (LPARAM)value);
			StringCbPrintf(value, 15, TEXT("%d"), (int)(*loc_targetChar->GetFace()->GetBlush() * 9.0f));
			SendMessage(thisPtr->m_edBlush, WM_SETTEXT, 0, (LPARAM)value);
			StringCbPrintf(value, 15, TEXT("%d"), (int)(*loc_targetChar->GetFace()->GetBlushLines() * 9.0f));
			SendMessage(thisPtr->m_edBlushLines, WM_SETTEXT, 0, (LPARAM)value);
			SendMessage(thisPtr->m_chkEyeTrack, BM_SETCHECK, loc_targetChar->GetFace()->m_eyeTracking ? BST_CHECKED : BST_UNCHECKED, 0);

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
				else if (LOWORD(wparam) == IDC_PPS_EDVALUEX || LOWORD(wparam) == IDC_PPS_EDVALUEY || LOWORD(wparam) == IDC_PPS_EDVALUEZ) {
					if (!loc_syncing) {
						thisPtr->SyncEdit();
						ApplySlider();
					}
				}
				else if (LOWORD(wparam) == IDC_PPS_EDMOUTH) {
					int val = General::GetEditInt(ed);
					loc_targetChar->GetFace()->m_mouth = val;
				}
				else if (LOWORD(wparam) == IDC_PPS_EDMOUTHOPEN) {
					float val = General::GetEditFloat(ed);
					loc_targetChar->GetFace()->m_mouthOpen = val;
				}
				else if (LOWORD(wparam) == IDC_PPS_EDEYE) {
					int val = General::GetEditInt(ed);
					loc_targetChar->GetFace()->m_eye = val;
				}
				else if (LOWORD(wparam) == IDC_PPS_EDEYEOPEN) {
					float val = General::GetEditFloat(ed);
					loc_targetChar->GetFace()->m_eyeOpen = val;
				}
				else if (LOWORD(wparam) == IDC_PPS_EDEYEBROW) {
					int val = General::GetEditInt(ed);
					loc_targetChar->GetFace()->m_eyebrow = val;
				}
				else if (LOWORD(wparam) == IDC_PPS_EDBLUSH) {
					int val = General::GetEditInt(ed);
					*loc_targetChar->GetFace()->GetBlush() = (float)val/9.0f;
				}
				else if (LOWORD(wparam) == IDC_PPS_EDBLUSH2) {
					int val = General::GetEditInt(ed);
					*loc_targetChar->GetFace()->GetBlushLines() = (float)val/9.0f;
				}
				else if (LOWORD(wparam) == IDC_PPS_EDCHARACTER) {
					int val = General::GetEditInt(ed);
					loc_targetChar = loc_targetCharacters[val % loc_targetCharacters.size()];
					//SendMessage(thisPtr->m_listOperation, LB_SETCURSEL, CurrentSlider()->curOperation, 0);
					//SendMessage(thisPtr->m_listAxis, LB_SETCURSEL, CurrentSlider()->curAxis, 0);
					LRESULT cat = SendMessage(thisPtr->m_listCategories, LB_GETCURSEL, 0, 0);
					LRESULT bon = SendMessage(thisPtr->m_listBones, LB_GETCURSEL, 0, 0);
					if (cat != LB_ERR && bon != LB_ERR) {
						if (!loc_sliderCategories[(PoseMods::FrameCategory)cat].empty()) {
							loc_targetChar->CurrentSlider = &loc_targetChar->SliderInfos[loc_sliderCategories[(PoseMods::FrameCategory)cat][bon]];
						}
					}
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
					for (auto it = loc_targetChar->SliderInfos.begin(); it != loc_targetChar->SliderInfos.end(); it++) {
						it->reset();
						ApplySlider(&(*it));
					}
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
					if (CurrentSlider()->currentOperation != Scale) {
						CurrentSlider()->setValue(X, -CurrentSlider()->getValue(X));
						thisPtr->SyncList();
						ApplySlider();
					}
				}
				else if (id == IDC_PPS_CHKEYETRACK) {
					LRESULT res = SendMessage(thisPtr->m_chkEyeTrack, BM_GETCHECK, 0, 0);
					loc_targetChar->GetFace()->m_eyeTracking = res == BST_CHECKED;
				}
				else if (id == IDC_PPS_CHKTEARS) {
					LRESULT res = SendMessage(thisPtr->m_chkTears, BM_GETCHECK, 0, 0);
					loc_targetChar->SetTears(res == BST_CHECKED);
				}
				else if (id == IDC_PPS_CHKTONGUEJUICE) {
					LRESULT res = SendMessage(thisPtr->m_chkTongueJuice, BM_GETCHECK, 0, 0);
					loc_targetChar->SetTongueJuice(res == BST_CHECKED);
				}
				else if (id == IDC_PPS_CHKDIMEYES) {
					LRESULT res = SendMessage(thisPtr->m_chkDimEyes, BM_GETCHECK, 0, 0);
					loc_targetChar->SetDimEyes(res == BST_CHECKED);
				}
				else if (id == IDC_PPS_BTNCLOTHES) {
					const TCHAR* path = General::SaveFileDialog(General::BuildPlayPath(TEXT("data\\save\\cloth")).c_str());
					if (path != NULL) {
						thisPtr->LoadCloth(General::FileToBuffer(path));
					}
				}
				else if (id == IDC_PPS_CHKALWAYSONTOP) {
					LRESULT res = SendMessage(thisPtr->m_chkAlwaysOnTop, BM_GETCHECK, 0, 0);
					bool isChecked = res == BST_CHECKED;
					const static DWORD rule[]{ 0x368274 };
					SetParent(thisPtr->m_dialog, isChecked ? *(HWND*)ExtVars::ApplyRule(rule) : NULL);
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
						CurrentSlider()->setCurrentOperation(Operation(res));
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
					SetHiddenFrame(frame, idx != 0);
			}
			break; }
		case WM_HOTKEY: {
			PoserWindow* thisPtr = (PoserWindow*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			SetWindowText(hwndDlg, (LPWSTR)wparam);
			if (thisPtr == NULL) return FALSE;
			int id = wparam;
			if (id == hkTranslate) {
				SendMessage(thisPtr->m_listOperation, LB_SETCURSEL, Operation::Translate, 0);
				LRESULT res = SendMessage(thisPtr->m_listOperation, LB_GETCURSEL, 0, 0);
				if (res != LB_ERR) {
					CurrentSlider()->setCurrentOperation(Operation(res));
					thisPtr->SyncList();
				}
			}
			else if (id == hkRotate) {
				SendMessage(thisPtr->m_listOperation, LB_SETCURSEL, Operation::Rotate, 0);
				LRESULT res = SendMessage(thisPtr->m_listOperation, LB_GETCURSEL, 0, 0);
				if (res != LB_ERR) {
					CurrentSlider()->setCurrentOperation(Operation(res));
					thisPtr->SyncList();
				}
			}
			else if (id == hkScale) {
				SendMessage(thisPtr->m_listOperation, LB_SETCURSEL, Operation::Scale, 0);
				LRESULT res = SendMessage(thisPtr->m_listOperation, LB_GETCURSEL, 0, 0);
				if (res != LB_ERR) {
					CurrentSlider()->setCurrentOperation(Operation(res));
					thisPtr->SyncList();
				}
			}
			break; }
		}

		return FALSE;
	}

	void PoserWindow::NewCharacter(int index) {
		if (loc_sliderCategories[PoseMods::Other].size()) {
			PoserCharacter* c = loc_targetCharacters.at(index);
			c->CurrentSlider = &c->SliderInfos.at(loc_sliderCategories[PoseMods::Other].front());
			if (c == loc_targetChar) {
				SendMessage(m_listCategories, LB_SETCURSEL, PoseMods::Other, 0);
				SyncBones();
				SendMessage(m_listBones, LB_SETCURSEL, 0, 0);
				SyncOperation();
				SyncList();
			}
		}
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
			CurrentSlider()->increment(float(sign * (int)tab.lParam), axis);
		}
		else {
			CurrentSlider()->increment(0, axis);
		}
		SyncList();
		ApplySlider();
	}

	void PoserWindow::SyncBones() {
		LRESULT res = SendMessage(m_listCategories, LB_GETCURSEL, 0, 0);
		loc_currentCategory = PoseMods::FrameCategory(res);
		if (res != LB_ERR) {
			if ((PoseMods::FrameCategory)res == PoseMods::FrameCategory::Room) {
				LRESULT cur = SendMessage(m_listBones, LB_GETCURSEL, 0, 0);
				SendMessage(m_listBones, LB_RESETCONTENT, 0, 0);
				for (auto s = loc_sliderInfosRoom.begin(); s != loc_sliderInfosRoom.end(); s++) {
					SendMessage(this->m_listBones, LB_ADDSTRING, 0, LPARAM(General::CastToWString(s->first).c_str()));
				}
				SendMessage(m_listBones, LB_SETCURSEL, cur, 0);
			}
			else if ((PoseMods::FrameCategory)res == PoseMods::FrameCategory::Prop) {
				LRESULT cur = SendMessage(m_listBones, LB_GETCURSEL, 0, 0);
				SendMessage(m_listBones, LB_RESETCONTENT, 0, 0);
				for (auto s = loc_sliderInfosProps[loc_targetChar->id].begin(); s != loc_sliderInfosProps[loc_targetChar->id].end(); s++) {
					SendMessage(this->m_listBones, LB_ADDSTRING, 0, LPARAM(General::CastToWString(s->first).c_str()));
				}
				SendMessage(m_listBones, LB_SETCURSEL, cur, 0);
			}
			else {
				SendMessage(m_listBones, LB_RESETCONTENT, 0, 0);
				for (auto s : loc_sliderCategories[(PoseMods::FrameCategory)res]) {
					SendMessage(this->m_listBones, LB_ADDSTRING, 0, LPARAM(loc_sliderInfos.at(s).descr.c_str()));
				}
				SendMessage(m_listBones, LB_SETCURSEL, loc_targetChar->CategoryCurrentSlider[PoseMods::FrameCategory(res)], 0);
			}
		}
	}

	void PoserWindow::SyncOperation() {
		LRESULT cat = SendMessage(m_listCategories, LB_GETCURSEL, 0, 0);
		LRESULT res = SendMessage(m_listBones, LB_GETCURSEL, 0, 0);
		if (res != LB_ERR) {
			SliderInfo* slider = nullptr;
			if ((PoseMods::FrameCategory)cat == PoseMods::FrameCategory::Room) {
				WCHAR name[255];
				SendMessage(m_listBones, LB_GETTEXT, res, (LPARAM)name);
				loc_currentRoomFrame = General::CastToString(std::wstring(name));
				auto match = loc_sliderInfosRoom.find(General::CastToString(std::wstring(name)));
				if (match != loc_sliderInfosRoom.end()) {
					slider = &match->second;
				}
			}
			else if ((PoseMods::FrameCategory)cat == PoseMods::FrameCategory::Prop) {
				WCHAR name[255];
				SendMessage(m_listBones, LB_GETTEXT, res, (LPARAM)name);
				loc_currentPropsFrame[loc_targetChar->id] = General::CastToString(std::wstring(name));
				auto match = loc_sliderInfosProps[loc_targetChar->id].find(General::CastToString(std::wstring(name)));
				if (match != loc_sliderInfosProps[loc_targetChar->id].end()) {
					slider = &match->second;
				}
			}
			else {
				loc_targetChar->CategoryCurrentSlider[(PoseMods::FrameCategory)cat] = res;
				int idx = loc_sliderCategories[(PoseMods::FrameCategory)cat][res];
				loc_targetChar->CurrentSlider = &loc_targetChar->SliderInfos[idx];
				slider = CurrentSlider();
			}
			if (slider) {
				SendMessage(m_listOperation, LB_SETCURSEL, slider->currentOperation, 0);
				if (slider->xxFrame)
					TabCtrl_SetCurSel(m_tabShowHide, GetIsHiddenFrame(slider->xxFrame) ? 1 : 0);
			}
			SyncList();
		}
	}

	void PoserWindow::SyncEdit() {
		if (!loc_targetChar) return;
		loc_syncing = true;
		float valueX = General::GetEditFloat(m_edValueX);
		float valueY = General::GetEditFloat(m_edValueY);
		float valueZ = General::GetEditFloat(m_edValueZ);
		CurrentSlider()->setValue(valueX, valueY, valueZ);
		//SendMessage(m_sliderValueX, TBM_SETPOS, TRUE, CurrentSlider()->toSliderX());
		//SendMessage(m_sliderValueY, TBM_SETPOS, TRUE, CurrentSlider()->toSliderY());
		//SendMessage(m_sliderValueZ, TBM_SETPOS, TRUE, CurrentSlider()->toSliderZ());
		loc_syncing = false;
	}

	void PoserWindow::SyncList() {
		if (!loc_targetChar) return;
		loc_syncing = true;
		SendMessage(m_sliderValueX, TBM_SETPOS, TRUE, CurrentSlider()->toSliderX());
		SendMessage(m_sliderValueY, TBM_SETPOS, TRUE, CurrentSlider()->toSliderY());
		SendMessage(m_sliderValueZ, TBM_SETPOS, TRUE, CurrentSlider()->toSliderZ());
		TCHAR number[52];
		_snwprintf_s(number, 52, 16, TEXT("%.3f"), CurrentSlider()->getValue(X));
		SendMessage(m_edValueX, WM_SETTEXT, 0, (LPARAM)number);
		_snwprintf_s(number, 52, 16, TEXT("%.3f"), CurrentSlider()->getValue(Y));
		SendMessage(m_edValueY, WM_SETTEXT, 0, (LPARAM)number);
		_snwprintf_s(number, 52, 16, TEXT("%.3f"), CurrentSlider()->getValue(Z));
		SendMessage(m_edValueZ, WM_SETTEXT, 0, (LPARAM)number);
		loc_syncing = false;
	}

	void PoserWindow::SyncSlider() {
		if (!loc_targetChar) return;
		loc_syncing = true;
		CurrentSlider()->fromSliderX(SendMessage(m_sliderValueX, TBM_GETPOS, 0, 0));
		CurrentSlider()->fromSliderY(SendMessage(m_sliderValueY, TBM_GETPOS, 0, 0));
		CurrentSlider()->fromSliderZ(SendMessage(m_sliderValueZ, TBM_GETPOS, 0, 0));
		TCHAR number[52];
		float x, y, z;
		if (CurrentSlider()->currentOperation == Rotate) {
			float angle[3];
			CurrentSlider()->getEulerAngles(angle);
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

	void ApplySlider(void* slider) {
		SliderInfo* targetSlider = slider == NULL ? CurrentSlider() : static_cast<SliderInfo*>(slider);
		ExtClass::Frame *frame = targetSlider->xxFrame;
		if(frame) {
			//note that somehow those frame manipulations dont quite work as expected;
			//by observation, rotation happens around the base of the bone whos frame got manipulated,
			//rather than the tip.
			//so to correct that, we gotta translate back

			ExtClass::Frame* origFrame = &frame->m_children[0];

			D3DMATRIX transMatrix;
			(*Shared::D3DXMatrixTranslation)(&transMatrix, targetSlider->translate.value[X], targetSlider->translate.value[Y], targetSlider->translate.value[Z]);
			D3DMATRIX rotMatrix;
			//(*Shared::D3DXMatrixRotationYawPitchRoll)(&rotMatrix, targetSlider->rotate.value[Y], targetSlider->rotate.value[X], targetSlider->rotate.value[Z]);
			(*Shared::D3DXMatrixRotationQuaternion)(&rotMatrix, &targetSlider->getRotation());
			D3DMATRIX scaleMatrix;
			(*Shared::D3DXMatrixScaling)(&scaleMatrix, targetSlider->scale.value[X], targetSlider->scale.value[Y], targetSlider->scale.value[Z]);
			D3DMATRIX matrix = origFrame->m_matrix5;
			(*Shared::D3DXMatrixMultiply)(&matrix, &matrix, &transMatrix);
			(*Shared::D3DXMatrixMultiply)(&matrix, &matrix, &rotMatrix);
			(*Shared::D3DXMatrixMultiply)(&rotMatrix, &scaleMatrix, &rotMatrix);

			D3DXVECTOR3 translations;
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

	void FrameModEvent(ExtClass::XXFile* xxFile) {
		using namespace ExtClass;
		static const char prefix[]{ "pose_" };
		static const char prefixTrans[]{ "pose_tr_" };
		static const char prefixRot[]{ "pose_rot_" };
		static const char propFramePrefix[]{ "AS00_N_Prop" };
		static const char propPrefixTrans[]{ "prop_tr_" };
		static const char propPrefixRot[]{ "prop_rot_" };
		static const std::wstring suffixTrans(L" TR");
		static const std::wstring suffixRot(L" ROT");

		if (xxFile == NULL) return;
		if (loc_targetCharacters.size() == 0) return;
		PoserCharacter* targetChar = nullptr;
		ExtClass::CharacterStruct::Models model;
		model = General::GetModelFromName(xxFile->m_name);
		targetChar = loc_loadCharacter;

		auto InsertFrame = [&xxFile](ExtClass::Frame* bone, ExtClass::Frame** child, const char* prefix) {
			//make copy of the bone first
			Frame* newMatch = (Frame*)Shared::IllusionMemAlloc(sizeof(Frame));
			memcpy_s(newMatch, sizeof(Frame), bone, sizeof(Frame));

			//turn match into a copy of the root for now, since there are a lot of members i dont know
			memcpy_s(bone, sizeof(Frame), xxFile->m_root, sizeof(Frame));

			//change parent and child stuff
			bone->m_parent = newMatch->m_parent;
			bone->m_nChildren = 1;
			bone->m_children = newMatch;
			newMatch->m_parent = bone;
			for (unsigned int i = 0; i < newMatch->m_nChildren; i++) {
				newMatch->m_children[i].m_parent = newMatch;
			}

			//change name
			int namelength = newMatch->m_nameBufferSize + strlen(prefix);
			bone->m_name = (char*)Shared::IllusionMemAlloc(namelength);
			bone->m_nameBufferSize = namelength;
			strcpy_s(bone->m_name, bone->m_nameBufferSize, prefix);
			strcat_s(bone->m_name, bone->m_nameBufferSize, newMatch->m_name);
			*child = newMatch;
		};

		if (model == ExtClass::CharacterStruct::SKELETON) {
			if (targetChar->Character->m_xxSkeleton != xxFile) {
				for (PoserCharacter* c : loc_targetCharacters) {
					if (c->Character->m_xxSkeleton == xxFile)
						targetChar = c;
				}
			}
		}
		else if (model == ExtClass::CharacterStruct::FACE || model == ExtClass::CharacterStruct::TONGUE || model == ExtClass::CharacterStruct::SKIRT) {
			targetChar = loc_loadCharacter;
		}
		else if (model == ExtClass::CharacterStruct::H3DROOM) {
			xxFile->EnumBonesPostOrder([&](ExtClass::Frame* bone) {

				//make copy of the bone first
				Frame* trans, *rot;
				Frame* child = nullptr;
				InsertFrame(bone, &child, prefixTrans); 
				trans = bone;
				bone = child;
				InsertFrame(bone, &child, prefixRot);
				rot = bone;

				auto match = loc_sliderInfosRoom.find(bone->m_name);
				if (match != loc_sliderInfosRoom.end()) {
					match->second.xxFrame = bone;
					ApplySlider(&match->second);
				}
				else {
					SliderInfo slider;
					slider.descr = General::CastToWStringN(child->m_name, child->m_nameBufferSize) + suffixTrans;
					slider.frameName = slider.descr;
					slider.reset();
					slider.xxFrame = trans;
					slider.category = PoseMods::FrameCategory::Room;
					slider.setCurrentOperation(Rotate);
					loc_sliderInfosRoom[General::CastToString(slider.frameName)] = slider;
					ApplySlider(&slider);

					slider.descr = General::CastToWStringN(child->m_name, child->m_nameBufferSize) + suffixRot;
					slider.frameName = slider.descr;
					slider.xxFrame = rot;
					loc_sliderInfosRoom[General::CastToString(slider.frameName)] = slider;
					ApplySlider(&slider);
				}
			});
		}
		else
			return;

		//adjust bone matrizes
		xxFile->EnumBonesPostOrder([&](ExtClass::Frame* bone) {
			
			//try to find matches in both the bone rules and slider rules
			auto match = loc_frameMap.find(bone->m_name);

			//apply matches by adding matrizes
			if (match != loc_frameMap.end()) {
				if (model != ExtClass::CharacterStruct::SKIRT || targetChar->SliderInfos[match->second].category == PoseMods::FrameCategory::Skirt) {
					//make copy of the bone first
					Frame* child = nullptr;
					InsertFrame(bone, &child, prefix);

					targetChar->SliderInfos[match->second].xxFrame = bone;
					ApplySlider(&targetChar->SliderInfos[match->second]);
				}
			} 
			else {
				if (strncmp(bone->m_name, propFramePrefix, sizeof(propFramePrefix) - 1) == 0) {
					auto& map = loc_sliderInfosProps[targetChar->id];

					//make copy of the bone first
					Frame* trans, *rot;
					Frame* child = nullptr;
					InsertFrame(bone, &child, prefixTrans);
					trans = bone;
					bone = child;
					InsertFrame(bone, &child, prefixRot);
					rot = bone;

					auto sliderTrans = map.find(trans->m_name);
					auto sliderRot= map.find(rot->m_name);
					if (sliderTrans != map.end() && sliderRot != map.end()) {
						sliderTrans->second.xxFrame = trans;
						sliderRot->second.xxFrame = rot;
						ApplySlider(&sliderTrans->second);
						ApplySlider(&sliderRot->second);
					}
					else {
						SliderInfo info;
						info.descr = General::CastToWStringN(child->m_name, child->m_nameBufferSize) + suffixTrans;
						info.frameName = info.descr;
						info.reset();
						info.xxFrame = trans;
						info.setCurrentOperation(Rotate);
						info.category = PoseMods::FrameCategory::Prop;
						map[trans->m_name] = info;

						info.descr = General::CastToWStringN(child->m_name, child->m_nameBufferSize) + suffixRot;
						info.frameName = info.descr;
						info.xxFrame = rot;
						map[rot->m_name] = info;

						ApplySlider(&info);
					}
				}
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
			PoseMods::FrameCategory category = std::get<0>(elem);
			std::string& strFrame = std::get<1>(elem);
			std::string& strDesc = std::get<2>(elem);

			std::wstring wstrFrame(strFrame.begin(), strFrame.end());
			info.frameName = wstrFrame;
			std::wstring wstrDescr(strDesc.begin(),strDesc.end());
			info.descr = wstrDescr;
			
			info.reset();

			info.xxFrame = NULL;
			info.setCurrentOperation(Rotate);
			info.category = category;

			loc_sliderInfos.push_back(info);
			loc_sliderCategories[category].push_back(loc_sliderInfos.size() - 1);
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

		if (picojson::get_last_error().empty() && json.is<object>()) {
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
					float x, y, z;
					x = elem.matrix[0];
					y = elem.matrix[1];
					z = elem.matrix[2];
					(*Shared::D3DXQuaternionRotationYawPitchRoll)(&slider->rotation.quaternion, y, x, z);
					slider->translate.value[X] = elem.matrix[3];
					slider->translate.value[Y] = elem.matrix[4];
					slider->translate.value[Z] = elem.matrix[5];
					slider->scale.value[X] = elem.matrix[6];
					slider->scale.value[Y] = elem.matrix[7];
					slider->scale.value[Z] = elem.matrix[8];
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
			int version = 1;
			SliderInfo* slider;
			try {
				auto versionCheck = load.find("_VERSION_");
				if (versionCheck != load.end()) {
					version = (int)versionCheck->second.get<double>();
				}
				c->Character->m_xxSkeleton->m_poseNumber = (int)load.at("pose").get<double>();
				c->Character->m_xxSkeleton->m_animFrame = (float)load.at("frame").get<double>();
				object sliders = load.at("sliders").get<object>();
				for (auto it = loc_targetChar->SliderInfos.begin(); it != loc_targetChar->SliderInfos.end(); it++) {
					it->reset();
					ApplySlider(&(*it));
				}

				auto LoadSlider = [&version](SliderInfo* slider, array& mods) {
					if ((mods.size() == 9 && version == 1) || (mods.size() == 10 && version == 2)) {
						float x, y, z, w;
						int index = 0;
						x = (float)mods[index++].get<double>();
						y = (float)mods[index++].get<double>();
						z = (float)mods[index++].get<double>();
						if (version == 1) {
							(*Shared::D3DXQuaternionRotationYawPitchRoll)(&slider->rotation.quaternion, y, x, z);
						}
						else if (version == 2) {
							w = (float)mods[index++].get<double>();
							slider->rotation.quaternion.x = x;
							slider->rotation.quaternion.y = y;
							slider->rotation.quaternion.z = z;
							slider->rotation.quaternion.w = w;
						}
						slider->translate.value[X] = (float)mods[index++].get<double>();
						slider->translate.value[Y] = (float)mods[index++].get<double>();
						slider->translate.value[Z] = (float)mods[index++].get<double>();
						slider->scale.value[X] = (float)mods[index++].get<double>();
						slider->scale.value[Y] = (float)mods[index++].get<double>();
						slider->scale.value[Z] = (float)mods[index++].get<double>();
						ApplySlider(slider);
					}
					else {
						//invalid json data
					}
				};
				for (auto s = sliders.cbegin(); s != sliders.cend(); s++) {
					auto match = c->FrameMap.find((*s).first);
					if (match != loc_targetChar->FrameMap.end()) {
						array mods = (*s).second.get<array>();
						LoadSlider(&c->SliderInfos[match->second], mods);
					}
					else {
						auto match = loc_sliderInfosProps[c->id].find((*s).first);
						if (match != loc_sliderInfosProps[c->id].end()) {
							array mods = (*s).second.get<array>();
							LoadSlider(&match->second, mods);
						}
					}
				}
			}
			catch (std::out_of_range& e) {
				//key doesn't exist
			}
			catch (std::runtime_error& e) {
				//invalid json data
			}

			try {
				object face = load.at("face").get<object>();
				auto it = face.find("mouth");
				if (it != face.end())
					c->GetFace()->m_mouth = (int)it->second.get<double>();
				it = face.find("mouthopen");
				if (it != face.end())
					c->GetFace()->m_mouthOpen = (float)it->second.get<double>();
				it = face.find("eye");
				if (it != face.end())
					c->GetFace()->m_eye = (int)it->second.get<double>();
				it = face.find("eyeopen");
				if (it != face.end())
					c->GetFace()->m_eyeOpen = (float)it->second.get<double>();
				it = face.find("eyebrow");
				if (it != face.end()) {
					int eyebrow = c->GetFace()->m_eyebrow;
					eyebrow = eyebrow - (eyebrow % 7);
					int newEyebrow = (int)it->second.get<double>() % 7;
					c->GetFace()->m_eyebrow = eyebrow + newEyebrow;
				}
				it = face.find("blush");
				if (it != face.end())
					*c->GetFace()->GetBlush() = (float)(it->second.get<double>() / 9.0f);
				it = face.find("blushlines");
				if (it != face.end())
					*c->GetFace()->GetBlushLines() = (float)(it->second.get<double>() / 9.0f);
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
		json["_VERSION_"] = value((double)2);
		json["pose"] = value((double)c->Character->m_xxSkeleton->m_poseNumber);
		json["frame"] = value((double)c->Character->m_xxSkeleton->m_animFrame);
		value::object sliders;
		auto SaveSlider = [&sliders](auto& slider) {
			value::array values(10);
			values[0] = value((double)slider.rotation.quaternion.x);
			values[1] = value((double)slider.rotation.quaternion.y);
			values[2] = value((double)slider.rotation.quaternion.z);
			values[3] = value((double)slider.rotation.quaternion.w);
			values[4] = value((double)slider.translate.value[X]);
			values[5] = value((double)slider.translate.value[Y]);
			values[6] = value((double)slider.translate.value[Z]);
			values[7] = value((double)slider.scale.value[X]);
			values[8] = value((double)slider.scale.value[Y]);
			values[9] = value((double)slider.scale.value[Z]);
			sliders[std::string(slider.frameName.cbegin(), slider.frameName.cend())] = value(values);
		};
		for (SliderInfo& slider : c->SliderInfos) {
			SaveSlider(slider);
		}
		for (auto i = loc_sliderInfosProps[c->id].begin(); i != loc_sliderInfosProps[c->id].end(); i++) {
			SaveSlider(i->second);
		}
		json["sliders"] = value(sliders);

		value::object face;
		face["eye"] = value((double)c->GetFace()->m_eye);
		face["eyeopen"] = value((double)c->GetFace()->m_eyeOpen);
		face["eyebrow"] = value((double)c->GetFace()->m_eyebrow);
		face["mouth"] = value((double)c->GetFace()->m_mouth);
		face["mouthopen"] = value((double)c->GetFace()->m_mouthOpen);
		face["blush"] = value(round((double)(*c->GetFace()->GetBlush() * 9.0f)));
		face["blushlines"] = value(round((double)(*c->GetFace()->GetBlushLines() * 9.0f)));
		json["face"] = value(face);
		return value(json);
	}

	void PoserWindow::LoadCloth(std::vector<BYTE> &file) {
		ClothFile load(file);
		if (!load.IsValid()) return;
		ExtClass::CharacterData::Clothes* cloth = &loc_targetChar->Character->m_charData->m_clothes[loc_targetChar->Character->m_currClothes];
		cloth->slot = load.m_slot;
		cloth->skirtLength = load.m_shortSkirt;
		cloth->socks = load.m_socksId;
		cloth->indoorShoes = load.m_shoesIndoorId;
		cloth->outdoorShoes = load.m_shoesOutdoorId;
		cloth->isOnePiece = load.m_isOnePiece;
		cloth->hasUnderwear = load.m_hasUnderwear;
		cloth->hasSkirt = load.m_hasSkirt;
		cloth->colorTop1 = load.m_colorTop1;
		cloth->colorTop2 = load.m_colorTop2;
		cloth->colorTop3 = load.m_colorTop3;
		cloth->colorTop4 = load.m_colorTop4;
		cloth->colorBottom1 = load.m_colorBottom1;
		cloth->colorBottom2 = load.m_colorBottom2;
		cloth->colorUnderwear = load.m_colorUnderwear;
		cloth->colorSocks = load.m_colorSocks;
		cloth->colorIndoorShoes = load.m_colorIndoorShoes;
		cloth->colorOutdoorShoes = load.m_colorOutdoorShoes;
		cloth->textureBottom1 = load.m_skirtTextureId;
		cloth->textureUnderwear = load.m_underwearTextureId;
		cloth->textureBottom1Hue = load.m_skirtHue;
		cloth->textureBottom1Lightness = load.m_skirtBrightness;
		cloth->shadowBottom1Hue = load.m_skirtShadowHue;
		cloth->shadowBottom1Lightness = load.m_skirtShadowBrightness;
		cloth->textureUnderwearHue = load.m_underwearHue;
		cloth->textureUnderwearLightness = load.m_underwearBrightness;
		cloth->shadowUnderwearHue = load.m_underwearShadowHue;
		cloth->shadowUnderwearLightness = load.m_underwearShadowBrightness;
	}
}
