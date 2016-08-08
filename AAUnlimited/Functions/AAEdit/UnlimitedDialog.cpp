#include "UnlimitedDialog.h"

#include <Windows.h>
#include <CommCtrl.h>

#include "External\ExternalVariables\AAEdit\WindowData.h"
#include "External\ExternalClasses\TextureStruct.h"
#include "General\ModuleInfo.h"
#include "General\Util.h"
#include "Functions\AAEdit\Globals.h"
#include "resource.h"
#include "config.h"


namespace AAEdit {

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
		CreateDialogParam(General::DllInst, MAKEINTRESOURCE(IDD_AAUPROPDIALOG),
			NULL /*ExtVars::AAEdit::MainWnd*/, MainDialogProc, (LPARAM)this);
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

void UnlimitedDialog::Refresh() {
	int index = TabCtrl_GetCurSel(m_tabs);
	switch (index) {
	case 0:
		m_moDialog.Refresh();
		break;
	case 1:
		m_aoDialog.Refresh();
		break;
	case 2:
		m_arDialog.Refresh();
		break;
	case 3:
		m_etDialog.Refresh();
		break;
	case 4:
		m_hrDialog.Refresh();
	default:
		break;
	}
}

void UnlimitedDialog::AddDialog(int resourceName, Dialog* dialog, int tabN, const TCHAR* tabName,
								RECT rct, INT_PTR(CALLBACK *dialogProc)(HWND, UINT, WPARAM, LPARAM)) {
	CreateDialogParam(General::DllInst, MAKEINTRESOURCE(resourceName),
		this->m_tabs, dialogProc, (LPARAM)dialog);
	MoveWindow(dialog->m_dialog, rct.left, rct.top, rct.right - rct.left, rct.bottom - rct.top, FALSE);
	TCITEM item;
	item.mask = TCIF_TEXT | TCIF_PARAM;
	item.pszText = (LPWSTR)tabName;
	item.lParam = (LPARAM)dialog;
	TabCtrl_InsertItem(this->m_tabs, tabN, &item);
}

/******
* Dialog Proc for the Main Dialog (the tabs, essentially)
******/
INT_PTR CALLBACK UnlimitedDialog::MainDialogProc(_In_ HWND hwndDlg, _In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	switch (msg) {
	case WM_INITDIALOG: {
		UnlimitedDialog* thisPtr = (UnlimitedDialog*)lparam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lparam); //register class to this hwnd
		thisPtr->m_dialog = hwndDlg;
		thisPtr->m_tabs = GetDlgItem(hwndDlg, IDC_TABS);
		
		RECT rct;
		GetWindowRect(hwndDlg, &rct);
		rct.top += 20; rct.right -= 20;
		rct.left += 10; rct.left -= 10;
		TabCtrl_AdjustRect(thisPtr->m_tabs, FALSE, &rct);

		thisPtr->AddDialog(IDD_MESHOVERRIDE, &thisPtr->m_moDialog, 0, TEXT("Mesh Overrides"),
				rct, MODialog::DialogProc);
		thisPtr->AddDialog(IDD_ARCHIVEOVERRIDE, &thisPtr->m_aoDialog, 1, TEXT("Archive Overrides"),
			rct, AODialog::DialogProc);
		thisPtr->AddDialog(IDD_ARCHIVEREDIRECT, &thisPtr->m_arDialog, 2, TEXT("Archive Redirects"),
			rct, ARDialog::DialogProc);
		thisPtr->AddDialog(IDD_EYETEXTURE, &thisPtr->m_etDialog, 3, TEXT("Eye Textures"),
			rct, ETDialog::DialogProc);

		ShowWindow(thisPtr->m_moDialog.m_dialog, SW_SHOW);
		return TRUE;
		break; }
	case WM_NOTIFY: {
		UnlimitedDialog* thisPtr = (UnlimitedDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		NMHDR* param = (NMHDR*)(lparam);
		DWORD code = param->code;
		if (param->hwndFrom == thisPtr->m_tabs) {
			if (code == TCN_SELCHANGING) {
				//hide current dialog
				Dialog* currDialog = (Dialog*)(thisPtr->GetCurrTabItemData());
				if (currDialog != NULL) {
					currDialog->Show(false);
				}
				return TRUE;
			}
			else if (code == TCN_SELCHANGE) {
				//show new dialog
				Dialog* currDialog = (Dialog*)(thisPtr->GetCurrTabItemData());
				if (currDialog != NULL) {
					currDialog->Show(true);
					currDialog->Refresh();
				}
				return TRUE;
			}
		}
		break; }
	}
	return FALSE;
}

LPARAM UnlimitedDialog::GetCurrTabItemData() {
	int currSel = TabCtrl_GetCurSel(this->m_tabs);
	if (currSel == -1) return NULL;
	TCITEM item;
	item.mask = TCIF_PARAM;
	BOOL suc = TabCtrl_GetItem(this->m_tabs, currSel, &item);
	if (suc == FALSE) return NULL;
	return item.lParam;
}

/************************/
/* Mesh Override Dialog */
/************************/


/*
 * Dialog proc for the Mesh Texture Override Dialog
 */
INT_PTR CALLBACK UnlimitedDialog::MODialog::DialogProc(_In_ HWND hwndDlg, _In_ UINT msg,
	_In_ WPARAM wparam, _In_ LPARAM lparam)
{
	switch (msg) {
	case WM_INITDIALOG: {
		//initialize dialog members from the loaded dialog
		MODialog* thisPtr = (MODialog*)lparam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lparam); //register class to this hwnd
		thisPtr->m_dialog = hwndDlg;
		thisPtr->m_cbOverride = GetDlgItem(hwndDlg, IDC_MO_COMBOFROM);
		thisPtr->m_edOverrideWith = GetDlgItem(hwndDlg, IDC_MO_EDITTO);
		thisPtr->m_lbOverrides = GetDlgItem(hwndDlg, IDC_MO_LIST);

		thisPtr->RefreshRuleList();
		return TRUE;
		break; }
	case WM_VKEYTOITEM: {
		//DEL-key was pressed while the list box had the focus. our target is to remove
		//the selected override rule.
		MODialog* thisPtr = (MODialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		if (LOWORD(wparam) == VK_DELETE) {
			//get current selection text
			int sel = SendMessage(thisPtr->m_lbOverrides, LB_GETCURSEL, 0, 0);
			//remove this rule
			g_cardData.RemoveMeshOverride(sel);
			thisPtr->RefreshRuleList();
			return TRUE;
		}
		break; }
	case WM_COMMAND: {
		MODialog* thisPtr = (MODialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		switch (HIWORD(wparam)) {
		case CBN_DROPDOWN:
			//dropdown menu opened, refresh known textures
			thisPtr->RefreshTextureList();
			return TRUE;
			break;
		case BN_CLICKED: {
			DWORD identifier = LOWORD(wparam);
			if (identifier == IDC_MO_BTNBROWSE) {
				//TODO: implement browse button. should be put into the aaedit/data/texture/override folder.
				std::wstring initialDir = General::BuildEditPath(OVERRIDE_IMAGE_PATH, NULL);
				if (!General::DirExists(initialDir.c_str())) {
					CreateDirectory(initialDir.c_str(), NULL);
				}
				const TCHAR* choice = General::OpenFileDialog(initialDir.c_str());
				if (choice != NULL) {
					if (General::StartsWith(choice, initialDir.c_str())) {
						const TCHAR* rest = choice + initialDir.size();
						SendMessage(thisPtr->m_edOverrideWith, WM_SETTEXT, 0, (LPARAM)rest);
					}
				}
				return TRUE;
			}
			else if (identifier == IDC_MO_BTNAPPLY) {
				//apply button pressed.
				std::wstring override, toOverride;
				TCHAR buffer[1024];
				SendMessage(thisPtr->m_cbOverride, WM_GETTEXT, 1024, (LPARAM)buffer);
				override = buffer;
				SendMessage(thisPtr->m_edOverrideWith, WM_GETTEXT, 1024, (LPARAM)buffer);
				toOverride = buffer;
				if (AAEdit::g_cardData.AddMeshOverride(override.c_str(), toOverride.c_str())) {
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

/*
 * Refreshes the rule list based on the override rules set
 * in the global g_cardData struct
 */
void UnlimitedDialog::MODialog::RefreshRuleList() {
	SendMessage(this->m_lbOverrides, LB_RESETCONTENT, 0, 0);
	auto list = AAEdit::g_cardData.GetMeshOverrideList();
	for (size_t i = 0; i < list.size(); i++) {
		std::wstring listEntry(list[i].first);
		listEntry += TEXT(" -> ") + list[i].second;
		SendMessage(this->m_lbOverrides, LB_INSERTSTRING, i, (LPARAM)listEntry.c_str());
	}
}

/*
 * Refresh the known textures in the combobox
 * based on the global mesh texture list (for now, more textures might follow)
 */
void UnlimitedDialog::MODialog::RefreshTextureList() {
	//mesh textures: global location is AA2Edit.exe+353290+1408 , size is 1024 (in entrys)
	SendMessage(this->m_cbOverride, CB_RESETCONTENT, 0, 0);
	TextureStruct** arrTex = (TextureStruct**)(General::GameBase + 0x353290 + 0x1408);
	for (int i = 0; i < 1024; i++) {
		if (arrTex[i] != NULL) {
			TCHAR buffer[256];
			size_t out;
			mbstowcs_s(&out, buffer, arrTex[i]->m_name, 256);
			SendMessage(this->m_cbOverride, CB_ADDSTRING, 0, (LPARAM)(buffer));
		}
	}
}

void UnlimitedDialog::MODialog::Refresh() {
	RefreshRuleList();
	RefreshTextureList();
}


/***************************/
/* Archive Override Dialog */
/***************************/

INT_PTR CALLBACK UnlimitedDialog::AODialog::DialogProc(_In_ HWND hwndDlg, _In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	switch (msg) {
	case WM_INITDIALOG: {
		//initialize dialog members from the loaded dialog
		AODialog* thisPtr = (AODialog*)lparam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lparam); //register class to this hwnd
		thisPtr->m_dialog = hwndDlg;
		thisPtr->m_edArchive = GetDlgItem(hwndDlg, IDC_AO_EDARCHIVE);
		thisPtr->m_edArchiveFile = GetDlgItem(hwndDlg, IDC_AO_EDARCHIVEFILE);
		thisPtr->m_edOverrideFile = GetDlgItem(hwndDlg, IDC_AO_EDFILE);
		thisPtr->m_btApply = GetDlgItem(hwndDlg, IDC_AO_BTNAPPLY);
		thisPtr->m_btBrowse = GetDlgItem(hwndDlg, IDC_AO_BTNBROWSE);
		thisPtr->m_lbOverrides = GetDlgItem(hwndDlg, IDC_AO_LIST);

		thisPtr->RefreshRuleList();
		return TRUE;
		break; }
	case WM_VKEYTOITEM: {
		//DEL-key was pressed while the list box had the focus. our target is to remove
		//the selected override rule.
		AODialog* thisPtr = (AODialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		if (LOWORD(wparam) == VK_DELETE) {
			//get current selection text
			int sel = SendMessage(thisPtr->m_lbOverrides, LB_GETCURSEL, 0, 0);
			//remove this rule
			g_cardData.RemoveArchiveOverride(sel);
			thisPtr->RefreshRuleList();
			return TRUE;
		}
		break; }
	case WM_COMMAND: {
		AODialog* thisPtr = (AODialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		switch (HIWORD(wparam)) {
		case BN_CLICKED: {
			DWORD identifier = LOWORD(wparam);
			if (identifier == IDC_AO_BTNBROWSE) {
				std::wstring initialPlayDir = General::BuildPlayPath(OVERRIDE_ARCHIVE_PATH, NULL);
				std::wstring initialEditDir = General::BuildEditPath(OVERRIDE_ARCHIVE_PATH, NULL);
				const TCHAR* choice = General::OpenFileDialog(initialEditDir.c_str());
				if (choice != NULL) {
					if (General::StartsWith(choice, initialPlayDir.c_str())) {
						const TCHAR* rest = choice + initialPlayDir.size();
						SendMessage(thisPtr->m_edOverrideFile, WM_SETTEXT, 0, (LPARAM)rest);
					}
					else if (General::StartsWith(choice, initialEditDir.c_str())) {
						const TCHAR* rest = choice + initialEditDir.size();
						SendMessage(thisPtr->m_edOverrideFile, WM_SETTEXT, 0, (LPARAM)rest);
					}
				}
				return TRUE;
			}
			else if (identifier == IDC_AO_BTNAPPLY) {
				//apply button pressed.
				std::wstring archive, archivefile, toOverride;
				TCHAR buffer[1024];
				SendMessage(thisPtr->m_edArchive, WM_GETTEXT, 1024, (LPARAM)buffer);
				archive = buffer;
				SendMessage(thisPtr->m_edArchiveFile, WM_GETTEXT, 1024, (LPARAM)buffer);
				archivefile = buffer;
				SendMessage(thisPtr->m_edOverrideFile, WM_GETTEXT, 1024, (LPARAM)buffer);
				toOverride = buffer;
				if (AAEdit::g_cardData.AddArchiveOverride(archive.c_str(), archivefile.c_str(), toOverride.c_str())) {
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

void UnlimitedDialog::AODialog::RefreshRuleList() {
	SendMessage(this->m_lbOverrides, LB_RESETCONTENT, 0, 0);
	auto list = AAEdit::g_cardData.GetArchiveOverrideList();
	for (size_t i = 0; i < list.size(); i++) {
		std::wstring listEntry(TEXT("["));
		listEntry += list[i].first.first + TEXT("/") + list[i].first.second + TEXT("] -> ");
		listEntry += list[i].second;
		SendMessage(this->m_lbOverrides, LB_INSERTSTRING, i, (LPARAM)listEntry.c_str());
	}
}

void UnlimitedDialog::AODialog::Refresh() {
	RefreshRuleList();
}


/***************************/
/* Archive Redirect Dialog */
/***************************/

INT_PTR CALLBACK UnlimitedDialog::ARDialog::DialogProc(_In_ HWND hwndDlg, _In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	switch (msg) {
	case WM_INITDIALOG: {
		//initialize dialog members from the loaded dialog
		ARDialog* thisPtr = (ARDialog*)lparam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lparam); //register class to this hwnd
		thisPtr->m_dialog = hwndDlg;
		thisPtr->m_edArFrom = GetDlgItem(hwndDlg, IDC_AR_EDARCHFROM);
		thisPtr->m_edFileFrom = GetDlgItem(hwndDlg, IDC_AR_EDFILEFROM);
		thisPtr->m_edArTo = GetDlgItem(hwndDlg, IDC_AR_EDARCHTO);
		thisPtr->m_edFileTo = GetDlgItem(hwndDlg, IDC_AR_EDFILETO);
		thisPtr->m_btApply = GetDlgItem(hwndDlg, IDC_AR_BTNAPPLY);
		thisPtr->m_lbOverrides = GetDlgItem(hwndDlg, IDC_AR_LIST);

		thisPtr->RefreshRuleList();
		return TRUE;
		break; }
	case WM_VKEYTOITEM: {
		//DEL-key was pressed while the list box had the focus. our target is to remove
		//the selected override rule.
		ARDialog* thisPtr = (ARDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		if (LOWORD(wparam) == VK_DELETE) {
			//get current selection text
			int sel = SendMessage(thisPtr->m_lbOverrides, LB_GETCURSEL, 0, 0);
			//remove this rule
			g_cardData.RemoveArchiveRedirect(sel);
			thisPtr->RefreshRuleList();
			return TRUE;
		}
		break; }
	case WM_COMMAND: {
		ARDialog* thisPtr = (ARDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		switch (HIWORD(wparam)) {
		case BN_CLICKED: {
			DWORD identifier = LOWORD(wparam);
			if (identifier == IDC_AR_BTNAPPLY) {
				//apply button pressed.
				std::wstring archive, archivefile, overrArchive, overrFile;
				TCHAR buffer[1024];
				SendMessage(thisPtr->m_edArFrom, WM_GETTEXT, 1024, (LPARAM)buffer);
				archive = buffer;
				SendMessage(thisPtr->m_edFileFrom, WM_GETTEXT, 1024, (LPARAM)buffer);
				archivefile = buffer;
				SendMessage(thisPtr->m_edArTo, WM_GETTEXT, 1024, (LPARAM)buffer);
				overrArchive = buffer;
				SendMessage(thisPtr->m_edFileTo, WM_GETTEXT, 1024, (LPARAM)buffer);
				overrFile = buffer;
				
				if (AAEdit::g_cardData.AddArchiveRedirect(archive.c_str(), archivefile.c_str(),
						overrArchive.c_str(), overrFile.c_str())) {
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

void UnlimitedDialog::ARDialog::RefreshRuleList() {
	SendMessage(this->m_lbOverrides, LB_RESETCONTENT, 0, 0);
	auto list = AAEdit::g_cardData.GetArchiveRedirectList();
	for (size_t i = 0; i < list.size(); i++) {
		std::wstring listEntry(TEXT("["));
		listEntry += list[i].first.first + TEXT("/") + list[i].first.second + TEXT("] -> [");
		listEntry += list[i].second.first + TEXT("/") + list[i].second.second + TEXT("]");
		SendMessage(this->m_lbOverrides, LB_INSERTSTRING, i, (LPARAM)listEntry.c_str());
	}
}

void UnlimitedDialog::ARDialog::Refresh() {
	RefreshRuleList();
}

/**********************/
/* Eye Texture Dialog */
/**********************/


INT_PTR CALLBACK UnlimitedDialog::ETDialog::DialogProc(_In_ HWND hwndDlg, _In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	switch (msg) {
	case WM_INITDIALOG: {
		//initialize dialog members from the loaded dialog
		ETDialog* thisPtr = (ETDialog*)lparam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lparam); //register class to this hwnd
		thisPtr->m_dialog = hwndDlg;
		thisPtr->m_eyes[0].cbActive = GetDlgItem(hwndDlg, IDC_ET_LEFT_CBUSE);
		thisPtr->m_eyes[0].cbSaveInside = GetDlgItem(hwndDlg, IDC_ET_LEFT_CBSAVEINSIDE);
		thisPtr->m_eyes[0].edFile = GetDlgItem(hwndDlg, IDC_ET_LEFT_EDTEX);
		thisPtr->m_eyes[0].btBrowse = GetDlgItem(hwndDlg, IDC_ET_LEFT_BTBROWSE);
		thisPtr->m_eyes[1].cbActive = GetDlgItem(hwndDlg, IDC_ET_RIGHT_CBUSE);
		thisPtr->m_eyes[1].cbSaveInside = GetDlgItem(hwndDlg, IDC_ET_RIGHT_CBSAVEINSIDE);
		thisPtr->m_eyes[1].edFile = GetDlgItem(hwndDlg, IDC_ET_RIGHT_EDTEX);
		thisPtr->m_eyes[1].btBrowse = GetDlgItem(hwndDlg, IDC_ET_RIGHT_BTBROWSE);
		thisPtr->RefreshEnableState();

		return TRUE;
		break; }
	case WM_COMMAND: {
		ETDialog* thisPtr = (ETDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		switch (HIWORD(wparam)) {
		case BN_CLICKED: {
			DWORD identifier = LOWORD(wparam);
			if (identifier == IDC_ET_LEFT_CBUSE || identifier == IDC_ET_RIGHT_CBUSE) {
				thisPtr->RefreshEnableState();
				return TRUE;
			}
			else if (identifier == IDC_ET_LEFT_BTBROWSE || identifier == IDC_ET_RIGHT_BTBROWSE) {
				std::wstring initDir = General::BuildEditPath(TEXT("data\\texture\\eye"));
				const TCHAR* path = General::OpenFileDialog(initDir.c_str());
				if (path != NULL && General::StartsWith(path, initDir.c_str())) {
					const TCHAR* fileName = General::FindFileInPath(path);
					int index = identifier == IDC_ET_LEFT_BTBROWSE ? 0 : 1;
					SendMessage(thisPtr->m_eyes[index].edFile, WM_SETTEXT, 0, (LPARAM)fileName);
				}
				return TRUE;
			}
			break; }
		case EN_UPDATE: {
			DWORD identifier = LOWORD(wparam);
			int index = identifier == IDC_ET_LEFT_EDTEX ? 0 : 1;
			TCHAR fileName[512];
			fileName[0] = '\0';
			SendMessage((HWND)lparam, WM_GETTEXT, 512, (LPARAM)fileName);
			bool save = SendMessage(thisPtr->m_eyes[index].cbSaveInside, BM_GETCHECK, 0, 0) == BST_CHECKED;
			g_cardData.SetEyeTexture(index, fileName, save);
			break; }
		}
		break; }
	}
	return FALSE;
}

void UnlimitedDialog::ETDialog::RefreshEnableState() {
	BOOL state = (SendMessage(this->m_eyes[0].cbActive, BM_GETCHECK, 0, 0) == BST_CHECKED) ? TRUE : FALSE;
	EnableWindow(this->m_eyes[0].cbSaveInside, state);
	EnableWindow(this->m_eyes[0].edFile, state);
	EnableWindow(this->m_eyes[0].btBrowse, state);
	if (state == FALSE) {
		g_cardData.SetEyeTexture(0, NULL, false);
	}

	state = (SendMessage(this->m_eyes[1].cbActive, BM_GETCHECK, 0, 0) == BST_CHECKED) ? TRUE : FALSE;
	EnableWindow(this->m_eyes[1].cbSaveInside, state);
	EnableWindow(this->m_eyes[1].edFile, state);
	EnableWindow(this->m_eyes[1].btBrowse, state);
	if (state == FALSE) {
		g_cardData.SetEyeTexture(1, NULL, false);
	}
}

void UnlimitedDialog::ETDialog::Refresh() {
	const std::wstring& leftPath = g_cardData.GetEyeTexture(0);
	const std::wstring& rightPath = g_cardData.GetEyeTexture(1);
	SendMessage(this->m_eyes[0].edFile, WM_SETTEXT, 0, (LPARAM)leftPath.c_str());
	SendMessage(this->m_eyes[1].edFile, WM_SETTEXT, 0, (LPARAM)leftPath.c_str());
}

/************************/
/* Hair Redirect Dialog */
/************************/


INT_PTR CALLBACK UnlimitedDialog::HRDialog::DialogProc(_In_ HWND hwndDlg, _In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	switch (msg) {
	case WM_INITDIALOG: {
		//initialize dialog members from the loaded dialog
		HRDialog* thisPtr = (HRDialog*)lparam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lparam); //register class to this hwnd
		thisPtr->m_dialog = hwndDlg;
		thisPtr->m_arrRbRedirects[0][0] = GetDlgItem(hwndDlg, IDC_HR_RADIO_00);
		thisPtr->m_arrRbRedirects[0][1] = GetDlgItem(hwndDlg, IDC_HR_RADIO_01);
		thisPtr->m_arrRbRedirects[0][2] = GetDlgItem(hwndDlg, IDC_HR_RADIO_02);
		thisPtr->m_arrRbRedirects[0][3] = GetDlgItem(hwndDlg, IDC_HR_RADIO_03);
		thisPtr->m_arrRbRedirects[1][0] = GetDlgItem(hwndDlg, IDC_HR_RADIO_10);
		thisPtr->m_arrRbRedirects[1][1] = GetDlgItem(hwndDlg, IDC_HR_RADIO_11);
		thisPtr->m_arrRbRedirects[1][2] = GetDlgItem(hwndDlg, IDC_HR_RADIO_12);
		thisPtr->m_arrRbRedirects[1][3] = GetDlgItem(hwndDlg, IDC_HR_RADIO_13);
		thisPtr->m_arrRbRedirects[2][0] = GetDlgItem(hwndDlg, IDC_HR_RADIO_20);
		thisPtr->m_arrRbRedirects[2][1] = GetDlgItem(hwndDlg, IDC_HR_RADIO_21);
		thisPtr->m_arrRbRedirects[2][2] = GetDlgItem(hwndDlg, IDC_HR_RADIO_22);
		thisPtr->m_arrRbRedirects[2][3] = GetDlgItem(hwndDlg, IDC_HR_RADIO_23);
		thisPtr->m_arrRbRedirects[3][0] = GetDlgItem(hwndDlg, IDC_HR_RADIO_30);
		thisPtr->m_arrRbRedirects[3][1] = GetDlgItem(hwndDlg, IDC_HR_RADIO_31);
		thisPtr->m_arrRbRedirects[3][2] = GetDlgItem(hwndDlg, IDC_HR_RADIO_32);
		thisPtr->m_arrRbRedirects[3][3] = GetDlgItem(hwndDlg, IDC_HR_RADIO_33);
		SendMessage(thisPtr->m_arrRbRedirects[0][0], BM_SETCHECK, BST_CHECKED, 0);
		SendMessage(thisPtr->m_arrRbRedirects[1][1], BM_SETCHECK, BST_CHECKED, 0);
		SendMessage(thisPtr->m_arrRbRedirects[2][2], BM_SETCHECK, BST_CHECKED, 0);
		SendMessage(thisPtr->m_arrRbRedirects[3][3], BM_SETCHECK, BST_CHECKED, 0);
		return TRUE;
		break; }
	}
	return FALSE;
}

BYTE UnlimitedDialog::HRDialog::GetHairTarget(BYTE hairCategory) {
	for (BYTE i = 0; i < 4; i++) {
		if (SendMessage(this->m_arrRbRedirects[hairCategory][i], BM_GETCHECK, 0, 0) == BST_CHECKED) {
			return i;
		}
	}
	return hairCategory; //fallback
}

void UnlimitedDialog::HRDialog::Refresh() {
	for (BYTE i = 0; i < 4; i++) {
		BYTE n = g_cardData.GetHairRedirect(i);
		if (n > 3) n = i;
		SendMessage(this->m_arrRbRedirects[i][n], BM_CLICK, BST_CHECKED, 0);
	}
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