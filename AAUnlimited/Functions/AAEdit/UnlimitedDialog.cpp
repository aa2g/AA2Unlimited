#include "UnlimitedDialog.h"

#include "External\ExternalVariables\AAEdit\WindowData.h"
#include "External\ExternalClasses\TextureStruct.h"
#include "General\ModuleInfo.h"
#include "General\Util.h"
#include "Functions\AAEdit\Globals.h"
#include "resource.h"
#include "config.h"


namespace AAEdit {

const char UnlimitedDialog::ListSeperator[5] = " -> ";

UnlimitedDialog::UnlimitedDialog() : m_dialog(NULL), m_visible(false)
{
}


UnlimitedDialog::~UnlimitedDialog()
{
	if (m_dialog != NULL) {
		Destroy();
	}
}

void UnlimitedDialog::Initialize() 
{
	if (m_visible) return;
	if (m_dialog == NULL) {
		m_dialog = CreateDialogParam(General::DllInst, MAKEINTRESOURCE(IDD_AAUPROPDIALOG),
			NULL /*ExtVars::AAEdit::MainWnd*/, DialogProc, (LPARAM)this);
	}
	ShowWindow(m_dialog, SW_SHOW);
	m_visible = true;
}

void UnlimitedDialog::Hide() {
	ShowWindow(m_dialog, SW_HIDE);
	m_visible = true;
}

void UnlimitedDialog::Destroy() {
	DestroyWindow(m_dialog);
	m_dialog = NULL;
	m_visible = false;
}

/*
 * Refreshes the rule list based on the override rules set
 * in the global g_cardData struct
 */
void UnlimitedDialog::RefreshRuleList() {
	SendMessage(this->m_lbOverrides, LB_RESETCONTENT, 0, 0);
	auto list = AAEdit::g_cardData.GetOverrideList();
	for (const auto& it : list) {
		std::string listEntry(it.first);
		listEntry += ListSeperator + it.second;
		SendMessage(this->m_lbOverrides, LB_ADDSTRING, 0, (LPARAM)listEntry.c_str());
	}
}

/*
 * Refresh the known textures in the combobox
 * based on the global mesh texture list (for now, more textures might follow)
 */
void UnlimitedDialog::RefreshTextureList() {
	//mesh textures: global location is AA2Edit.exe+353290+1408 , size is 1024 (in entrys)
	SendMessage(this->m_cbOverride, CB_RESETCONTENT, 0, 0);
	TextureStruct** arrTex = (TextureStruct**)(General::GameBase + 0x353290 + 0x1408);
	for (int i = 0; i < 1024; i++) {
		if (arrTex[i] != NULL) {
			SendMessage(this->m_cbOverride, CB_ADDSTRING, 0, (LPARAM)(arrTex[i]->m_name));
		}
	}
}

/*
 * Dialog proc. Does everything that the dialog does as a response to clicks and user actions.
 */
INT_PTR CALLBACK UnlimitedDialog::DialogProc(_In_ HWND hwndDlg, _In_ UINT msg, 
											 _In_ WPARAM wparam, _In_ LPARAM lparam) 
{
	switch (msg) {
	case WM_INITDIALOG: {
		//initialize dialog members from the loaded dialog
		UnlimitedDialog* thisPtr = (UnlimitedDialog*)lparam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lparam); //register class to this hwnd
		thisPtr->m_cbOverride = GetDlgItem(hwndDlg, IDC_COMBOFROM);
		thisPtr->m_edOverrideWith = GetDlgItem(hwndDlg, IDC_EDITOVERRIDE);
		thisPtr->m_lbOverrides = GetDlgItem(hwndDlg, IDC_LIST1);

		thisPtr->RefreshRuleList();
		return TRUE;
		break; }
	case WM_VKEYTOITEM: {
		//DEL-key was pressed while the list box had the focus. our target is to remove
		//the selected override rule.
		UnlimitedDialog* thisPtr = (UnlimitedDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		if (LOWORD(wparam) == VK_DELETE) {
			//get current selection text
			DWORD sel = SendMessage(thisPtr->m_lbOverrides, LB_GETCURSEL, 0, 0);
			if (sel == LB_ERR) return TRUE; //no selection
			int textLength = SendMessage(thisPtr->m_lbOverrides, LB_GETTEXTLEN, sel, 0) + 1;
			char* tempBuffer = new char[textLength];
			SendMessage(thisPtr->m_lbOverrides, LB_GETTEXT, sel, (LPARAM)tempBuffer);
			//split text into texture and override
			char* seperatorPtr = strstr(tempBuffer, ListSeperator);
			*seperatorPtr = '\0';
			char* rest = seperatorPtr + sizeof(ListSeperator) - 1;
			//remove this rule
			g_cardData.RemoveOverride(tempBuffer, rest);
			delete[] tempBuffer;
			thisPtr->RefreshRuleList();
			return TRUE;
		}
		break; }
	case WM_COMMAND: {
		UnlimitedDialog* thisPtr = (UnlimitedDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		switch (HIWORD(wparam)) {
		case CBN_DROPDOWN:
			//dropdown menu opened, refresh known textures
			thisPtr->RefreshTextureList();
			return TRUE;
			break;
		case BN_CLICKED: {
			DWORD identifier = LOWORD(wparam);
			if (identifier == IDC_BUTTONBROWSE) {
				//TODO: implement browse button. should be put into the aaedit/data/texture/override folder.
				std::string initialDir = General::BuildEditPath(OVERRIDE_IMAGE_PATH, NULL);
				const char* choice = General::OpenFileDialog(initialDir.c_str());
				if (choice != NULL) {
					if (General::StartsWith(choice, initialDir.c_str())) {
						const char* rest = choice + initialDir.size();
						SendMessage(thisPtr->m_edOverrideWith, WM_SETTEXT, 0, (LPARAM)rest);
					}
				}
				return TRUE;
			}
			else if (identifier == IDC_BUTTONAPPLY) {
				//apply button pressed.
				std::string override, toOverride;
				char buffer[1024];
				SendMessage(thisPtr->m_cbOverride, WM_GETTEXT, 1024, (LPARAM)buffer);
				override = buffer;
				SendMessage(thisPtr->m_edOverrideWith, WM_GETTEXT, 1024, (LPARAM)buffer);
				toOverride = buffer;
				if(AAEdit::g_cardData.AddOverride(override.c_str(), toOverride.c_str())) {
					thisPtr->RefreshRuleList();
				}
				return TRUE;
			}
			break; }
		}
		break; }
	}
	return FALSE;
}

UnlimitedDialog g_AAUnlimitDialog;

//note that we cant get WM_INITDIALOG here, because the injection checks the HWNDs against 
//the global hwnd array, but that one is only set after the WM_INITDIALOG has been processed.
LRESULT __stdcall SystemDialogNotification(void* internclass, HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (!g_AAUnlimitDialog.IsVisible()) {
		g_AAUnlimitDialog.Initialize();
	}
	
	return 0;
}

}