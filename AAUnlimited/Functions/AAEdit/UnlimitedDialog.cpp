#include "UnlimitedDialog.h"

/*
 * TODO: break up this file. every single tab is defined in this file. its impossible to read if you dont know what youre searching for,
 * and even then its pretty spaghetti
 */

#include <Windows.h>
#include <CommCtrl.h>
#include <queue>
#include <set>
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "External\ExternalVariables\AAEdit\WindowData.h"
#include "External\ExternalClasses\TextureStruct.h"
#include "External\ExternalVariables\AAEdit\CardData.h"
#include "External\ExternalVariables\AAEdit\RefreshTable.h"
#include "External\AddressRule.h"
#include "General\ModuleInfo.h"
#include "General\Util.h"
#include "Functions\AAEdit\Globals.h"
#include "Functions\Shared\Globals.h"
#include "Functions\Shared\Overrides.h"
#include "Functions\Shared\Triggers\Triggers.h"
#include "Functions\Shared\Triggers\Expressions.h"
#include "Files\Logger.h"
#include "resource.h"
#include "config.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"") 

namespace {
	static HFONT GetSysFont() {
		NONCLIENTMETRICS ncm;
		ncm.cbSize = sizeof(ncm);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof(NONCLIENTMETRICS),&ncm,0);
		return CreateFontIndirect(&(ncm.lfMenuFont));
	}
	HFONT sysFont = GetSysFont();
}

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
								INT_PTR(CALLBACK *dialogProc)(HWND, UINT, WPARAM, LPARAM)) {
	HWND res = CreateDialogParam(General::DllInst, MAKEINTRESOURCE(resourceName),
		this->m_tabs, dialogProc, (LPARAM)dialog);
	if(res == NULL) {
		int error = GetLastError();
		LOGPRIO(Logger::Priority::WARN) << "failed to make dialog " << resourceName << ": " << error << "\r\n";
	}

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

		int index = 0;
		thisPtr->AddDialog(IDD_GENERAL,&thisPtr->m_gnDialog,index++,TEXT("General"),
			GNDialog::DialogProc);
		thisPtr->AddDialog(IDD_EYETEXTURE, &thisPtr->m_etDialog,index++, TEXT("Eye Textures"),
			ETDialog::DialogProc);
		thisPtr->AddDialog(IDD_TANSELECT, &thisPtr->m_tsDialog,index++, TEXT("Tan"),
			TSDialog::DialogProc);
		thisPtr->AddDialog(IDD_HAIR, &thisPtr->m_hrDialog,index++, TEXT("Hair"),
			HRDialog::DialogProc);
		thisPtr->AddDialog(IDD_MESHOVERRIDE, &thisPtr->m_moDialog,index++, TEXT("Mesh Overrides"),
			MODialog::DialogProc);
		thisPtr->AddDialog(IDD_ARCHIVEOVERRIDE, &thisPtr->m_aoDialog,index++, TEXT("Archive Overrides"),
			AODialog::DialogProc);
		thisPtr->AddDialog(IDD_ARCHIVEREDIRECT, &thisPtr->m_arDialog,index++, TEXT("Archive Redirects"),
			ARDialog::DialogProc);
		thisPtr->AddDialog(IDD_OBJECTOVERRIDE,&thisPtr->m_ooDialog,index++,TEXT("Object Overrides"),
			OODialog::DialogProc);
		thisPtr->AddDialog(IDD_BODY,&thisPtr->m_bdDialog,index++,TEXT("Body"),
			BDDialog::DialogProc);
		thisPtr->AddDialog(IDD_BODYSLIDER,&thisPtr->m_bsDialog,index++,TEXT("Body Slider"),
			BSDialog::DialogProc);
		thisPtr->AddDialog(IDD_TRIGGERS,&thisPtr->m_trDialog,index++,TEXT("Triggers"),
			TRDialog::DialogProc);
		thisPtr->AddDialog(IDD_MODULES,&thisPtr->m_mdDialog,index++,TEXT("Modules"),
			MDDialog::DialogProc);

		int count = TabCtrl_GetItemCount(thisPtr->m_tabs);
		RECT rct;
		GetClientRect(thisPtr->m_tabs,&rct);
		TabCtrl_AdjustRect(thisPtr->m_tabs,FALSE,&rct);
		for(int i = 0; i < count; i++) {
			TCITEM item;
			item.mask = TCIF_PARAM;
			BOOL suc = TabCtrl_GetItem(thisPtr->m_tabs,i,&item);
			if (suc == FALSE) continue;
			Dialog* diag = (Dialog*)(item.lParam);

			MoveWindow(diag->m_dialog,rct.left,rct.top,rct.right - rct.left,rct.bottom - rct.top,FALSE);
		}

		DWORD charDataRule[] { 0x353254, 0x2C, 0};
		AAEdit::g_currChar.m_char = (ExtClass::CharacterStruct*) ExtVars::ApplyRule(charDataRule);

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
		thisPtr->m_cbSaveEyeTexture = GetDlgItem(hwndDlg,IDC_GN_CBSAVEEYETEX);
		thisPtr->m_cbSaveEyeHighlight = GetDlgItem(hwndDlg,IDC_GN_CBSAVEEYEHI);
		thisPtr->m_lbAAuSets = GetDlgItem(hwndDlg,IDC_GN_LBAAUSETS);
		thisPtr->m_btnAAuSetAdd = GetDlgItem(hwndDlg,IDC_GN_BTNAAUSETADD);
		thisPtr->m_edAAuSetName = GetDlgItem(hwndDlg,IDC_GN_EDAAUSETNAME);

		return TRUE;
		break; }
	case WM_VKEYTOITEM: {
		//DEL-key was pressed while the list box had the focus
		GNDialog* thisPtr = (GNDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		if (LOWORD(wparam) == VK_DELETE) {
			//get current selection text
			int sel = SendMessage(thisPtr->m_lbAAuSets,LB_GETCURSEL,0,0);
			//remove this rule
			g_currChar.m_cardData.RemoveAAUDataSet(sel);
			thisPtr->RefreshAAuSetList();
			return TRUE;
		}
		break; }
	
	case WM_COMMAND: {
		GNDialog* thisPtr = (GNDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		switch(HIWORD(wparam)) {
		case BN_CLICKED: {
			DWORD identifier = LOWORD(wparam);
			if(identifier == IDC_GN_BTNAAUSETADD) {
				TCHAR buf[256];
				SendMessage(thisPtr->m_edAAuSetName,WM_GETTEXT,256,(LPARAM)&buf);
				g_currChar.m_cardData.CopyAAUDataSet(buf);
				thisPtr->RefreshAAuSetList();
				return TRUE;
			}
			break; }
		case LBN_SELCHANGE: {
			GNDialog* thisPtr = (GNDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
			HWND wnd = (HWND)lparam;
			if (wnd == thisPtr->m_lbAAuSets) {
				int sel = SendMessage(thisPtr->m_lbAAuSets,LB_GETCURSEL,0,0);
				if (sel != LB_ERR) {
					AAEdit::g_currChar.m_cardData.SwitchActiveAAUDataSet(sel);
					using namespace ExtVars::AAEdit;
					RedrawBodyPart(Category::FIGURE,RedrawId::FIGURE_HEIGHT);
				}
				return TRUE;
			}
			break; }
		}
		break; }
	}
	return FALSE;
}

void UnlimitedDialog::GNDialog::RefreshAAuSetList() {
	SendMessage(this->m_lbAAuSets,LB_RESETCONTENT,0,0);
	auto list = AAEdit::g_currChar.m_cardData.GetAAUSetDataList();
	for (size_t i = 0; i < list.size(); i++) {
		SendMessage(this->m_lbAAuSets,LB_INSERTSTRING,i,(LPARAM)list[i].c_str());
	}
	SendMessage(this->m_lbAAuSets,LB_SETCURSEL,AAEdit::g_currChar.m_cardData.GetCurrAAUSet(),0);
}

void UnlimitedDialog::GNDialog::Refresh() {
	RefreshAAuSetList();
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
				std::wstring initialDir = General::BuildEditPath(OVERRIDE_PATH, NULL);
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
				std::wstring initialEditDir = General::BuildOverridePath(NULL);
				const TCHAR* choice = General::OpenFileDialog(initialEditDir.c_str());
				if (choice != NULL) {
					if (General::StartsWith(choice, initialEditDir.c_str())) {
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
		case LBN_DBLCLK: {
			DWORD identifier = LOWORD(wparam);
			if (identifier == IDC_AO_LIST) {
				LRESULT res = SendMessage(thisPtr->m_lbOverrides, LB_GETCURSEL, 0, 0);
				if (res != LB_ERR) {
					auto list = AAEdit::g_currChar.m_cardData.GetArchiveOverrideList();
					auto override = list.at(res);
					SetWindowText(thisPtr->m_edArchive, override.first.first.c_str());
					SetWindowText(thisPtr->m_edArchiveFile, override.first.second.c_str());
					SetWindowText(thisPtr->m_edOverrideFile, override.second.c_str());
				}
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
		case LBN_DBLCLK: {
			DWORD identifier = LOWORD(wparam);
			if (identifier == IDC_AR_LIST) {
				LRESULT res = SendMessage(thisPtr->m_lbOverrides, LB_GETCURSEL, 0, 0);
				if (res != LB_ERR) {
					auto list = AAEdit::g_currChar.m_cardData.GetArchiveRedirectList();
					auto override = list.at(res);
					SetWindowText(thisPtr->m_edArFrom, override.first.first.c_str());
					SetWindowText(thisPtr->m_edFileFrom, override.first.second.c_str());
					SetWindowText(thisPtr->m_edArTo, override.second.first.c_str());
					SetWindowText(thisPtr->m_edFileTo, override.second.second.c_str());
				}
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

/**************************/
/* Object Override Dialog */
/**************************/

INT_PTR CALLBACK UnlimitedDialog::OODialog::DialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam) {
	switch (msg) {
	case WM_INITDIALOG: {
		//initialize dialog members from the loaded dialog
		OODialog* thisPtr = (OODialog*)lparam;
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lparam); //register class to this hwnd
		thisPtr->m_dialog = hwndDlg;
		thisPtr->m_edFile = GetDlgItem(hwndDlg,IDC_OO_EDFILE);
		thisPtr->m_edObject = GetDlgItem(hwndDlg,IDC_OO_EDOBJECT);
		thisPtr->m_btnApply = GetDlgItem(hwndDlg,IDC_OO_BTNAPPLY);
		thisPtr->m_btnBrowse = GetDlgItem(hwndDlg,IDC_OO_BTNBROWSE);
		thisPtr->m_lbOverrides = GetDlgItem(hwndDlg,IDC_OO_LIST);

		thisPtr->RefreshRuleList();
		return TRUE;
		break; }
	case WM_VKEYTOITEM: {
		//DEL-key was pressed while the list box had the focus. our target is to remove
		//the selected override rule.
		OODialog* thisPtr = (OODialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		if (LOWORD(wparam) == VK_DELETE) {
			//get current selection text
			int sel = SendMessage(thisPtr->m_lbOverrides,LB_GETCURSEL,0,0);
			//remove this rule
			g_currChar.m_cardData.RemoveObjectOverride(sel);
			thisPtr->RefreshRuleList();
			return TRUE;
		}
		break; }
	case WM_COMMAND: {
		OODialog* thisPtr = (OODialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		if (thisPtr == NULL) return FALSE;
		switch (HIWORD(wparam)) {
		case BN_CLICKED: {
			DWORD identifier = LOWORD(wparam);
			if (identifier == IDC_OO_BTNBROWSE) {
				std::wstring initialPlayDir = General::BuildPlayPath(OVERRIDE_PATH,NULL);
				std::wstring initialEditDir = General::BuildEditPath(OVERRIDE_PATH,NULL);
				const TCHAR* choice = General::OpenFileDialog(initialEditDir.c_str());
				if (choice != NULL) {
					/*if (General::StartsWith(choice,initialPlayDir.c_str())) {
						const TCHAR* rest = choice + initialPlayDir.size();
						SendMessage(thisPtr->m_edFile,WM_SETTEXT,0,(LPARAM)rest);
					}*/
					if (General::StartsWith(choice,initialEditDir.c_str())) {
						const TCHAR* rest = choice + initialEditDir.size();
						SendMessage(thisPtr->m_edFile,WM_SETTEXT,0,(LPARAM)rest);
					}
				}
				return TRUE;
			}
			else if (identifier == IDC_OO_BTNAPPLY) {
				//apply button pressed.
				std::wstring object,file;
				TCHAR buffer[1024];
				SendMessage(thisPtr->m_edObject,WM_GETTEXT,1024,(LPARAM)buffer);
				object = buffer;
				SendMessage(thisPtr->m_edFile,WM_GETTEXT,1024,(LPARAM)buffer);
				file = buffer;

				if (AAEdit::g_currChar.m_cardData.AddObjectOverride(object.c_str(),file.c_str())) {
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

void UnlimitedDialog::OODialog::RefreshRuleList() {
	SendMessage(this->m_lbOverrides,LB_RESETCONTENT,0,0);
	auto list = AAEdit::g_currChar.m_cardData.GetObjectOverrideList();
	for (size_t i = 0; i < list.size(); i++) {
		std::wstring listEntry(TEXT("["));
		listEntry += list[i].first + TEXT("] -> [");
		listEntry += list[i].second + TEXT("]");
		SendMessage(this->m_lbOverrides,LB_INSERTSTRING,i,(LPARAM)listEntry.c_str());
	}
}

void UnlimitedDialog::OODialog::Refresh() {
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
		thisPtr->m_eye.cbActive = GetDlgItem(hwndDlg, IDC_ET_RIGHT_CBUSE);
		thisPtr->m_eye.edFile = GetDlgItem(hwndDlg, IDC_ET_RIGHT_EDTEX);
		thisPtr->m_eye.btBrowse = GetDlgItem(hwndDlg, IDC_ET_RIGHT_BTBROWSE);
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
			else if (identifier == IDC_ET_RIGHT_CBSAVEINSIDE) {
				TCHAR fileName[512];
				fileName[0] = '\0';
				SendMessage(thisPtr->m_eye.edFile, WM_GETTEXT, 512, (LPARAM)fileName);
				bool save = SendMessage((HWND)lparam, BM_GETCHECK, 0, 0) == BST_CHECKED;
				g_currChar.m_cardData.SetEyeTexture(1, fileName, save);
			}
			else if (identifier == IDC_ET_RIGHT_BTBROWSE) {
				std::wstring initDir = General::BuildEditPath(TEXT("data\\texture\\eye"));
				const TCHAR* path = General::OpenFileDialog(initDir.c_str());
				if (path != NULL && General::StartsWith(path, initDir.c_str())) {
					const TCHAR* fileName = General::FindFileInPath(path);
					int index = identifier == IDC_ET_LEFT_BTBROWSE ? 0 : 1;
					SendMessage(thisPtr->m_eye.edFile, WM_SETTEXT, 0, (LPARAM)fileName);
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
			g_currChar.m_cardData.SetEyeTexture(index, fileName, false);
			ExtVars::AAEdit::RedrawBodyPart(ExtVars::AAEdit::Category::EYES,ExtVars::AAEdit::RedrawId::EYES_ALL);
			break; }
		}
		break; }
	}
	return FALSE;
} 

void UnlimitedDialog::ETDialog::RefreshEnableState() {
	BOOL state = (SendMessage(this->m_eye.cbActive, BM_GETCHECK, 0, 0) == BST_CHECKED) ? TRUE : FALSE;
	EnableWindow(this->m_eye.edFile, state);
	EnableWindow(this->m_eye.btBrowse, state);
	if (state == FALSE) {
		g_currChar.m_cardData.SetEyeTexture(1, NULL, false);
	}
}

void UnlimitedDialog::ETDialog::Refresh() {
	const std::wstring& rightPath = g_currChar.m_cardData.GetEyeTexture(1);
	SendMessage(this->m_eye.edFile, WM_SETTEXT, 0, (LPARAM)rightPath.c_str());
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
			//redraw tan
			ExtVars::AAEdit::RedrawBodyPart(ExtVars::AAEdit::BODY_COLOR, ExtVars::AAEdit::BODYCOLOR_TAN);
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
	std::wstring tanDirectory = General::BuildOverridePath(TAN_PATH, TEXT("*"));
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
				std::wstring initialDir = General::BuildOverridePath(HAIR_HIGHLIGHT_PATH, NULL);
				if (!General::DirExists(initialDir.c_str())) {
					CreateDirectory(initialDir.c_str(), NULL);
				}
				const TCHAR* choice = General::OpenFileDialog(initialDir.c_str());
				if (choice != NULL) {
					if (General::StartsWith(choice, initialDir.c_str())) {
						const TCHAR* rest = choice + initialDir.size();
						SendMessage(thisPtr->m_edHighlight, WM_SETTEXT, 0, (LPARAM)rest);
						g_currChar.m_cardData.SetHairHighlight(rest);
						//redraw all hairs
						using namespace ExtVars::AAEdit;
						RedrawBodyPart(HAIR,HAIR_BACK);
						RedrawBodyPart(HAIR,HAIR_FRONT);
						RedrawBodyPart(HAIR,HAIR_SIDE);
						RedrawBodyPart(HAIR,HAIR_EXTENSION);
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
				//redraw hair of added king
				using namespace ExtVars::AAEdit;
				if(kind == 0) RedrawBodyPart(HAIR,HAIR_FRONT);
				else if(kind == 1) RedrawBodyPart(HAIR,HAIR_SIDE);
				else if(kind == 2) RedrawBodyPart(HAIR,HAIR_BACK);
				else if(kind == 3) RedrawBodyPart(HAIR,HAIR_EXTENSION);
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
		for (unsigned int i = 0; i < list.size(); i++) {
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

		thisPtr->m_cbTanColor = GetDlgItem(hwndDlg,IDC_BD_CBTANCOLOR);
		thisPtr->m_edTanColorRed = GetDlgItem(hwndDlg,IDC_BD_EDTANCOLOR_RED);
		thisPtr->m_edTanColorGreen = GetDlgItem(hwndDlg,IDC_BD_EDTANCOLOR_GREEN);
		thisPtr->m_edTanColorBlue = GetDlgItem(hwndDlg,IDC_BD_EDTANCOLOR_BLUE);

		thisPtr->m_bmBtnAdd = GetDlgItem(hwndDlg,IDC_BD_BM_BTNADD);
		thisPtr->m_bmCbXXFile = GetDlgItem(hwndDlg,IDC_BD_BM_CBXXFILE);
		thisPtr->m_bmCbBone = GetDlgItem(hwndDlg,IDC_BD_BM_CBBONE);
		thisPtr->m_bmList = GetDlgItem(hwndDlg,IDC_BD_BM_LIST);
		thisPtr->m_bmEdMatrix[0][0] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR11);
		thisPtr->m_bmEdMatrix[0][1] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR12);
		thisPtr->m_bmEdMatrix[0][2] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR13);
		thisPtr->m_bmEdMatrix[1][0] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR21);
		thisPtr->m_bmEdMatrix[1][1] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR22);
		thisPtr->m_bmEdMatrix[1][2] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR23);
		thisPtr->m_bmEdMatrix[2][0] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR31);
		thisPtr->m_bmEdMatrix[2][1] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR32);
		thisPtr->m_bmEdMatrix[2][2] = GetDlgItem(hwndDlg,IDC_BD_BM_EDMATR33);
		thisPtr->m_bmRbFrameMod = GetDlgItem(hwndDlg,IDC_BD_BM_RBFRAME);
		thisPtr->m_bmRbBoneMod = GetDlgItem(hwndDlg,IDC_BD_BM_RBBONE);

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				SendMessage(thisPtr->m_bmEdMatrix[i][j],WM_SETTEXT,0,(LPARAM)TEXT("0"));
			}
		}
		
		SendMessage(GetDlgItem(hwndDlg,IDC_BD_SPINRED),UDM_SETRANGE,0,MAKELPARAM(255,0));
		SendMessage(GetDlgItem(hwndDlg,IDC_BD_SPINGREEN),UDM_SETRANGE,0,MAKELPARAM(255,0));
		SendMessage(GetDlgItem(hwndDlg,IDC_BD_SPINBLUE),UDM_SETRANGE,0,MAKELPARAM(255,0));

		SendMessage(GetDlgItem(hwndDlg,IDC_BD_SPINREDTAN),UDM_SETRANGE,0,MAKELPARAM(255,0));
		SendMessage(GetDlgItem(hwndDlg,IDC_BD_SPINGREENTAN),UDM_SETRANGE,0,MAKELPARAM(255,0));
		SendMessage(GetDlgItem(hwndDlg,IDC_BD_SPINBLUETAN),UDM_SETRANGE,0,MAKELPARAM(255,0));

		SendMessage(GetDlgItem(hwndDlg,IDC_BD_BM_RBFRAME),BM_SETCHECK,BST_CHECKED,0);

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
			g_currChar.m_cardData.RemoveBoneRule(sel);
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
			else if (identifier == IDC_BD_CBTANCOLOR) {
				BOOL visible = SendMessage(thisPtr->m_cbTanColor,BM_GETCHECK,0,0) == BST_CHECKED;
				g_currChar.m_cardData.SetHasTanColor(visible == TRUE);
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
			else if(ed == thisPtr->m_edTanColorRed
				     || ed == thisPtr->m_edTanColorGreen
					 || ed == thisPtr->m_edTanColorBlue)
			{
				int newval = General::GetEditInt(ed);
				if (newval < 0) {
					SendMessage(ed,WM_SETTEXT,0,(LPARAM)TEXT("0"));
				}
				else if (newval > 255) {
					SendMessage(ed,WM_SETTEXT,0,(LPARAM)TEXT("255"));
				}
				else {
					int red = General::GetEditInt(thisPtr->m_edTanColorRed);
					int green = General::GetEditInt(thisPtr->m_edTanColorGreen);
					int blue = General::GetEditInt(thisPtr->m_edTanColorBlue);
					g_currChar.m_cardData.SetTanColor(RGB(blue,green,red));
					using namespace ExtVars::AAEdit;
					RedrawBodyPart(BODY_COLOR,BODYCOLOR_SKINTONE);
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
			HWND wnd = (HWND)lparam;
			if(wnd == thisPtr->m_bmList) {
				int sel = SendMessage(thisPtr->m_bmList,LB_GETCURSEL,0,0);
				if(sel != LB_ERR) {
					thisPtr->LoadData(sel);
				}
				return TRUE;
			}
			break; }
		};
		break; }
	};
	return FALSE;
}

void UnlimitedDialog::BDDialog::LoadData(int listboxId) {
	const auto& vec = g_currChar.m_cardData.GetMeshRuleList();
	const auto& rule = vec[listboxId];
	//combo box with title
	SendMessage(m_bmCbXXFile,WM_SETTEXT,0,(LPARAM)rule.first.second.first.c_str());
	SendMessage(m_bmCbBone,WM_SETTEXT,0,(LPARAM)rule.first.second.second.c_str());
	int flags = rule.first.first;
	if(flags & AAUCardData::MODIFY_BONE) {
		SendMessage(m_bmRbBoneMod,BM_SETCHECK,BST_CHECKED,0);
	}
	if (flags & AAUCardData::MODIFY_FRAME) {
		SendMessage(m_bmRbFrameMod,BM_SETCHECK,BST_CHECKED,0);
	}
	//the matrix

	for(int i = 0; i < 3; i++) {
		for(int j = 0; j < 3; j++) {
			std::wstring num = std::to_wstring(rule.second.mods[i][j]);
			SendMessage(m_bmEdMatrix[i][j],WM_SETTEXT,0,(LPARAM)num.c_str());
		}
	}
	//
}

void UnlimitedDialog::BDDialog::ApplyInput() {
	TCHAR xxname[128];
	SendMessage(m_bmCbXXFile,WM_GETTEXT,128,(LPARAM)xxname);
	TCHAR bonename[128];
	SendMessage(m_bmCbBone,WM_GETTEXT,128,(LPARAM)bonename);
	int flags = 0;
	if (SendMessage(m_bmRbBoneMod,BM_GETCHECK,0,0) == BST_CHECKED) flags |= AAUCardData::MODIFY_BONE;
	if (SendMessage(m_bmRbFrameMod,BM_GETCHECK,0,0) == BST_CHECKED) flags |= AAUCardData::MODIFY_FRAME;
	//remove transformation if it allready exists
	const auto& vec = g_currChar.m_cardData.GetMeshRuleList();
	unsigned int match;
	for(match = 0; match < vec.size(); match++) {
		if (vec[match].first.first == flags && vec[match].first.second.first == xxname && vec[match].first.second.second == bonename) break;
	}
	if (match < vec.size()) g_currChar.m_cardData.RemoveBoneRule(match);

	//get matrix
	AAUCardData::BoneMod mod;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			TCHAR num[128];
			SendMessage(m_bmEdMatrix[i][j],WM_GETTEXT,128,(LPARAM)num);
			float f = wcstof(num, NULL);
			mod.mods[i][j] = f;
		}
	}
	//save
	g_currChar.m_cardData.AddBoneRule((AAUCardData::MeshModFlag)flags, xxname,bonename,mod);
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
	const auto& list = AAEdit::g_currChar.m_cardData.GetMeshRuleList();
	for (size_t i = 0; i < list.size(); i++) {
		std::wstring listEntry(TEXT("["));
		if (list[i].first.first & AAUCardData::MODIFY_BONE) listEntry += TEXT("BONE");
		else listEntry += TEXT("FRAME");
		listEntry += TEXT("]");
		listEntry += list[i].first.second.first + TEXT("|") + list[i].first.second.second + TEXT("]");
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
		std::queue<ExtClass::Frame*> fileQueue;
		for (ExtClass::XXFile* file : xxlist) {
			if (file == NULL) continue;
			ExtClass::Frame* root = file->m_root;
			fileQueue.push(root);
			while (!fileQueue.empty()) {
				ExtClass::Frame* bone = fileQueue.front();
				fileQueue.pop();
				size_t conv;
				mbstowcs_s(&conv,tmpBuff,bone->m_name,bone->m_nameBufferSize);
				SendMessage(m_bmCbBone,CB_ADDSTRING,0,(LPARAM)tmpBuff);
				for (unsigned int i = 0; i < bone->m_nChildren; i++) {
					fileQueue.push(bone->m_children + i);
				}
			}
		}
	}
	
}

/**********************/
/* Body Slider Dialog */
/**********************/

INT_PTR CALLBACK UnlimitedDialog::BSDialog::DialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam) {
	static bool ignoreNextSlider = false;

	switch (msg) {
	case WM_INITDIALOG: {
		BSDialog* thisPtr = (BSDialog*)lparam;
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lparam); //register class to this hwnd
		thisPtr->m_dialog = hwndDlg;

		ShowWindow(GetDlgItem(hwndDlg,IDC_BS_STEXAMPLE),SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg,IDC_BS_SLDEXAMPLE),SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg,IDC_BS_EDEXAMPLE),SW_HIDE);

		using namespace ExtClass;
		static struct GUISlider { //this needs a name for some reason, else it wont compile
			const TCHAR* label;
			struct LogicalSlider {
				CharacterStruct::Models model;
				int index;
			};
			std::vector<LogicalSlider> logicalSliders;
			float min;
			float max;
		} sliderIds[] = {
			{ TEXT("Mouth Width"), 
				{ { CharacterStruct::FACE, 0 } }, 
				-0.9f, 0.5f 
			},
			{ TEXT("Total Body Height"),
				{ { CharacterStruct::SKELETON, 0 } },
				-0.9f, 1.0f 
			},
			{ TEXT("Total Body Width"),
				{ { CharacterStruct::SKELETON, 1 } }, 
				-0.9f, 1.0f
			},
			{ TEXT("Total Body Thickness"),
				{ { CharacterStruct::SKELETON, 2 } }, 
				-0.9f, 1.0f
			},
			{ TEXT("Total Ear Height"),
				{ { CharacterStruct::FACE, 1 } },
				-0.05f, 0.05f
			},
			{ TEXT("Mouth Height"),
				{ { CharacterStruct::FACE, 2 } },
				-0.05f, 0.05f
			},
			/*{ TEXT("Arm Thickness"),
				{ { CharacterStruct::SKELETON, 3 },{ CharacterStruct::SKELETON, 4 } },
				-0.5f, 1.0f
			},*/
			{ TEXT("Hand Length"),
				{ { CharacterStruct::SKELETON, 5 },{ CharacterStruct::SKELETON, 6 },
				  { CharacterStruct::SKELETON, 13 },{ CharacterStruct::SKELETON, 14 } },
				-0.4f, 0.2f
			},
			{ TEXT("Foot Length"),
			{ { CharacterStruct::SKELETON, 15 },{ CharacterStruct::SKELETON, 16 } },
				-0.15f, 0.2f
			},
			{ TEXT("Foot Width"),
				{ { CharacterStruct::SKELETON, 17 },{ CharacterStruct::SKELETON, 18 } },
				-0.2f, 0.2f
			},
			{ TEXT("Eyebrow Height"),
				{ { CharacterStruct::FACE, 3 },{ CharacterStruct::FACE, 4 } },
				-0.1f, 0.1f
			},
			{ TEXT("Eye Depth"),
				{ { CharacterStruct::FACE, 5 } },
				-0.1f, 0.1f
			},
			{ TEXT("Bottom Bone Width"),
				{ { CharacterStruct::BODY, 0 } },
				-0.5f, 0.5f
			},
			{ TEXT("Bottom Bone Thickness"),
				{ { CharacterStruct::BODY, 1 }  },
				-0.5f, 0.5f
			},
			{ TEXT("Bottom Width"),
				{ { CharacterStruct::SKELETON, 7 } },
				0.5f, 1.5f
			},
			{ TEXT("Bottom Thickness"),
				{ { CharacterStruct::SKELETON, 8 } },
				0.5f, 1.5f
			},
			{ TEXT("Lower Bottom Thickness"),
				{ { CharacterStruct::SKELETON, 9 }, { CharacterStruct::SKELETON, 10 } },
				-0.5f, 0.5f
			},
			{ TEXT("Lower Bottom Width"),
				{ { CharacterStruct::SKELETON, 11 },{ CharacterStruct::SKELETON, 12 } },
				-0.5f, 0.5f
			},
			{ TEXT("Thigh Thickness"),
				{ { CharacterStruct::SKELETON, 19 },{ CharacterStruct::SKELETON, 20 } },
				-0.5f, 0.5f
			},
			{ TEXT("Shoulder Width"),
				{ { CharacterStruct::SKELETON, 36 },{ CharacterStruct::SKELETON, 37 } },
				-0.5f, 0.5f
			},
			{ TEXT("Shoulder Size"),
				{ { CharacterStruct::SKELETON, 21 },{ CharacterStruct::SKELETON, 22 } },
				-0.5f, 0.5f
			},
			{ TEXT("Shoulder Height"),
				{ { CharacterStruct::SKELETON, 23 },{ CharacterStruct::SKELETON, 24 } },
				-0.5f, 0.5f
			},
			{ TEXT("Neck Bone Size"),
				{ { CharacterStruct::BODY, 2 },{ CharacterStruct::BODY, 3 } },
				-0.5f, 0.5f
			},
			{ TEXT("Arm Bone Size"),
				{ { CharacterStruct::BODY, 4 },{ CharacterStruct::BODY, 5 },{ CharacterStruct::BODY, 6 },
				  { CharacterStruct::BODY, 7 },{ CharacterStruct::BODY, 8 },{ CharacterStruct::BODY, 9 },
				  { CharacterStruct::BODY, 10 },{ CharacterStruct::BODY, 11 } },
				-0.5f, 0.5f
			},
			{ TEXT("Ball Size"),
				{ { CharacterStruct::SKELETON, 29 } },
				0.5f, 1.5f
			},
			{ TEXT("Strapon Length"),
				{ { CharacterStruct::SKELETON, 30 },{ CharacterStruct::SKELETON, 31 } },
				-0.5f, 0.5f
			},
			{ TEXT("Strapon Girth"),
				{ { CharacterStruct::SKELETON, 32 },{ CharacterStruct::SKELETON, 33 } },
				0.5f, 1.5f
			},
			{ TEXT("Glans Size"),
				{ { CharacterStruct::SKELETON, 34 },{ CharacterStruct::SKELETON, 35 } },
				0.5f, 1.5f
			},
			{ TEXT("[split] Ear Width"),
				{ { CharacterStruct::FACE, 6 },{ CharacterStruct::FACE, 7 } },
				-0.5f, 1.5f
			},
			{ TEXT("[split] Ear Height"),
				{ { CharacterStruct::FACE, 8 },{ CharacterStruct::FACE, 9 } },
				-0.5f, 1.5f
			},
		};
		
		int xpos = 10,ypos = 10;
		for(int i = 0; i < ARRAYSIZE(sliderIds); i++) {
			std::vector<const Shared::Slider*> sldVec;
			for(size_t j = 0; j < sliderIds[i].logicalSliders.size(); j++) {
				sldVec.push_back(&Shared::g_sliders[sliderIds[i].logicalSliders[j].model][sliderIds[i].logicalSliders[j].index]);
			}
			thisPtr->m_sliders.push_back(BodySlider(hwndDlg,sliderIds[i].label,xpos,ypos,
				sldVec,sliderIds[i].min,sliderIds[i].max));
			ypos += 40; //move next one a bit farer down
		}
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_PAGE | SIF_RANGE;
		si.nMin = 0;
		si.nMax = ypos;
		si.nPage = 200;
		SetScrollInfo(hwndDlg,SB_VERT,&si,FALSE);
		

		return TRUE; }
	case WM_HSCROLL: {
		BSDialog* thisPtr = (BSDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		if (thisPtr == NULL) return FALSE;
		if (ignoreNextSlider) {
			ignoreNextSlider = false;
			//return TRUE;
		}
		HWND wnd = (HWND)lparam;
		if (wnd == NULL) break; //not slider control, but automatic scroll
		for (size_t i = 0; i < thisPtr->m_sliders.size(); i++) {
			if(wnd == thisPtr->m_sliders[i].slider) {
				ignoreNextSlider = true;
				thisPtr->m_sliders[i].Sync(false);
				thisPtr->ApplySlider(i);
				break;
			}
		}
		break; }
	case WM_VSCROLL: {
		BSDialog* thisPtr = (BSDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		if (thisPtr == NULL) return FALSE;
		HWND wnd = (HWND)lparam;
		if(wnd == NULL) {
			General::ScrollWindow(hwndDlg,wparam);
			return TRUE;
		}
		break; }
	case WM_COMMAND: {
		BSDialog* thisPtr = (BSDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		if (thisPtr == NULL) return FALSE;
		switch (HIWORD(wparam)) {
		case EN_CHANGE: {
			if (ignoreNextSlider) {
				ignoreNextSlider = false;
				return TRUE;
			}
			HWND ed = (HWND)lparam;
			for (size_t i = 0; i < thisPtr->m_sliders.size(); i++) {
				if (ed == thisPtr->m_sliders[i].edit) {
					ignoreNextSlider = true;
					thisPtr->m_sliders[i].Sync(true);
					thisPtr->ApplySlider(i);
					break;
				}
			}
			return TRUE; }
		};
		break; }
	};
	return FALSE;
}

void UnlimitedDialog::BSDialog::ApplySlider(int index) {
	//make sure slider operations are valid. else, crashes might occur
	if(ExtVars::AAEdit::GetCurrentCharacter() == NULL || ExtVars::AAEdit::GetCurrentCharacter()->m_xxSkeleton == NULL) {
		//every character has a skeleton, and the current character is only filled in the editor (previews dont count). therefor,
		//if neither is true, we are not currently editing a valid model
		for(int i = 0; i < ExtClass::CharacterStruct::N_MODELS; i++) {
			Shared::g_xxBoneMods[i].clear();
			Shared::g_xxBoneParents[i].clear();
			Shared::g_xxMods[i].clear();
		}
		return;
	}
	bool faceSliderRenew = false;
	std::set<ExtClass::CharacterStruct::Models> renewFiles;
	for (auto& slider : m_sliders[index].sliderData) {
		g_currChar.m_cardData.SetSliderValue(slider->target,slider->index,m_sliders[index].currVal);
		renewFiles.insert(slider->target);
		faceSliderRenew = faceSliderRenew || slider->target == ExtClass::CharacterStruct::FACE_SLIDERS;

		//reset targeted matrix in case we 0ed it and the for loops below do not cover it anymore
		if(m_sliders[index].currVal == slider->GetNeutralValue()) {
			if(slider->flags & AAUCardData::MODIFY_FRAME) {
				for (auto& elem : Shared::g_xxMods[slider->target]) {
					ExtClass::Frame* frame = elem.first;
					TCHAR buff[256];
					size_t written;
					mbstowcs_s(&written,buff,frame->m_name+5,256);
					std::wstring str(buff);
					if (str == slider->boneName) {
						frame->m_matrix1 = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
						frame->m_matrix5 = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
						break;
					}
				}
			}
			if (slider->flags & AAUCardData::MODIFY_BONE) {
				for (auto& elem : Shared::g_xxBoneParents[slider->target]) {
					std::string mbBoneName(elem.boneName.begin(),elem.boneName.end());
					if (elem.boneName == slider->boneName) {
						for(auto& frameParents : elem.parents) {
							for(int i = 0; i < frameParents->m_nBones; i++) {
								ExtClass::Bone* bone = &frameParents->m_bones[i];
								if(bone->m_name == mbBoneName) {
									bone->m_matrix = elem.origMatrix;
									break;
								}
							}
						}
						break;
					}
				}
			}

		}
	}
	

	for (ExtClass::CharacterStruct::Models model : renewFiles) {
		for(auto& elem : Shared::g_xxMods[model]) {
			ExtClass::Frame* frame = (ExtClass::Frame*)elem.first;
			TCHAR buff[256];
			size_t written;
			mbstowcs_s(&written,buff,frame->m_name+5,256);
			std::wstring str(buff);
			auto* rule = Shared::g_currentChar->m_cardData.GetSliderFrameRule(model,str);
			if(rule != NULL) {
				D3DMATRIX& mat = elem.second;
				D3DVECTOR3 scale = { mat._11, mat._12, mat._13 };
				D3DVECTOR3 rot = { mat._21, mat._22, mat._23 };
				D3DVECTOR3 trans = { mat._31, mat._32, mat._33 };
				for(auto& elem : *rule) {
					Shared::Slider::ModifySRT(&scale,&rot,&trans,elem.first->op,elem.second);
				}
				auto res = General::MatrixFromSRT(scale,rot,trans);
				frame->m_matrix1 = res;
				frame->m_matrix5 = res;
			}
		}
		for (auto& elem : Shared::g_xxBoneParents[model]) {
			auto* rule = Shared::g_currentChar->m_cardData.GetSliderBoneRule(model,elem.boneName);
			if(rule != NULL) {
				std::string strBoneName(elem.boneName.begin(), elem.boneName.end());
				for(auto* frame : elem.parents) {
					for(int i = 0; i < frame->m_nBones; i++) {
						ExtClass::Bone* bone = &frame->m_bones[i];
						if(bone->m_name == strBoneName) {
							D3DMATRIX& mat = elem.srtMatrix;
							D3DMATRIX& origMat = elem.origMatrix;
							D3DVECTOR3 scale = { mat._11, mat._12, mat._13 };
							D3DVECTOR3 rot = { mat._21, mat._22, mat._23 };
							D3DVECTOR3 trans = { mat._31, mat._32, mat._33 };
							for (auto& elem : *rule) {
								Shared::Slider::ModifySRT(&scale,&rot,&trans,elem.first->op,elem.second);
							}
							auto res = General::MatrixFromSRT(scale,rot,trans);
							(*Shared::D3DXMatrixMultiply)(&bone->m_matrix,&res,&origMat);
						}
					}
				}
			}
		}
		/*for(auto& elem : Shared::g_xxBoneMods[model]) {
			for(auto& savedBone : elem.bones) {
				ExtClass::Bone* bone = savedBone.ptr;
				if (bone->m_name == NULL) {
					LOGPRIO(Logger::Priority::WARN) << "bone name is NULL for some reason\r\n";
					continue;
				}
				TCHAR buff[256];
				size_t written;
				mbstowcs_s(&written,buff,bone->m_name,256);
				std::wstring str(buff);
				auto* rule = Shared::g_currentChar->m_cardData.GetSliderBoneRule(model,str);
				if (rule != NULL) {
					D3DMATRIX& mat = elem.srtMatrix;
					D3DMATRIX& origMat = elem.origMatrix;
					D3DVECTOR3 scale = { mat._11, mat._12, mat._13 };
					D3DVECTOR3 rot = { mat._21, mat._22, mat._23 };
					D3DVECTOR3 trans = { mat._31, mat._32, mat._33 };
					for (auto& elem : *rule) {
						Shared::Slider::ModifySRT(&scale,&rot,&trans,elem.first->op,elem.second);
					}
					auto res = General::MatrixFromSRT(scale,rot,trans);
					(*Shared::D3DXMatrixMultiply)(&bone->m_matrix,&res,&origMat);
				}
			}
		}*/
	}

	if(faceSliderRenew) {
		ExtVars::AAEdit::RedrawBodyPart(ExtVars::AAEdit::EYES,ExtVars::AAEdit::EYES_ALL);
	}
}

void UnlimitedDialog::BSDialog::Refresh() {
	for (size_t i = 0; i < m_sliders.size(); i++) {
		m_sliders[i].FromCard();
	}
}

void UnlimitedDialog::BSDialog::BodySlider::Sync(bool useEdit) {
	if(useEdit) {
		//sync slider with edit
		currVal = General::GetEditFloat(edit);
		int ret = SendMessage(slider,TBM_SETPOS,TRUE,Val2Sld(currVal));
		ret++;
	}
	else {
		//sync edit with slider
		int pos = SendMessage(slider,TBM_GETPOS,0,0);
		currVal = Sld2Val(pos);
		TCHAR number[52];
		swprintf_s(number,TEXT("%f"),currVal);
		SendMessage(edit,WM_SETTEXT,0,(LPARAM)number);
	}
}


UnlimitedDialog::BSDialog::BodySlider::BodySlider()  {

}

UnlimitedDialog::BSDialog::BodySlider::BodySlider(HWND dialog, const TCHAR* label, int xStart, int yStart,
												std::vector<const Shared::Slider*> sliderData, float min, float max)
	: sliderData(sliderData) {
	HWND templateStatic = GetDlgItem(dialog,IDC_BS_STEXAMPLE);
	HWND templateSlider = GetDlgItem(dialog,IDC_BS_SLDEXAMPLE);
	HWND templateEdit = GetDlgItem(dialog,IDC_BS_EDEXAMPLE);

	this->stLabel = CreateWindowEx(0,TEXT("STATIC"),label,WS_CHILD | WS_VISIBLE,
		0,0,0,0,
		dialog,0,General::DllInst,0);
	this->slider = CreateWindowEx(0,TEXT("msctls_trackbar32"),label,WS_CHILD | WS_VISIBLE | TBS_BOTH | TBS_NOTICKS | WS_TABSTOP,
		0,0,0,0,
		dialog,0,General::DllInst,0);
	this->edit = CreateWindowEx(WS_EX_CLIENTEDGE,TEXT("EDIT"),label,WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
		0,0,0,0,
		dialog,0,General::DllInst,0);

	using namespace General;

	auto CopyStyleFromWindow = [](HWND to, HWND from){
		LONG exStyles = GetWindowLong(from,GWL_EXSTYLE);
		LONG styles = GetWindowLong(from,GWL_STYLE) | WS_VISIBLE;
		SetWindowLong(to,GWL_EXSTYLE,exStyles);
		SetWindowLong(to,GWL_STYLE,styles);
		HFONT font = (HFONT)SendMessage(from,WM_GETFONT,0,0);
		SendMessage(to,WM_SETFONT,(WPARAM)font,FALSE);
	};

	//adjust style from templace
	CopyStyleFromWindow(stLabel,templateStatic);
	CopyStyleFromWindow(slider,templateSlider);
	CopyStyleFromWindow(edit,templateEdit);

	//move window according to template
	RECT rctTmplStatic,rctTmplSlider,rctTmplEdit;
	GetWindowRect(templateStatic,&rctTmplStatic);
	GetWindowRect(templateSlider,&rctTmplSlider);
	GetWindowRect(templateEdit,&rctTmplEdit);
	//get top left corner
	LONG left = rctTmplStatic.left,top = min(rctTmplStatic.top,min(rctTmplSlider.top,rctTmplEdit.top));
	//adjust top left corner to (xStart|yStart)
	RectMoveBy(rctTmplStatic,-left+xStart,-top+yStart);
	RectMoveBy(rctTmplSlider,-left+xStart,-top+yStart);
	RectMoveBy(rctTmplEdit,-left+xStart,-top+yStart);
	//move actual window
	MoveWindowRect(stLabel,rctTmplStatic,FALSE);
	MoveWindowRect(slider,rctTmplSlider,FALSE);
	MoveWindowRect(edit,rctTmplEdit,FALSE);



	sliderMin = min;
	sliderMax = max;
	staticLabel = label;
	int ret = SendMessage(this->slider,TBM_SETRANGEMIN,TRUE,0);
	ret = SendMessage(this->slider,TBM_SETRANGEMAX,TRUE,0x10000);
	ret = SendMessage(this->slider,TBM_SETLINESIZE,0, 0x10000 / 100);
	ret = SendMessage(this->slider,TBM_SETPAGESIZE,0, 0x10000 / 100);
}

float UnlimitedDialog::BSDialog::BodySlider::Sld2Val(int sld) {
	return sliderMin + (sliderMax-sliderMin)/0x10000 * sld;
}

int UnlimitedDialog::BSDialog::BodySlider::Val2Sld(float val) {
	if (val < sliderMin || val > sliderMax) return 0;
	float max = sliderMax - sliderMin; //map from [min,max] to [0,max]
	val -= sliderMin;
	float coeff = 0x10000 / max;
	return int(coeff * val);
}

float UnlimitedDialog::BSDialog::BodySlider::GetCoeffFromMod(AAUCardData::BoneMod mod) {
	float lastCoeff = 0;
	for (int i = 0; i < 9; i++) {
		float coeff = 0;
		if (mod.data[i] != 0) coeff = mod.data[i] / mod.data[i];
		if (lastCoeff == 0) lastCoeff = coeff;
		else {
			//different coeffs, doesnt add up
			return 0;
		}
	}
	return lastCoeff;
}

/*
 * Set slider and edit from card data
 */
void UnlimitedDialog::BSDialog::BodySlider::FromCard() {
	currVal = 0;
	if (sliderData.size() == 0) return;
	currVal = sliderData[0]->GetNeutralValue();
	const auto& list = g_currChar.m_cardData.GetSliderList();
	for(auto& slider : sliderData) {
		for (auto elem : list) {
			if (&Shared::g_sliders[elem.first.first][elem.first.second] == slider) {
				currVal = elem.second;
				break;
			}
		}
	}
	

	TCHAR buffer[256];
	swprintf_s(buffer,TEXT("%f"),currVal);
	SendMessage(edit,WM_SETTEXT,0,(LPARAM)buffer);
	Sync(true);
}


/*****************/
/* Module Dialog */
/*****************/

INT_PTR CALLBACK UnlimitedDialog::MDDialog::DialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam) {
	switch (msg) {
	case WM_INITDIALOG: {
		MDDialog* thisPtr = (MDDialog*)lparam;
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lparam); //register class to this hwnd
		thisPtr->m_dialog = hwndDlg;
		thisPtr->m_lbModulesAvailable = GetDlgItem(hwndDlg,IDC_MB_LBAVAILABLE);
		thisPtr->m_lbModulesUsed = GetDlgItem(hwndDlg,IDC_MD_LBINUSE);
		thisPtr->m_edName = GetDlgItem(hwndDlg,IDC_MD_EDNAME);
		thisPtr->m_edDescr = GetDlgItem(hwndDlg,IDC_MD_EDDESCR);

		return TRUE;
		break; }
	case WM_COMMAND: {
		MDDialog* thisPtr = (MDDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		DWORD identifier = LOWORD(wparam);
		DWORD notification = HIWORD(wparam);
		HWND wnd = (HWND)lparam;
		switch (identifier) {
		case IDC_MD_LBAVAILABLE:
		case IDC_MD_LBINUSE:
			if(notification == LBN_SELCHANGE) {
				int sel = SendMessage(wnd,LB_GETCURSEL,0,0);
				if(sel != LB_ERR) {
					SendMessage(thisPtr->m_edName,WM_SETTEXT,0,(LPARAM)thisPtr->m_modules[sel].mod.name.c_str());
					SendMessage(thisPtr->m_edDescr,WM_SETTEXT,0,(LPARAM)thisPtr->m_modules[sel].mod.description.c_str());
					return TRUE;
				}
			}
			break;
		case IDC_MD_BTNRIGHT:
			if(notification == BN_CLICKED) {
				int sel = SendMessage(thisPtr->m_lbModulesAvailable,LB_GETCURSEL,0,0);
				if(sel != LB_ERR) {
					g_currChar.m_cardData.AddModule(thisPtr->m_modules[sel].mod);
					thisPtr->Refresh();
				}
			}
			break;
		case IDC_MD_BTNLEFT:
			if (notification == BN_CLICKED) {
				int sel = SendMessage(thisPtr->m_lbModulesUsed,LB_GETCURSEL,0,0);
				if (sel != LB_ERR) {
					g_currChar.m_cardData.RemoveModule(sel);
					thisPtr->Refresh();
				}
			}
			break;
		case IDC_MD_BTNADDASTRIGGER:
			if (notification == BN_CLICKED) {
				int sel = SendMessage(thisPtr->m_lbModulesUsed,LB_GETCURSEL,0,0);
				if (sel != LB_ERR) {
					for(auto& trigger : g_currChar.m_cardData.GetModules()[sel].triggers) {
						g_currChar.m_cardData.GetTriggers().push_back(trigger);
					}
				}
				g_currChar.m_cardData.RemoveModule(sel);
				thisPtr->Refresh();
			}
			break;
		}
		break; }
	}
	return FALSE;
}

void UnlimitedDialog::MDDialog::Refresh() {
	//list available modules
	m_modules.clear();
	SendMessage(m_lbModulesAvailable,LB_RESETCONTENT,0,0);
	std::wstring modDirectory = General::BuildOverridePath(MODULE_PATH,TEXT("*"));
	if (!General::DirExists(modDirectory.c_str())) {
		CreateDirectory(modDirectory.c_str(),NULL);
	}
	WIN32_FIND_DATA data;
	HANDLE hSearch = FindFirstFile(modDirectory.c_str(),&data);

	if (hSearch != INVALID_HANDLE_VALUE) {
		BOOL suc = FALSE;
		do {
			if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				ModuleFile mod(General::BuildOverridePath(MODULE_PATH,data.cFileName).c_str());
				if(mod.IsGood()) {
					m_modules.push_back(mod);
					SendMessage(m_lbModulesAvailable,LB_ADDSTRING,0,(LPARAM)data.cFileName);
				}
			}
			suc = FindNextFile(hSearch,&data);
		} while (suc != FALSE);
		FindClose(hSearch);
	}

	//list current modules
	SendMessage(m_lbModulesUsed,LB_RESETCONTENT,0,0);
	for(auto& elem : g_currChar.m_cardData.GetModules()) {
		SendMessage(m_lbModulesUsed,LB_ADDSTRING,0,(LPARAM)elem.name.c_str());
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