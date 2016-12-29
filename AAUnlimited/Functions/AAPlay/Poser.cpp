#include "Poser.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <vector>
#include <map>
#include <CommCtrl.h>

#include "General\IllusionUtil.h"
#include "General\Util.h"
#include "Functions\Shared\Globals.h"
#include "Files\PoseMods.h"
#include "Files\Config.h"
#include "Files\PoseFile.h"
#include "resource.h"
#include "config.h"

namespace Poser {

	PoserWindow g_PoserWindow;

	ExtClass::CharacterStruct* loc_targetChar = NULL;

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
	};
	std::vector<SliderInfo> loc_sliderInfos;
	std::map<std::string,unsigned int> loc_frameMap;

	SliderInfo *loc_curSlider;

	bool loc_syncing;

	void GenSliderInfo();

	void StartEvent() {
		if (!g_Config.GetKeyValue(Config::USE_POSER).bVal) return;
		GenSliderInfo();
		g_PoserWindow.Init();
	}

	void EndEvent() {
		if (!g_Config.GetKeyValue(Config::USE_POSER).bVal) return;
		loc_sliderInfos.clear();
		loc_targetChar = NULL;
		g_PoserWindow.Hide();
	}

	void SetTargetCharacter(ExtClass::CharacterStruct* c) {
		if (!g_Config.GetKeyValue(Config::USE_POSER).bVal) return;
		loc_targetChar = c;
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
			thisPtr->m_edPose = GetDlgItem(hwndDlg,IDC_PPS_EDPOSE);
			thisPtr->m_edFrame = GetDlgItem(hwndDlg, IDC_PPS_EDFRAME);
			thisPtr->m_listBones = GetDlgItem(hwndDlg, IDC_PPS_LISTBONES);
			thisPtr->m_listOperation = GetDlgItem(hwndDlg, IDC_PPS_LISTOP);
			thisPtr->m_listAxis = GetDlgItem(hwndDlg, IDC_PPS_LISTAXIS);
			thisPtr->m_sliderValue = GetDlgItem(hwndDlg, IDC_PPS_SLIDERMOD);
			thisPtr->m_edValue = GetDlgItem(hwndDlg, IDC_PPS_EDVALUE);

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

			loc_curSlider = NULL;
			if (loc_sliderInfos.size()) {
				loc_curSlider = &loc_sliderInfos[0];
				thisPtr->SyncList();
			}

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
			thisPtr->SyncSlider();
			thisPtr->ApplySlider();
			break; }
		case WM_TIMER: {
			PoserWindow* thisPtr = (PoserWindow*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
			if (thisPtr == NULL) return FALSE;
			if (loc_targetChar == NULL) return TRUE;
			ExtClass::XXFile* skeleton = loc_targetChar->m_xxSkeleton;
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
			
			break; }
		case WM_COMMAND: {
			PoserWindow* thisPtr = (PoserWindow*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
			if (thisPtr == NULL) return FALSE;
			switch (HIWORD(wparam)) {
			case EN_CHANGE: {
				HWND ed = (HWND)lparam;
				if(LOWORD(wparam) == IDC_PPS_EDPOSE) {
					int val = General::GetEditInt(ed);
					ExtClass::XXFile* skeleton = loc_targetChar->m_xxSkeleton;
					if (skeleton == NULL) return TRUE;
					skeleton->m_poseNumber = val;
				}
				else if(LOWORD(wparam) == IDC_PPS_EDFRAME) {
					float val = General::GetEditFloat(ed);
					ExtClass::XXFile* skeleton = loc_targetChar->m_xxSkeleton;
					if (skeleton == NULL) return TRUE;
					skeleton->m_animFrame = val;
				}
				else if (LOWORD(wparam) == IDC_PPS_EDVALUE) {
					thisPtr->SyncEdit();
					thisPtr->ApplySlider();
				}

				return TRUE; }
			case BN_CLICKED: {
				PoserWindow* thisPtr = (PoserWindow*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
				if (thisPtr == NULL) return FALSE;
				int id = LOWORD(wparam);
				if(id == IDC_PPS_BTNSAVE) {
					const TCHAR* path = General::SaveFileDialog(NULL);
					if(path != NULL) {
						PoseFile saveFile;
						saveFile.SetPoseInfo(General::GetEditInt(thisPtr->m_edPose),General::GetEditFloat(thisPtr->m_edFrame));
						/*for (int i = 0; i < thisPtr->m_sliders.size(); i++) {
							if (thisPtr->m_sliders[i].GetCurrVal() == 0) continue;
							PoseFile::FrameMod mod;
							mod.frameName = std::string(loc_sliderInfos[i].frame.begin(),loc_sliderInfos[i].frame.end());;
							auto kind = loc_sliderInfos[i].mod;
							if (kind == SliderInfo::ROLL) mod.modKind = 'z';
							else if (kind == SliderInfo::YAW) mod.modKind = 'y';
							else mod.modKind = 'x';
							mod.value = thisPtr->m_sliders[i].GetCurrVal();
							saveFile.AddFrameMod(mod);
						}
						saveFile.DumpToFile(path);*/
					}
				}
				else if (id == IDC_PPS_BTNLOAD) {
					/*const TCHAR* path = General::OpenFileDialog(NULL);
					if(path != NULL) {
						for(auto& elem : thisPtr->m_sliders) {
							std::wstring num(TEXT("0"));
							SendMessage(elem.GetEdit(),WM_SETTEXT,0,(LPARAM)num.c_str());
						}
						PoseFile openFile(path);
						std::wstring str;
						str = std::to_wstring(openFile.GetPose());
						SendMessage(thisPtr->m_edPose,WM_SETTEXT,0,(LPARAM)str.c_str());
						str = std::to_wstring(openFile.GetFrame());
						SendMessage(thisPtr->m_edFrame,WM_SETTEXT,0,(LPARAM)str.c_str());
						for(auto elem : openFile.GetMods()) {
							std::wstring wstrName(elem.frameName.begin(),elem.frameName.end());
							SliderInfo::Mod modkind = SliderInfo::PITCH;
							if (elem.modKind == 'x') modkind = SliderInfo::PITCH;
							else if (elem.modKind == 'y') modkind = SliderInfo::YAW;
							else if (elem.modKind == 'z') modkind = SliderInfo::ROLL;
							for(int i = 0; i < loc_sliderInfos.size(); i++) {
								if(loc_sliderInfos[i].frame == wstrName && loc_sliderInfos[i].mod == modkind) {
									std::wstring num = std::to_wstring(elem.value);
									SendMessage(thisPtr->m_sliders[i].GetEdit(),WM_SETTEXT,0,(LPARAM)num.c_str());
									break;
								}
							}
						}

					}*/
				}
				else if (id == IDC_PPS_BTNRESET) {
					/*for(int i = 0; i < thisPtr->m_sliders.size(); i++) {
						static const TCHAR numZero[] = TEXT("0");
						SendMessage(thisPtr->m_sliders[i].GetEdit(),WM_SETTEXT,0,(LPARAM)numZero);
						thisPtr->ApplySlider(i);
					}*/
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
						loc_curSlider = &loc_sliderInfos[res];
						SendMessage(thisPtr->m_listOperation, LB_SETCURSEL, loc_curSlider->curOperation, 0);
						SendMessage(thisPtr->m_listAxis, LB_SETCURSEL, loc_curSlider->curAxis, 0);
						thisPtr->SyncList();
					}
					break; }
				case (IDC_PPS_LISTOP): {
					LRESULT res = SendMessage(thisPtr->m_listOperation, LB_GETCURSEL, 0, 0);
					if (res != LB_ERR) {
						loc_curSlider->curOperation = Operation(res);
						thisPtr->SyncList();
					}
					break; }
				case (IDC_PPS_LISTAXIS): {
					LRESULT res = SendMessage(thisPtr->m_listAxis, LB_GETCURSEL, 0, 0);
					if (res != LB_ERR) {
						loc_curSlider->curAxis = res;
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
		if (loc_syncing) return;
		loc_syncing = true;
		float value = General::GetEditFloat(m_edValue);
		loc_curSlider->setValue(value);
		SendMessage(m_sliderValue, TBM_SETPOS, TRUE, loc_curSlider->toSlider());
		loc_syncing = false;
	}

	void PoserWindow::SyncList() {
		loc_syncing = true;
		SendMessage(m_sliderValue, TBM_SETPOS, TRUE, loc_curSlider->toSlider());
		TCHAR number[52];
		_snwprintf_s(number, 52, 16, TEXT("%.3f"), *loc_curSlider->curValue());
		SendMessage(m_edValue, WM_SETTEXT, 0, (LPARAM)number);
		loc_syncing = false;
	}

	void PoserWindow::SyncSlider() {
		if (loc_syncing) return;
		loc_syncing = true;
		int pos = SendMessage(m_sliderValue, TBM_GETPOS, 0, 0);
		loc_curSlider->fromSlider(pos);
		TCHAR number[52];
		_snwprintf_s(number, 52, 16, TEXT("%.3f"), *loc_curSlider->curValue());
		SendMessage(m_edValue, WM_SETTEXT, 0, (LPARAM)number);
		loc_syncing = false;
	}

	void PoserWindow::ApplySlider() {
		ExtClass::Frame *frame = loc_curSlider->xxFrame;
		if(frame) {
			//note that somehow those frame manipulations dont quite work as expected;
			//by observation, rotation happens around the base of the bone whos frame got manipulated,
			//rather than the tip.
			//so to correct that, we gotta translate back

			ExtClass::Frame* origFrame = &frame->m_children[0];

			D3DMATRIX rotMatrix;
			(*Shared::D3DXMatrixRotationYawPitchRoll)(&rotMatrix, loc_curSlider->rotate.x, loc_curSlider->rotate.y, loc_curSlider->rotate.z);
			D3DMATRIX rotTransMatrix = origFrame->m_matrix5;
			(*Shared::D3DXMatrixMultiply)(&rotTransMatrix,&rotTransMatrix,&rotMatrix);

			D3DVECTOR3 translations;
			translations.x = origFrame->m_matrix5._41 - rotTransMatrix._41;
			translations.y = origFrame->m_matrix5._42 - rotTransMatrix._42;
			translations.z = origFrame->m_matrix5._43 - rotTransMatrix._43;

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
			SendMessageW(this->m_listBones, LB_ADDSTRING, 0, LPARAM(loc_sliderInfos.at(i).frameName.c_str()));
		}
	}



	void FrameModEvent(ExtClass::XXFile* xxFile) {
		using namespace ExtClass;
		static const char prefix[]{ "pose_" };

		if (xxFile == NULL) return;
		if (loc_targetChar == NULL) return;
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

				loc_sliderInfos[match->second].xxFrame = bone;
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
			
			//info.mod = (SliderInfo::Mod)iMod;
			info.translate = { 0,0,0,0,0 };
			info.rotate = { 0,0,0,(float)(-M_PI),(float)M_PI };
			info.scale = { 1,1,1,1,1 };

			info.curAxis = 0;
			info.curOperation = Rotate;

			info.xxFrame = NULL;

			loc_sliderInfos.push_back(info);
			loc_frameMap.insert(std::make_pair(strFrame, loc_sliderInfos.size() - 1));
		}
	}

}
