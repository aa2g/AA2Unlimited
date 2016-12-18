#include "Poser.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <vector>
#include <map>

#include "General\IllusionUtil.h"
#include "General\Util.h"
#include "Functions\Shared\Globals.h"
#include "Files\PoseMods.h"
#include "Files\Config.h"
#include "resource.h"
#include "config.h"

namespace Poser {

	PoserWindow g_PoserWindow;

	ExtClass::CharacterStruct* loc_targetChar = NULL;

	struct SliderInfo {
		std::wstring frame;
		enum Mod {
			YAW,PITCH,ROLL
		} mod;
		std::wstring descr;
	};
	std::vector<SliderInfo> loc_sliderInfos;
	struct MapData {
		ExtClass::Frame* frame;
		float yaw,pitch,roll;
	};
	std::map<std::string,MapData> loc_frameMap;

	void GenSliderInfo();
	void GenFrameMap();

	void StartEvent() {
		if (!g_Config.GetKeyValue(Config::USE_POSER).bVal) return;
		GenSliderInfo();
		GenFrameMap();
		g_PoserWindow.Init();
	}

	void EndEvent() {
		if (!g_Config.GetKeyValue(Config::USE_POSER).bVal) return;
		loc_frameMap.clear();
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
			thisPtr->m_edFrame = GetDlgItem(hwndDlg,IDC_PPS_EDFRAME);

			thisPtr->InitSliders();
			ShowWindow(GetDlgItem(hwndDlg,IDC_PPS_EDEXAMPLE),SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg,IDC_PPS_SLDEXAMPLE),SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg,IDC_PPS_STEXAMPLE),SW_HIDE);

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
			if (ignoreNextSlider) {
				ignoreNextSlider = false;
				//return TRUE;
			}
			HWND wnd = (HWND)lparam;
			if (wnd == NULL) break; //not slider control, but automatic scroll
			for (size_t i = 0; i < thisPtr->m_sliders.size(); i++) {
				if (wnd == thisPtr->m_sliders[i].GetSlider()) {
					ignoreNextSlider = true;
					thisPtr->m_sliders[i].Sync(false);
					thisPtr->ApplySlider(i);
					break;
				}
			}
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
				std::wstring frame;
				frame = std::to_wstring(skeleton->m_animFrame);
				SendMessage(thisPtr->m_edFrame,WM_SETTEXT,0,(LPARAM)frame.c_str());
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
				if (ignoreNextSlider) {
					ignoreNextSlider = false;
					return TRUE;
				}
				
				for (size_t i = 0; i < thisPtr->m_sliders.size(); i++) {
					if (ed == thisPtr->m_sliders[i].GetEdit()) {
						ignoreNextSlider = true;
						thisPtr->m_sliders[i].Sync(true);
						thisPtr->ApplySlider(i);
						break;
					}
				}
				return TRUE; }
			};
			break; }
		}

		return FALSE;
	}

	void PoserWindow::ApplySlider(int index) {
		std::wstring str = loc_sliderInfos[index].frame;
		size_t n;
		char mbFrame[256];
		wcstombs_s(&n,mbFrame,str.c_str(),256);
		auto match = loc_frameMap.find(mbFrame);
		if(match != loc_frameMap.end()) {
			switch(loc_sliderInfos[index].mod) {
			case SliderInfo::YAW:
				match->second.yaw = m_sliders[index].GetCurrVal();
				break;
			case SliderInfo::PITCH:
				match->second.pitch = m_sliders[index].GetCurrVal();
				break;
			case SliderInfo::ROLL:
				match->second.roll = m_sliders[index].GetCurrVal();
				break;
			default:
				break;
			}
			if(match->second.frame != NULL) {
				
				(*Shared::D3DXMatrixRotationYawPitchRoll)(&match->second.frame->m_matrix1,match->second.yaw,match->second.pitch,match->second.roll);
			}
		}
	}


	void PoserWindow::InitSliders() {

		m_sliders.resize(loc_sliderInfos.size());
		RECT start;
		GetWindowRect(GetDlgItem(m_dialog,IDC_PPS_STEXAMPLE), &start);
		int x = start.left,y = start.top;
		for (int i = 0; i < loc_sliderInfos.size(); i++) {
			m_sliders[i] = GuiSlider(m_dialog,loc_sliderInfos[i].descr.c_str(),x,y,-M_PI,M_PI);
			SendMessage(m_sliders[i].GetEdit(),WM_SETTEXT,0,(LPARAM)TEXT("0"));
			m_sliders[i].Sync(true);
			y += 50;
		}

		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_PAGE | SIF_RANGE;
		si.nMin = 0;
		si.nMax = y;
		si.nPage = 200;
		SetScrollInfo(m_dialog,SB_VERT,&si,FALSE);

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
				for (int i = 0; i < newMatch->m_nChildren; i++) {
					newMatch->m_children[i].m_parent = newMatch;
				}

				//change name
				int namelength = newMatch->m_nameBufferSize + sizeof(prefix)-1;
				bone->m_name = (char*)Shared::IllusionMemAlloc(namelength);
				bone->m_nameBufferSize = namelength;
				strcpy_s(bone->m_name,bone->m_nameBufferSize,prefix);
				strcat_s(bone->m_name,bone->m_nameBufferSize,newMatch->m_name);

				match->second.frame = bone;
			}
		});



		//now, frames that represent a mesh have a bunch of bones; each bone has a pointer to its frame (more precisely,
		//its frames matrix2), which it uses to position its mesh. after this, those pointers will point to the artificial matrizes,
		//so we have to change that as well
		xxFile->EnumBonesPostOrder([&](ExtClass::Frame* frame) {
			for (int i = 0; i < frame->m_nBones; i++) {
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
			int iMod = std::get<1>(elem);
			std::string& strDesc = std::get<2>(elem);

			std::wstring wstrFrame(strFrame.begin(), strFrame.end());
			info.frame = wstrFrame;
			std::wstring wstrDescr(strDesc.begin(),strDesc.end());
			info.descr = wstrDescr;
			
			info.mod = (SliderInfo::Mod)iMod;

			loc_sliderInfos.push_back(info);
		}
	}

	void GenFrameMap() {
		for(auto& elem : loc_sliderInfos) {
			size_t n;
			char mbFrame[256];
			wcstombs_s(&n,mbFrame,elem.frame.c_str(), 256);

			auto match = loc_frameMap.find(mbFrame);
			if(match == loc_frameMap.end()) {
				MapData data;
				data.pitch = data.roll = data.yaw = 0;
				data.frame = NULL;
				auto pair = std::make_pair(std::string(mbFrame),data);
				loc_frameMap.insert(pair);
			}
		}
	}

}