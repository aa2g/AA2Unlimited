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
#include "Files\Logger.h"
#include "resource.h"
#include "defs.h"

#include "PoserController.h"
#include "StdAfx.h"
#include "3rdparty\picojson\picojson.h"
#include "Files/PoseMods.h"
#include "Strsafe.h"

namespace Poser {

#define X 0
#define Y 1
#define Z 2
#define W 3

	PoserController g_PoserController;
	SceneType currentScene = NoScene;
	int characterCount = 0;

	struct SliderHelper {
		int m_currentSlidingAxis;
		PoserController::SliderInfo::Operation m_currentOperation;
		D3DXVECTOR3 m_currentSlidingRotationAxisVector;
		D3DXQUATERNION m_currentSlidingRotationBase;
		PoserController::SliderInfo::TranslationScaleData m_translationScaleBase;
	} loc_sliderHelper;

	bool loc_syncing;

	void StartEvent(SceneType type) {
		currentScene = type;
		//g_PoserController.StartPoser();
		//LUA_EVENT_NORET("poserstart");
	}

	void EndEvent() {
		//LUA_EVENT_NORET("poserstop");
		//g_PoserController.StopPoser();
		currentScene = SceneType::NoScene;
	}

	void AddCharacter(ExtClass::CharacterStruct* charStruct) {
		g_PoserController.AddCharacter(charStruct);
	}

	void RemoveCharacter(ExtClass::CharacterStruct* charStruct) {
		g_PoserController.RemoveCharacter(charStruct);
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

		/*
		case WM_HSCROLL: {
			PoserWindow* thisPtr = (PoserWindow*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (thisPtr == NULL) return FALSE;
			HWND wnd = (HWND)lparam;
			if (wnd == NULL) break; //not slider control, but automatic scroll
			if (!loc_syncing) {
				PoserController::SliderInfo* current = g_PoserController.CurrentSlider();
				if (current) {
					int axis = wnd == thisPtr->m_sliderValueX ? X : wnd == thisPtr->m_sliderValueY ? Y : wnd == thisPtr->m_sliderValueZ ? Z : -1;
					if (axis >= 0) {
						int order = thisPtr->GetCurrentModifier();
						if (axis != loc_sliderHelper.m_currentSlidingAxis) {
							loc_sliderHelper.m_currentSlidingAxis = axis;
							loc_sliderHelper.m_currentOperation = current->currentOperation;
							if (current->currentOperation == PoserController::SliderInfo::Rotate) {
								loc_sliderHelper.m_currentSlidingRotationAxisVector = *current->rotation.rotAxes[axis].vector();
								loc_sliderHelper.m_currentSlidingRotationBase = current->getRotation();
							}
							else if (current->currentOperation == PoserController::SliderInfo::Translate) {
								loc_sliderHelper.m_translationScaleBase = current->translate;
							}
							else {
								loc_sliderHelper.m_translationScaleBase = current->scale;
							}
						}
						if (loc_sliderHelper.m_currentOperation == PoserController::SliderInfo::Rotate) {
							current->rotation.setRotationQuaternion(loc_sliderHelper.m_currentSlidingRotationBase);
							current->rotation.rotateAxis(loc_sliderHelper.m_currentSlidingAxis, (float)(SendMessage(wnd, TBM_GETPOS, 0, 0) - 0x8000) / 114159 * order);
						}
						else if (loc_sliderHelper.m_currentOperation == PoserController::SliderInfo::Translate) {
							current->translate.value[axis] = loc_sliderHelper.m_translationScaleBase.value[axis] + (float)(SendMessage(wnd, TBM_GETPOS, 0, 0) - 0x8000) / 0x10000 * order;
						}
						else if (loc_sliderHelper.m_currentOperation == PoserController::SliderInfo::Scale) {
							current->scale.value[axis] = loc_sliderHelper.m_translationScaleBase.value[axis] + (float)(SendMessage(wnd, TBM_GETPOS, 0, 0) - 0x8000) / 0x10000 * order;
						}
						thisPtr->SyncSlider();
						current->Apply();
					}
				}
			}
			break; }*/

/*	void PoserWindow::SyncStyles() {
		CharInstData* card = &AAPlay::g_characters[g_PoserController.CurrentCharacter()->m_character->m_seat];
		if (!card->IsValid()) return;
		SendMessage(this->m_listStyles, LB_RESETCONTENT, 0, 0);
		auto styles = card->m_cardData.m_styles;
		for (int i = 0; i < styles.size(); i++) {
			SendMessage(this->m_listStyles, LB_ADDSTRING, 0, LPARAM(styles[i].m_name));
		}
		SendMessage(this->m_listStyles, LB_SETCURSEL, card->m_cardData.GetCurrAAUSet(), 0);
	}*/

	void FrameModEvent(ExtClass::XXFile* xxFile) {
		if (xxFile) {
			LUA_EVENT_NORET("poserframemod", xxFile->m_name);
			g_PoserController.FrameModEvent(xxFile);
		}
	}

	void bindLua() {
		auto b = g_Lua[LUA_BINDING_TABLE].get();
		b["GetPoserCharacter"] = LUA_LAMBDA({
			ExtClass::CharacterStruct* charStruct = (ExtClass::CharacterStruct*)(s.get(1));
			s.push(g_PoserController.GetPoserCharacter(charStruct));
		});
		b["GetPoserController"] = LUA_LAMBDA({
			s.push(&g_PoserController);
		});
	}
}
