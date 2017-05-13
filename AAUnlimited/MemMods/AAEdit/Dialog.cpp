#include "Dialog.h"

#include <Windows.h>
#include <Windowsx.h>

#include "MemMods\Hook.h"
#include "General\ModuleInfo.h"

#include "External\ExternalVariables\AAEdit\WindowData.h"
#include "Functions\AAEdit\UnlimitedDialog.h"
#include "Functions\AAEdit\Globals.h"
#include "Files\Logger.h"

/*
 * These are similar to the AAFace versions.
 * In fact, there are pretty much copied.
 * Good thing i have a cleaner hook this time, so i can make it work
 */
namespace EditInjections {
	namespace Dialog {

		void __stdcall DialogProcPre(void* internclass, HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam) {
			if (wnd != NULL && wnd == ExtVars::AAEdit::Dialogs[1]) {
				AAEdit::SystemDialogNotification(internclass, wnd, msg, wparam, lparam);
			}
			
			switch (msg) {
			case WM_COMMAND: {
				switch (HIWORD(wparam)) {
				case BN_CLICKED: {
					if (General::IsAAEdit && LOWORD(wparam) == 10032) { //Save character button
						if (!AAEdit::g_currChar.IsValid()) {
							DWORD charDataRule[]{ 0x353254, 0x2C, 0 };
							AAEdit::g_currChar.m_char = (ExtClass::CharacterStruct*) ExtVars::ApplyRule(charDataRule);
						}
						if (AAEdit::g_currChar.IsValid()) {
							AAEdit::g_currChar.m_cardData.UpdateAAUDataSet(AAEdit::g_currChar.m_cardData.GetCurrAAUSet(), AAEdit::g_currChar.m_char->m_charData);
							AAEdit::g_currChar.m_cardData.SwitchActiveAAUDataSet(0, AAEdit::g_currChar.m_char->m_charData);
						}
					}
					break;
				}
				}
				break;
			}
			//case WM_NOTIFY: {
			//	if (lparam == NULL) break;
			//	NMHDR* param = (NMHDR*)(lparam);
			//	if (param->hwndFrom == GetDlgItem(wnd, 10008)) {
			//		enum MakerTabs {
			//			System,	Figure,	Chest,	BodyColor,	Face,	Eyes,	EyeColor,
			//			Eyebrows,	FaceDetail,	Hair,	HairColor,	Character, Personality, Traits
			//		};
			//		if (param->code == TCN_SELCHANGING) {
			//			//save current changes
			//			if (!AAEdit::g_currChar.IsValid()) {
			//				DWORD charDataRule[]{ 0x353254, 0x2C, 0 };
			//				AAEdit::g_currChar.m_char = (ExtClass::CharacterStruct*) ExtVars::ApplyRule(charDataRule);
			//			}
			//			AAEdit::g_currChar.m_cardData.UpdateAAUDataSet(AAEdit::g_currChar.m_cardData.GetCurrAAUSet(), AAEdit::g_currChar.m_char->m_charData);
			//		}
			//		else if (param->code == TCN_SELCHANGE) {
			//			//determine the new tab
			//			int currTab = TabCtrl_GetCurSel(param->hwndFrom);
			//			//LOGPRIO(Logger::Priority::SPAM) << curr << "\n";
			//			//retrieve Style data
			//			if (!AAEdit::g_currChar.IsValid()) {
			//				DWORD charDataRule[]{ 0x353254, 0x2C, 0 };
			//				AAEdit::g_currChar.m_char = (ExtClass::CharacterStruct*) ExtVars::ApplyRule(charDataRule);
			//			}
			//			auto styleData = AAEdit::g_currChar.m_cardData.m_aauSets[AAEdit::g_currChar.m_cardData.GetCurrAAUSet()].m_charSetData;
			//			//display current changes
			//			switch (currTab) {
			//			case MakerTabs::Figure: {
			//				//get control elements
			//				HWND Height1[] = {
			//					GetDlgItem(wnd, 10040),
			//					GetDlgItem(wnd, 10041),
			//					GetDlgItem(wnd, 10042),
			//					GetDlgItem(wnd, 10043),
			//					GetDlgItem(wnd, 10044),
			//					GetDlgItem(wnd, 10045),
			//					GetDlgItem(wnd, 10046),
			//					GetDlgItem(wnd, 10047)
			//				};
			//				HWND Height2[] = {
			//					GetDlgItem(wnd, 10056),
			//					GetDlgItem(wnd, 10057),
			//					GetDlgItem(wnd, 10058),
			//					GetDlgItem(wnd, 10059),
			//					GetDlgItem(wnd, 10060),
			//					GetDlgItem(wnd, 10061),
			//					GetDlgItem(wnd, 10062),
			//					GetDlgItem(wnd, 10063)
			//				};
			//				HWND Figure[] = {
			//					GetDlgItem(wnd, 10048),
			//					GetDlgItem(wnd, 10049),
			//					GetDlgItem(wnd, 10050),
			//					GetDlgItem(wnd, 10051),
			//					GetDlgItem(wnd, 10052),
			//					GetDlgItem(wnd, 10053),
			//					GetDlgItem(wnd, 10054),
			//					GetDlgItem(wnd, 10055)
			//				};
			//				HWND HeadSizeTextBox = GetDlgItem(wnd, 10037);
			//				HWND HeadLengthTextBox = GetDlgItem(wnd, 10038);
			//				HWND WaistTextBox = GetDlgItem(wnd, 10039);
			//				if (AAEdit::g_currChar.m_char->m_charData->m_gender == 1)	//Female
			//				{
			//					//set Height
			//					Button_SetCheck(Height1[styleData.m_figure.height], BST_UNCHECKED);
			//					//set Figure
			//					Button_SetCheck(Height1[styleData.m_figure.figure], BST_UNCHECKED);
			//					//set HeadSize
			//					Edit_SetText(HeadSizeTextBox, (LPTSTR)std::to_string(styleData.m_figure.headSize).c_str());
			//					//set HeadLength
			//					Edit_SetText(HeadLengthTextBox, (LPTSTR)std::to_string(styleData.m_figure.headLength).c_str());
			//					//set Waist
			//					Edit_SetText(WaistTextBox, (LPTSTR)std::to_string(styleData.m_figure.waist).c_str());
			//				}
			//				else														//Male
			//				{
			//					//lol
			//				}
			//				break;
			//			}
			//			case MakerTabs::Chest: {
			//				if (AAEdit::g_currChar.m_char->m_charData->m_gender == 1)	//Female
			//				{
			//				}
			//				else														//Male
			//				{
			//					//lol
			//				}
			//				break;
			//			}
			//			case MakerTabs::BodyColor: {
			//				if (AAEdit::g_currChar.m_char->m_charData->m_gender == 1)	//Female
			//				{
			//				}
			//				else														//Male
			//				{
			//					//lol
			//				}
			//				break;
			//			}
			//			case MakerTabs::Face: {
			//				if (AAEdit::g_currChar.m_char->m_charData->m_gender == 1)	//Female
			//				{
			//				}
			//				else														//Male
			//				{
			//					//lol
			//				}
			//				break;
			//			}
			//			case MakerTabs::Eyes: {
			//				if (AAEdit::g_currChar.m_char->m_charData->m_gender == 1)	//Female
			//				{
			//				}
			//				else														//Male
			//				{
			//					//lol
			//				}
			//				break;
			//			}
			//			case MakerTabs::EyeColor: {
			//				if (AAEdit::g_currChar.m_char->m_charData->m_gender == 1)	//Female
			//				{
			//				}
			//				else														//Male
			//				{
			//					//lol
			//				}
			//				break;
			//			}
			//			case MakerTabs::Eyebrows: {
			//				if (AAEdit::g_currChar.m_char->m_charData->m_gender == 1)	//Female
			//				{
			//				}
			//				else														//Male
			//				{
			//					//lol
			//				}
			//				break;
			//			}
			//			case MakerTabs::FaceDetail: {
			//				if (AAEdit::g_currChar.m_char->m_charData->m_gender == 1)	//Female
			//				{
			//				}
			//				else														//Male
			//				{
			//					//lol
			//				}
			//				break;
			//			}
			//			case MakerTabs::Hair: {
			//				if (AAEdit::g_currChar.m_char->m_charData->m_gender == 1)	//Female
			//				{
			//				}
			//				else														//Male
			//				{
			//					//lol
			//				}
			//				break;
			//			}
			//			case MakerTabs::HairColor: {
			//				if (AAEdit::g_currChar.m_char->m_charData->m_gender == 1)	//Female
			//				{
			//				}
			//				else														//Male
			//				{
			//					//lol
			//				}
			//				break;
			//			}
			//			}
			//		}
			//	}
			//	break;
			//}
			}
		}

		void DialogProcPost() {

		}

		DWORD DialogProcReturnPoint;
		void __declspec(naked) DialogProcRedirect() {
			__asm {
				//original code
				push ebx
				//also, save edx
				push edx
				//copy parameters
				push[esp + 0x10]
				push[esp + 0x10]
				push[esp + 0x10]
				push[esp + 0x10]
				push esi //also, class as parameter
				call DialogProcPre
				pop edx
				//call original function
				mov ecx, esi
				call edx
				push eax //save return value
				call DialogProcPost
				pop eax //restore return value
				jmp[DialogProcReturnPoint]
			}
		}

		void DialogProcInject() {
			//general virtual handler for all windows, including the main window.
			//we can use the hwnd to determine which window it is and distribute the calls accordingly.
			//4 param ecx stdcall
			/*AA2Edit.exe+19ABD9 - 8B 4C 24 1C           - mov ecx,[esp+1C]
			AA2Edit.exe+19ABDD - 8B 06                 - mov eax,[esi]
			AA2Edit.exe+19ABDF - 8B 50 04              - mov edx,[eax+04]
			AA2Edit.exe+19ABE2 - 55 - push ebp{ lparam }
			AA2Edit.exe+19ABE3 - 51 - push ecx{ wparam }
			AA2Edit.exe+19ABE4 - 57 - push edi{ msg }
			AA2Edit.exe+19ABE5 - 53 - push ebx{ hwnd }
			AA2Edit.exe+19ABE6 - 8B CE - mov ecx, esi
			AA2Edit.exe+19ABE8 - FF D2 - call edx{ class->handler(hwnd, msg, wparam, lparam) }
			AA2Edit.exe+19ABEA - 8B D8                 - mov ebx,eax*/

			DWORD address = General::GameBase + 0x19ABE5;
			DWORD redirectAddress = (DWORD)(&DialogProcRedirect);
			Hook((BYTE*)address,
			{ 0x53,
			0x8B, 0xCE,
			0xFF, 0xD2 },						//expected values
			{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress },	//redirect to our function
				NULL);
			DialogProcReturnPoint = General::GameBase + 0x19ABEA;
		}


	}
}