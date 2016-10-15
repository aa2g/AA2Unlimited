#include "UnlimitedDialog.h"

#include <Windows.h>
#include <CommCtrl.h>
#include <queue>

#include "External\ExternalVariables\AAEdit\WindowData.h"
#include "External\ExternalClasses\TextureStruct.h"
#include "External\ExternalVariables\AAEdit\CardData.h"
#include "General\ModuleInfo.h"
#include "General\Util.h"
#include "Functions\AAEdit\Globals.h"
#include "Files\Logger.h"
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
	TCITEM item;
	item.mask = TCIF_PARAM;
	BOOL suc = TabCtrl_GetItem(m_tabs, index, &item);
	if (suc == FALSE) return;
	Dialog* diag = (Dialog*)(item.lParam);
	diag->Refresh();
}

void UnlimitedDialog::AddDialog(int resourceName, Dialog* dialog, int tabN, const TCHAR* tabName,
								RECT rct, INT_PTR(CALLBACK *dialogProc)(HWND, UINT, WPARAM, LPARAM)) {
	HWND res = CreateDialogParam(General::DllInst, MAKEINTRESOURCE(resourceName),
		this->m_tabs, dialogProc, (LPARAM)dialog);
	if(res == NULL) {
		int error = GetLastError();
		LOGPRIO(Logger::Priority::WARN) << "failed to make dialog " << resourceName << ": " << error << "\r\n";
	}
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

		int index = 0;
		thisPtr->AddDialog(IDD_GENERAL,&thisPtr->m_gnDialog,index++,TEXT("General"),
			rct,GNDialog::DialogProc);
		thisPtr->AddDialog(IDD_EYETEXTURE, &thisPtr->m_etDialog,index++, TEXT("Eye Textures"),
			rct, ETDialog::DialogProc);
		thisPtr->AddDialog(IDD_TANSELECT, &thisPtr->m_tsDialog,index++, TEXT("Tan"),
			rct, TSDialog::DialogProc);
		thisPtr->AddDialog(IDD_HAIR, &thisPtr->m_hrDialog,index++, TEXT("Hair"),
			rct, HRDialog::DialogProc);
		thisPtr->AddDialog(IDD_MESHOVERRIDE, &thisPtr->m_moDialog,index++, TEXT("Mesh Overrides"),
			rct, MODialog::DialogProc);
		thisPtr->AddDialog(IDD_ARCHIVEOVERRIDE, &thisPtr->m_aoDialog,index++, TEXT("Archive Overrides"),
			rct, AODialog::DialogProc);
		thisPtr->AddDialog(IDD_ARCHIVEREDIRECT, &thisPtr->m_arDialog,index++, TEXT("Archive Redirects"),
			rct, ARDialog::DialogProc);
		thisPtr->AddDialog(IDD_BODY,&thisPtr->m_bdDialog,index++,TEXT("Body"),
			rct, BDDialog::DialogProc);


		TabCtrl_SetCurSel(thisPtr->m_tabs,0);

		thisPtr->Refresh();
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


/***************************/
/* General Override Dialog */
/***************************/

INT_PTR CALLBACK UnlimitedDialog::GNDialog::DialogProc(_In_ HWND hwndDlg,_In_ UINT msg,
	_In_ WPARAM wparam,_In_ LPARAM lparam)
{
	switch (msg) {
	case WM_INITDIALOG: {
		//initialize dialog members from the loaded dialog
		GNDialog* thisPtr = (GNDialog*)lparam;
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lparam); //register class to this hwnd
		thisPtr->m_dialog = hwndDlg;
		thisPtr->m_cbSaveFiles = GetDlgItem(hwndDlg,IDC_GN_CBSAVEFILES);

		return TRUE;
		break; }
	case WM_COMMAND: {
		GNDialog* thisPtr = (GNDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		
		break; }
	}
	return FALSE;
}

void UnlimitedDialog::GNDialog::Refresh() {

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
			g_currChar.m_cardData.RemoveMeshOverride(sel);
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
				if (AAEdit::g_currChar.m_cardData.AddMeshOverride(override.c_str(), toOverride.c_str())) {
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
	auto list = AAEdit::g_currChar.m_cardData.GetMeshOverrideList();
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
	ExtClass::TextureStruct** arrTex = (ExtClass::TextureStruct**)(General::GameBase + 0x353290 + 0x1408);
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
		if (thisPtr == NULL) return FALSE;
		if (LOWORD(wparam) == VK_DELETE) {
			//get current selection text
			int sel = SendMessage(thisPtr->m_lbOverrides, LB_GETCURSEL, 0, 0);
			//remove this rule
			g_currChar.m_cardData.RemoveArchiveOverride(sel);
			thisPtr->RefreshRuleList();
			return TRUE;
		}
		break; }
	case WM_COMMAND: {
		AODialog* thisPtr = (AODialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		if (thisPtr == NULL) return FALSE;
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
				if (AAEdit::g_currChar.m_cardData.AddArchiveOverride(archive.c_str(), archivefile.c_str(), toOverride.c_str())) {
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
	auto list = AAEdit::g_currChar.m_cardData.GetArchiveOverrideList();
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
			g_currChar.m_cardData.RemoveArchiveRedirect(sel);
			thisPtr->RefreshRuleList();
			return TRUE;
		}
		break; }
	case WM_COMMAND: {
		ARDialog* thisPtr = (ARDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		if (thisPtr == NULL) return FALSE;
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
				
				if (AAEdit::g_currChar.m_cardData.AddArchiveRedirect(archive.c_str(), archivefile.c_str(),
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
	auto list = AAEdit::g_currChar.m_cardData.GetArchiveRedirectList();
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
		if (thisPtr == NULL) return FALSE;
		switch (HIWORD(wparam)) {
		case BN_CLICKED: {
			DWORD identifier = LOWORD(wparam);
			if (identifier == IDC_ET_LEFT_CBUSE || identifier == IDC_ET_RIGHT_CBUSE) {
				thisPtr->RefreshEnableState();
				return TRUE;
			}
			else if (identifier == IDC_ET_LEFT_CBSAVEINSIDE || identifier == IDC_ET_RIGHT_CBSAVEINSIDE) {
				int index = identifier == IDC_ET_LEFT_CBSAVEINSIDE ? 0 : 1;
				TCHAR fileName[512];
				fileName[0] = '\0';
				SendMessage(thisPtr->m_eyes[index].edFile, WM_GETTEXT, 512, (LPARAM)fileName);
				bool save = SendMessage((HWND)lparam, BM_GETCHECK, 0, 0) == BST_CHECKED;
				g_currChar.m_cardData.SetEyeTexture(index, fileName, save);
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
			g_currChar.m_cardData.SetEyeTexture(index, fileName, save);
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
		g_currChar.m_cardData.SetEyeTexture(0, NULL, false);
	}

	state = (SendMessage(this->m_eyes[1].cbActive, BM_GETCHECK, 0, 0) == BST_CHECKED) ? TRUE : FALSE;
	EnableWindow(this->m_eyes[1].cbSaveInside, state);
	EnableWindow(this->m_eyes[1].edFile, state);
	EnableWindow(this->m_eyes[1].btBrowse, state);
	if (state == FALSE) {
		g_currChar.m_cardData.SetEyeTexture(1, NULL, false);
	}
}

void UnlimitedDialog::ETDialog::Refresh() {
	const std::wstring& leftPath = g_currChar.m_cardData.GetEyeTexture(0);
	const std::wstring& rightPath = g_currChar.m_cardData.GetEyeTexture(1);
	SendMessage(this->m_eyes[0].edFile, WM_SETTEXT, 0, (LPARAM)leftPath.c_str());
	SendMessage(this->m_eyes[1].edFile, WM_SETTEXT, 0, (LPARAM)leftPath.c_str());
}

/*********************/
/* Tan Select Dialog */
/*********************/

INT_PTR CALLBACK UnlimitedDialog::TSDialog::DialogProc(_In_ HWND hwndDlg, _In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	switch (msg) {
	case WM_INITDIALOG: {
		//initialize dialog members from the loaded dialog
		TSDialog* thisPtr = (TSDialog*)lparam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lparam); //register class to this hwnd
		thisPtr->m_dialog = hwndDlg;
		thisPtr->m_cbSelect = GetDlgItem(hwndDlg, IDC_TS_CBSELECTION);
		
		thisPtr->LoadTanList();

		return TRUE;
		break; }
	case WM_COMMAND: {
		TSDialog* thisPtr = (TSDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		if (thisPtr == NULL) return FALSE;
		switch (HIWORD(wparam)) {
		case CBN_SELCHANGE: {
			int sel = SendMessage(thisPtr->m_cbSelect, CB_GETCURSEL, 0, 0);
			TCHAR name[256];
			name[0] = '\0';
			SendMessage(thisPtr->m_cbSelect, CB_GETLBTEXT, sel, (LPARAM)name);
			g_currChar.m_cardData.SetTan(name);
			break; }
		}
		break; }
	}
	return FALSE;
}

void UnlimitedDialog::TSDialog::LoadTanList() {
	//always need a none field
	SendMessage(m_cbSelect, CB_ADDSTRING, 0, (LPARAM)TEXT("-- None --")); 

	//list all tan directories
	std::wstring tanDirectory = General::BuildEditPath(TAN_PATH, TEXT("*"));
	if (!General::DirExists(tanDirectory.c_str())) {
		CreateDirectory(tanDirectory.c_str(), NULL);
	}
	WIN32_FIND_DATA data;
	HANDLE hSearch = FindFirstFile(tanDirectory.c_str(), &data);

	const std::wstring& currentTan = g_currChar.m_cardData.GetTanName();
	bool containsTan = false;
	static const TCHAR* exceptionNames[] {TEXT("."), TEXT("..")};
	if (hSearch != INVALID_HANDLE_VALUE) {
		BOOL suc = FALSE;
		do {
			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				bool allowed = true;
				for (const TCHAR* name : exceptionNames) {
					if (wcscmp(data.cFileName, name) == 0) {
						allowed = false;
						break;
					}
				}
				if (allowed) {
					SendMessage(m_cbSelect, CB_ADDSTRING, 0, (LPARAM)data.cFileName);
					if (currentTan == data.cFileName) containsTan = true;
				}
			}
			suc = FindNextFile(hSearch, &data);
		} while (suc != FALSE);
		FindClose(hSearch);
	}

	//if the tan doesnt exists, at it anyway so we dont loose the data
	if (!containsTan && currentTan.size() > 0) {
		SendMessage(m_cbSelect, CB_ADDSTRING, 0, (LPARAM)currentTan.c_str());
	}
	
	//select tan in combobox as default, or none if no tan is set
	if (currentTan.size() > 0) {
		SendMessage(m_cbSelect, CB_SELECTSTRING, -1, (LPARAM)currentTan.c_str());
	}
	else {
		SendMessage(m_cbSelect, CB_SELECTSTRING, -1, (LPARAM)TEXT("-- None --"));
	}
}

void UnlimitedDialog::TSDialog::Refresh() {
	std::wstring name = g_currChar.m_cardData.GetTanName();
	if(SendMessage(m_cbSelect,CB_SELECTSTRING,-1,(LPARAM)name.c_str()) == CB_ERR) {
		//we dont have this tan
		SendMessage(m_cbSelect,CB_SELECTSTRING,-1,(LPARAM)TEXT("-- None --"));
	}
}

/***************/
/* Hair Dialog */
/***************/


INT_PTR CALLBACK UnlimitedDialog::HRDialog::DialogProc(_In_ HWND hwndDlg, _In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	switch (msg) {
	case WM_INITDIALOG: {
		//initialize dialog members from the loaded dialog
		HRDialog* thisPtr = (HRDialog*)lparam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lparam); //register class to this hwnd
		thisPtr->m_dialog = hwndDlg;
		thisPtr->m_rbKind[0] = GetDlgItem(hwndDlg,IDC_HR_RBFRONT);
		thisPtr->m_rbKind[1] = GetDlgItem(hwndDlg,IDC_HR_RBSIDE);
		thisPtr->m_rbKind[2] = GetDlgItem(hwndDlg,IDC_HR_RBBACK);
		thisPtr->m_rbKind[3] = GetDlgItem(hwndDlg,IDC_HR_RBEXT);
		SendMessage(thisPtr->m_rbKind[0], BM_SETCHECK, BST_CHECKED, 0);
		thisPtr->m_edSlot = GetDlgItem(hwndDlg,IDC_HR_EDSLOT);
		thisPtr->m_edAdjustment = GetDlgItem(hwndDlg,IDC_HR_EDADJUSTMENT);
		thisPtr->m_cbFlip = GetDlgItem(hwndDlg,IDC_HR_CBFLIP);
		thisPtr->m_edHighlight = GetDlgItem(hwndDlg, IDC_HR_EDHIGHLIGHT);
		thisPtr->m_lstHairs = GetDlgItem(hwndDlg,IDC_HR_LIST);

		LVCOLUMN column;
		column.mask = LVCF_TEXT | LVCF_WIDTH;
		column.cx = 50;
		column.pszText = TEXT("Hair Kind");
		ListView_InsertColumn(thisPtr->m_lstHairs,0,&column);
		column.pszText = TEXT("Slot");
		ListView_InsertColumn(thisPtr->m_lstHairs,1,&column);
		column.pszText = TEXT("Adjustment");
		ListView_InsertColumn(thisPtr->m_lstHairs,2,&column);
		column.pszText = TEXT("Flip");
		ListView_InsertColumn(thisPtr->m_lstHairs,3,&column);

		return TRUE;
		break; }
	case WM_COMMAND: {
		HRDialog* thisPtr = (HRDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		if (thisPtr == NULL) return FALSE;
		switch (HIWORD(wparam)) {
		case BN_CLICKED: {
			DWORD identifier = LOWORD(wparam);
			if (identifier == IDC_HR_BTNBROWSE) {
				std::wstring initialDir = General::BuildEditPath(HAIR_HIGHLIGHT_PATH, NULL);
				if (!General::DirExists(initialDir.c_str())) {
					CreateDirectory(initialDir.c_str(), NULL);
				}
				const TCHAR* choice = General::OpenFileDialog(initialDir.c_str());
				if (choice != NULL) {
					if (General::StartsWith(choice, initialDir.c_str())) {
						const TCHAR* rest = choice + initialDir.size();
						SendMessage(thisPtr->m_edHighlight, WM_SETTEXT, 0, (LPARAM)rest);
						g_currChar.m_cardData.SetHairHighlight(rest);
					}
				}
				return TRUE;
			}
			else if(identifier == IDC_HR_BTNADD) {
				BYTE kind;
				for(kind = 0; kind < 4; kind++) {
					if (SendMessage(thisPtr->m_rbKind[kind],BM_GETCHECK,0,0) == BST_CHECKED) break;
				}
				if (kind == 4) kind = 0;
				TCHAR buf[256];
				SendMessage(thisPtr->m_edSlot,WM_GETTEXT,256,(LPARAM)buf);
				BYTE slot = _wtoi(buf);
				SendMessage(thisPtr->m_edAdjustment,WM_GETTEXT,256,(LPARAM)buf);
				BYTE adjustment = _wtoi(buf);
				bool flip = SendMessage(thisPtr->m_cbFlip,BM_GETCHECK,0,0) == BST_CHECKED;
				g_currChar.m_cardData.AddHair(kind,slot,adjustment,flip);
				thisPtr->Refresh();
				return TRUE;
			}
			
			break; }
		}
		break; }
	case WM_NOTIFY: {
		HRDialog* thisPtr = (HRDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		if (thisPtr == NULL) return FALSE;
		NMHDR* mi = (NMHDR*)lparam;
		switch (mi->code) {
		case LVN_KEYDOWN:
			if(mi->idFrom == IDC_HR_LIST) {
				NMLVKEYDOWN* param = (NMLVKEYDOWN*)mi;
				if(param->wVKey == VK_DELETE) {
					//delete key was pressed in list
					int sel = SendMessage(thisPtr->m_lstHairs,LVM_GETSELECTIONMARK,0,0);
					if (sel == -1) return TRUE;
					g_currChar.m_cardData.RemoveHair(sel);
					thisPtr->Refresh();
					return TRUE;
				}
			}
			break;
		}
		break; }
						
	}
	return FALSE;
}

void UnlimitedDialog::HRDialog::RefreshHairList() {
	ListView_DeleteAllItems(m_lstHairs);
	int n = 0; //element
	for(int kind = 0; kind < 4; kind++) {
		const auto& list = g_currChar.m_cardData.GetHairs(kind);
		LVITEM item;
		item.mask = LVIF_TEXT;
		for (int i = 0; i < list.size(); i++) {
			//hair kind
			switch (list[i].kind) {
			case 0:
				item.pszText = TEXT("Front Hair");
				break;
			case 1:
				item.pszText = TEXT("Side Hair");
				break;
			case 2:
				item.pszText = TEXT("Back Hair");
				break;
			case 3:
				item.pszText = TEXT("Hair Extension");
				break;
			default:
				item.pszText = TEXT("Error");
				break;
			}
			item.iItem = n++;
			item.iSubItem = 0;
			ListView_InsertItem(m_lstHairs,&item);
			//hair slot
			TCHAR buf[256];
			_itow_s(list[i].slot,buf,10);
			item.iSubItem = 1;
			item.pszText = buf;
			ListView_SetItem(m_lstHairs,&item);
			//hair adjustment
			_itow_s(list[i].adjustment,buf,10);
			item.iSubItem = 2;
			item.pszText = buf;
			ListView_SetItem(m_lstHairs,&item);
			//flip
			if (list[i].flip) item.pszText = TEXT("true");
			else item.pszText = TEXT("false");
			item.iSubItem = 3;
			ListView_SetItem(m_lstHairs,&item);
		}
	}
}


void UnlimitedDialog::HRDialog::Refresh() {
	RefreshHairList();
	const std::wstring& name = g_currChar.m_cardData.GetHairHighlightName();
	SendMessage(m_edHighlight,WM_SETTEXT,0,(LPARAM)name.c_str());
}

/***************/
/* Body Dialog */
/***************/

INT_PTR CALLBACK UnlimitedDialog::BDDialog::DialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam) {
	switch (msg) {
	case WM_INITDIALOG: {
		BDDialog* thisPtr = (BDDialog*)lparam;
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lparam); //register class to this hwnd
		thisPtr->m_dialog = hwndDlg;
		thisPtr->m_cbOutlineColor = GetDlgItem(hwndDlg,IDC_BD_CBOUTLINECOLOR);
		thisPtr->m_edOutlineColorRed = GetDlgItem(hwndDlg,IDC_BD_EDOUTLINECOLOR_RED);
		thisPtr->m_edOutlineColorGreen = GetDlgItem(hwndDlg,IDC_BD_EDOUTLINECOLOR_GREEN);
		thisPtr->m_edOutlineColorBlue = GetDlgItem(hwndDlg,IDC_BD_EDOUTLINECOLOR_BLUE);

		thisPtr->m_bmBtnAdd = GetDlgItem(hwndDlg,IDC_BD_BM_BTNADD);
		thisPtr->m_bmCbSelect = GetDlgItem(hwndDlg,IDC_BD_BM_CBSELECT);
		thisPtr->m_bmList = GetDlgItem(hwndDlg,IDC_BD_BM_LIST);
		thisPtr->m_bmSldWidth = GetDlgItem(hwndDlg,IDC_BD_BM_SLDWIDTH);
		thisPtr->m_bmSldHeight = GetDlgItem(hwndDlg,IDC_BD_BM_SLDHEIGHT);
		thisPtr->m_bmEdMatrix[0][0] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR11);
		thisPtr->m_bmEdMatrix[0][1] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR12);
		thisPtr->m_bmEdMatrix[0][2] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR13);
		thisPtr->m_bmEdMatrix[0][3] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR14);
		thisPtr->m_bmEdMatrix[1][0] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR21);
		thisPtr->m_bmEdMatrix[1][1] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR22);
		thisPtr->m_bmEdMatrix[1][2] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR23);
		thisPtr->m_bmEdMatrix[1][3] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR24);
		thisPtr->m_bmEdMatrix[2][0] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR31);
		thisPtr->m_bmEdMatrix[2][1] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR32);
		thisPtr->m_bmEdMatrix[2][2] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR33);
		thisPtr->m_bmEdMatrix[2][3] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR34);
		thisPtr->m_bmEdMatrix[3][0] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR41);
		thisPtr->m_bmEdMatrix[3][1] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR42);
		thisPtr->m_bmEdMatrix[3][2] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR43);
		thisPtr->m_bmEdMatrix[3][3] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR44);

		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				SendMessage(thisPtr->m_bmEdMatrix[i][j],WM_SETTEXT,0,(LPARAM)TEXT("0"));
			}
		}
		for (int i = 0; i < 4; i++) {
			SendMessage(thisPtr->m_bmEdMatrix[i][i],WM_SETTEXT,0,(LPARAM)TEXT("1"));
		}
		
		SendMessage(GetDlgItem(hwndDlg,IDC_BD_SPINRED),UDM_SETRANGE,0,MAKELPARAM(255,0));
		SendMessage(GetDlgItem(hwndDlg,IDC_BD_SPINGREEN),UDM_SETRANGE,0,MAKELPARAM(255,0));
		SendMessage(GetDlgItem(hwndDlg,IDC_BD_SPINBLUE),UDM_SETRANGE,0,MAKELPARAM(255,0));
		return TRUE; }
	case WM_VKEYTOITEM: {
		//DEL-key was pressed while the list box had the focus.
		BDDialog* thisPtr = (BDDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		if (thisPtr == NULL) return FALSE;
		if (LOWORD(wparam) == VK_DELETE) {
			//get current selection text
			int sel = SendMessage(thisPtr->m_bmList,LB_GETCURSEL,0,0);
			if (sel == LB_ERR) return TRUE;
			//remove this rule
			g_currChar.m_cardData.RemoveBoneTransformation(sel);
			thisPtr->Refresh();
			return TRUE;
		}
		break; }
	case WM_COMMAND: {
		BDDialog* thisPtr = (BDDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		if (thisPtr == NULL) return FALSE;
		if (thisPtr == NULL) return FALSE;
		switch (HIWORD(wparam)) {
		case BN_CLICKED: {
			DWORD identifier = LOWORD(wparam);
			if(identifier == IDC_BD_CBOUTLINECOLOR) {
				BOOL visible = SendMessage(thisPtr->m_cbOutlineColor,BM_GETCHECK,0,0) == BST_CHECKED;
				g_currChar.m_cardData.SetHasOutlineColor(visible == TRUE);
				thisPtr->Refresh();
				return TRUE;
			}
			else if(identifier == IDC_BD_BM_BTNADD) {
				thisPtr->ApplyInput();
				return TRUE;
			}
			break; }
		case EN_CHANGE: {
			HWND ed = (HWND)lparam;
			if(ed == thisPtr->m_edOutlineColorBlue
				|| ed == thisPtr->m_edOutlineColorGreen
				|| ed == thisPtr->m_edOutlineColorRed) 
			{
				int newval = General::GetEditInt(ed);
				if (newval < 0) {
					SendMessage(ed,WM_SETTEXT,0,(LPARAM)TEXT("0"));
				}
				else if (newval > 255) {
					SendMessage(ed,WM_SETTEXT,0,(LPARAM)TEXT("255"));
				}
				else {
					int red = General::GetEditInt(thisPtr->m_edOutlineColorRed);
					int green = General::GetEditInt(thisPtr->m_edOutlineColorGreen);
					int blue = General::GetEditInt(thisPtr->m_edOutlineColorBlue);
					g_currChar.m_cardData.SetOutlineColor(RGB(red,green,blue));
				}
			}
			else {
				//make sure its a valid float
				TCHAR num[128];
				SendMessage(ed,WM_GETTEXT,128,(LPARAM)num);
				float f = wcstof(num,NULL); //returns 0 on errornous string, so invalid stuff will just turn to a 0
				std::wstring str = std::to_wstring(f);
				//if the value was changed, take the new one
				if(str != num) {
					SendMessage(ed,WM_SETTEXT,0,(LPARAM)str.c_str());
				}
			}
			
			return TRUE; }
		case LBN_SELCHANGE: {
			int sel = SendMessage(thisPtr->m_bmList,LB_GETCURSEL,0,0);
			thisPtr->LoadData(sel);
			return TRUE; }
		};
		break; }
	};
	return FALSE;
}

void UnlimitedDialog::BDDialog::LoadData(int listboxId) {
	const auto& vec = g_currChar.m_cardData.GetBoneTransformationList();
	const auto& rule = vec[listboxId];
	//combo box with title
	SendMessage(m_bmCbSelect,WM_SETTEXT,0,(LPARAM)rule.first.c_str());
	//the matrix
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			std::wstring num = std::to_wstring(rule.second.m[i][j]);
			SendMessage(m_bmEdMatrix[i][j],WM_SETTEXT,0,(LPARAM)num.c_str());
		}
	}
	//
}

void UnlimitedDialog::BDDialog::ApplyInput() {
	TCHAR name[128];
	SendMessage(m_bmCbSelect,WM_GETTEXT,128,(LPARAM)name);
	//remove transformation if it allready exists
	const auto& vec = g_currChar.m_cardData.GetBoneTransformationList();
	int match;
	for(match = 0; match < vec.size(); match++) {
		if (vec[match].first == name) break;
	}
	if (match < vec.size()) g_currChar.m_cardData.RemoveBoneTransformation(match);

	//get matrix
	D3DMATRIX m;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			TCHAR num[128];
			SendMessage(m_bmEdMatrix[i][j],WM_GETTEXT,128,(LPARAM)num);
			float f = wcstof(num, NULL);
			m.m[i][j] = f;
		}
	}
	//save
	g_currChar.m_cardData.AddBoneTransformation(name,m);
	Refresh();
}

void UnlimitedDialog::BDDialog::Refresh() {
	bool bOutline = g_currChar.m_cardData.HasOutlineColor();
	
	EnableWindow(this->m_edOutlineColorRed,bOutline);
	EnableWindow(this->m_edOutlineColorGreen,bOutline);
	EnableWindow(this->m_edOutlineColorBlue,bOutline);

	COLORREF color = g_currChar.m_cardData.GetOutlineColor();
	TCHAR text[10];
	_itow_s(GetRValue(color),text,10);
	SendMessage(this->m_edOutlineColorRed,WM_SETTEXT,0,(LPARAM)text);
	_itow_s(GetGValue(color),text,10);
	SendMessage(this->m_edOutlineColorGreen,WM_SETTEXT,0,(LPARAM)text);
	_itow_s(GetBValue(color),text,10);
	SendMessage(this->m_edOutlineColorBlue,WM_SETTEXT,0,(LPARAM)text);

	//listbox
	SendMessage(this->m_bmList,LB_RESETCONTENT,0,0);
	const auto& list = AAEdit::g_currChar.m_cardData.GetBoneTransformationList();
	for (size_t i = 0; i < list.size(); i++) {
		std::wstring listEntry(list[i].first);
		SendMessage(this->m_bmList,LB_INSERTSTRING,i,(LPARAM)listEntry.c_str());
	}

	//list possible bones
	ExtClass::CharacterStruct* curr = ExtVars::AAEdit::GetCurrentCharacter();
	if(curr != NULL) {
		ExtClass::XXFile* xxlist[] = {
			curr->m_xxFace, curr->m_xxGlasses, curr->m_xxFrontHair, curr->m_xxSideHair,
			curr->m_xxBackHair, curr->m_xxHairExtension, curr->m_xxTounge, curr->m_xxSkeleton,
			curr->m_xxBody, curr->m_xxLegs
		};
		TCHAR tmpBuff[256];
		std::queue<ExtClass::Bone*> fileQueue;
		for (ExtClass::XXFile* file : xxlist) {
			if (file == NULL) continue;
			ExtClass::Bone* root = file->m_root;
			fileQueue.push(root);
			while (!fileQueue.empty()) {
				ExtClass::Bone* bone = fileQueue.front();
				fileQueue.pop();
				size_t conv;
				mbstowcs_s(&conv,tmpBuff,bone->m_name,bone->m_nameBufferSize);
				SendMessage(m_bmCbSelect,CB_ADDSTRING,0,(LPARAM)tmpBuff);
				for (int i = 0; i < bone->m_arrSize; i++) {
					fileQueue.push(bone->m_boneArray + i);
				}
			}
		}
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