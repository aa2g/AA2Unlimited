#include "StdAfx.h"
#include "Files/PNGData.h"


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
			g_Config.bUnlimitedOnTop ? *ExtVars::AAEdit::MainWnd : 0, MainDialogProc, (LPARAM)this);
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
		if (g_Config.bTriggers) {
			thisPtr->AddDialog(IDD_TRIGGERS, &thisPtr->m_trDialog, index++, TEXT("Triggers"),
				TRDialog::DialogProc);
			thisPtr->AddDialog(IDD_MODULES, &thisPtr->m_mdDialog, index++, TEXT("Modules"),
				MDDialog::DialogProc);
		}

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

		const DWORD femaleRule[]{ 0x353254, 0x2C, 0 };
		const DWORD maleRule[]{ 0x353254, 0x30, 0 };
		AAEdit::g_currChar.m_char = (ExtClass::CharacterStruct*) ExtVars::ApplyRule(femaleRule);
		if (AAEdit::g_currChar.m_char == NULL) (ExtClass::CharacterStruct*) ExtVars::ApplyRule(maleRule);

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
		if (g_currChar.Editable())
			AAEdit::g_currChar.m_cardData.UpdateCardStyle(AAEdit::g_currChar.m_cardData.GetCurrAAUSet(), g_currChar.m_char->m_charData);
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
		thisPtr->m_btnLoadCloth = GetDlgItem(hwndDlg, IDC_GN_BTNLOADCLOTH);
		thisPtr->m_edAAuSetName = GetDlgItem(hwndDlg,IDC_GN_EDAAUSETNAME);

		return TRUE;
		break; }
	//case WM_VKEYTOITEM: {
	//	//DEL-key was pressed while the list box had the focus
	//	GNDialog* thisPtr = (GNDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
	//	if (LOWORD(wparam) == VK_DELETE) {
	//		//get current selection text
	//		int sel = SendMessage(thisPtr->m_lbAAuSets,LB_GETCURSEL,0,0);
	//		//remove this rule
	//		if (g_currChar.Editable()) {
	//			g_currChar.m_cardData.SwitchActiveCardStyle(0, g_currChar.m_char->m_charData);
	//			g_currChar.m_cardData.RemoveCardStyle(sel);
	//		}
	//		thisPtr->RefreshAAuSetList();
	//		return TRUE;
	//	}
	//	break; }
	
	case WM_COMMAND: {
		GNDialog* thisPtr = (GNDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		switch(HIWORD(wparam)) {
		case BN_CLICKED: {
			DWORD identifier = LOWORD(wparam);
			switch (identifier) {
			case IDC_GN_BTNAAUSETADD:
				{
					TCHAR bufAdd[256];
					SendMessage(thisPtr->m_edAAuSetName, WM_GETTEXT, 256, (LPARAM)&bufAdd);

					if (g_currChar.Editable()) {
						AAEdit::g_currChar.m_cardData.UpdateCardStyle(AAEdit::g_currChar.m_cardData.GetCurrAAUSet(), g_currChar.m_char->m_charData);
						AAEdit::g_currChar.m_cardData.CopyCardStyle(bufAdd, g_currChar.m_char->m_charData);
					}
					thisPtr->RefreshAAuSetList();
				}
				return TRUE;
			case IDC_GN_BTNAAUSETREMOVE:
				{
					//get current selection
					int selRemove = SendMessage(thisPtr->m_lbAAuSets, LB_GETCURSEL, 0, 0);
					if (selRemove == 0) return FALSE;
					//remove this rule
					if (g_currChar.Editable()) {
						g_currChar.m_cardData.SwitchActiveCardStyle(0, g_currChar.m_char->m_charData);
						g_currChar.m_cardData.RemoveCardStyle(selRemove);
					}
					thisPtr->RefreshAAuSetList();
				}
				return TRUE;
			case IDC_GN_BTNAAUSETRENAME:
				{
					TCHAR bufRename[256];
					SendMessage(thisPtr->m_edAAuSetName, WM_GETTEXT, 256, (LPARAM)&bufRename);
					//get current selection
					int selRename = SendMessage(thisPtr->m_lbAAuSets, LB_GETCURSEL, 0, 0);
					if (selRename == 0) return FALSE;

					if (g_currChar.Editable()) {
						wcscpy_s(AAEdit::g_currChar.m_cardData.m_styles[selRename].m_name, bufRename);
					}
					thisPtr->RefreshAAuSetList();
				}
				return TRUE;
			case IDC_GN_BTNSETCLOTH0:
			case IDC_GN_BTNSETCLOTH1:
			case IDC_GN_BTNSETCLOTH2:
			case IDC_GN_BTNSETCLOTH3:
				{
					auto idx = IDC_GN_BTNSETCLOTH0;
					const TCHAR* path = General::SaveFileDialog(General::BuildPlayPath(TEXT("data\\save\\cloth")).c_str());
					if (path != NULL) {
						auto buf = General::FileToBuffer(path);
						//Replacing the wrong bytes, that's just how it is. 
						std::vector<BYTE> backup{ buf[9], buf[10], buf[11] };
						buf.erase(buf.begin() + 9, buf.begin() + 12);
						buf.insert(buf.begin() + 73, backup.begin(), backup.end());

						if (buf.size() >= 92)
							memcpy(&Shared::PNG::saved_clothes[identifier - idx], buf.data() + 1, 91);
						ClothFile load(General::FileToBuffer(path));
						if (!load.IsValid()) return FALSE;
						if (g_currChar.Editable()) {
							auto cloth = &AAEdit::g_currChar.m_char->m_charData->m_clothes[identifier - idx];
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
							AAEdit::g_currChar.m_char->m_currClothes = identifier - idx;
							AAEdit::g_currChar.m_char->Update(1, 1);
						}
						return TRUE;
					}
				}
				return FALSE;
			}
			break;
		}
		case LBN_SELCHANGE: {
			GNDialog* thisPtr = (GNDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
			HWND wnd = (HWND)lparam;
			if (wnd == thisPtr->m_lbAAuSets) {
				int sel = SendMessage(thisPtr->m_lbAAuSets,LB_GETCURSEL,0,0);
				if (sel != LB_ERR) {
					if (g_currChar.Editable()) {
						AAEdit::g_currChar.m_cardData.UpdateCardStyle(AAEdit::g_currChar.m_cardData.GetCurrAAUSet(), g_currChar.m_char->m_charData);
						AAEdit::g_currChar.m_cardData.SwitchActiveCardStyle(sel, g_currChar.m_char->m_charData);
					}
					using namespace ExtVars::AAEdit;
					LUA_EVENT_NORET("update_edit_gui")
					Shared::preservingFrontHairSlot = AAEdit::g_currChar.m_char->m_charData->m_hair.frontHair;
					Shared::preservingSideHairSlot = AAEdit::g_currChar.m_char->m_charData->m_hair.sideHair;
					Shared::preservingBackHairSlot = AAEdit::g_currChar.m_char->m_charData->m_hair.backhair;
					Shared::preservingExtHairSlot = AAEdit::g_currChar.m_char->m_charData->m_hair.hairExtension;
					//RedrawBodyPart(Category::FIGURE, RedrawId::FIGURE_HEIGHT);
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
		case LBN_DBLCLK: {
			DWORD identifier = LOWORD(wparam);
			if (identifier == IDC_OO_LIST) {
				LRESULT res = SendMessage(thisPtr->m_lbOverrides, LB_GETCURSEL, 0, 0);
				if (res != LB_ERR) {
					auto list = AAEdit::g_currChar.m_cardData.GetObjectOverrideList();
					auto override = list.at(res);
					SetWindowText(thisPtr->m_edObject, override.first.c_str());
					SetWindowText(thisPtr->m_edFile, override.second.c_str());
				}
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

		thisPtr->m_cbTanColor = GetDlgItem(hwndDlg, IDC_BD_CBTANCOLOR);
		thisPtr->m_edTanColorRed = GetDlgItem(hwndDlg, IDC_BD_EDTANCOLOR_RED);
		thisPtr->m_edTanColorGreen = GetDlgItem(hwndDlg, IDC_BD_EDTANCOLOR_GREEN);
		thisPtr->m_edTanColorBlue = GetDlgItem(hwndDlg, IDC_BD_EDTANCOLOR_BLUE);
		thisPtr->m_edTanColorHue = GetDlgItem(hwndDlg, IDC_BD_EDTANCOLOR_HUE);
		thisPtr->m_edTanColorSat = GetDlgItem(hwndDlg, IDC_BD_EDTANCOLOR_SAT);
		thisPtr->m_edTanColorVal = GetDlgItem(hwndDlg, IDC_BD_EDTANCOLOR_VAL);

		SendMessage(GetDlgItem(hwndDlg, IDC_BD_SPINREDTAN), UDM_SETRANGE, 0, MAKELPARAM(255, 0));
		SendMessage(GetDlgItem(hwndDlg, IDC_BD_SPINGREENTAN), UDM_SETRANGE, 0, MAKELPARAM(255, 0));
		SendMessage(GetDlgItem(hwndDlg, IDC_BD_SPINBLUETAN), UDM_SETRANGE, 0, MAKELPARAM(255, 0));

		thisPtr->LoadTanList();
		thisPtr->m_bRefreshingColorBoxes = false;
		return TRUE;
		break; }
	case WM_COMMAND: {
		TSDialog* thisPtr = (TSDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		if (thisPtr == NULL) return FALSE;
		switch (HIWORD(wparam)) {
		case BN_CLICKED: {
			DWORD identifier = LOWORD(wparam);
			if (identifier == IDC_BD_CBTANCOLOR) {
				BOOL visible = SendMessage(thisPtr->m_cbTanColor, BM_GETCHECK, 0, 0) == BST_CHECKED;
				g_currChar.m_cardData.SetHasTanColor(visible == TRUE);
				thisPtr->Refresh();
				return TRUE;
			}
		}
		case CBN_SELCHANGE: {
			int sel = SendMessage(thisPtr->m_cbSelect, CB_GETCURSEL, 0, 0);
			TCHAR name[256];
			name[0] = '\0';
			if (sel) {
				SendMessage(thisPtr->m_cbSelect, CB_GETLBTEXT, sel, (LPARAM)name);
			}
			g_currChar.m_cardData.SetTan(name);
			//redraw tan
			ExtVars::AAEdit::RedrawBodyPart(ExtVars::AAEdit::BODY_COLOR, ExtVars::AAEdit::BODYCOLOR_TAN);
			break; }
		case EN_CHANGE: {
			HWND ed = (HWND)lparam;
			if (ed == thisPtr->m_edTanColorRed
				|| ed == thisPtr->m_edTanColorGreen
				|| ed == thisPtr->m_edTanColorBlue)
			{
				if (thisPtr->m_bRefreshingColorBoxes) break;
				int newval = General::GetEditInt(ed);
				if (newval < 0) {
					SendMessage(ed, WM_SETTEXT, 0, (LPARAM)TEXT("0"));
				}
				else if (newval > 255) {
					SendMessage(ed, WM_SETTEXT, 0, (LPARAM)TEXT("255"));
				}
				else {
					int red = General::GetEditInt(thisPtr->m_edTanColorRed);
					int green = General::GetEditInt(thisPtr->m_edTanColorGreen);
					int blue = General::GetEditInt(thisPtr->m_edTanColorBlue);
					auto color = RGB(red, green, blue);
					g_currChar.m_cardData.SetTanColor(color);
					ExtVars::AAEdit::RedrawBodyPart(ExtVars::AAEdit::BODY_COLOR, ExtVars::AAEdit::BODYCOLOR_SKINTONE);
				}
			}
			break;}
		}
	}
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
	LRESULT i = SendMessage(m_cbSelect, CB_FINDSTRINGEXACT, -1, (LPARAM)name.c_str());
	SendMessage(m_cbSelect, CB_SETCURSEL, i == CB_ERR ? 0 : i, NULL);

	TCHAR text[10];
	bool bTan = g_currChar.m_cardData.HasTanColor();
	EnableWindow(this->m_edTanColorRed, bTan);
	EnableWindow(this->m_edTanColorGreen, bTan);
	EnableWindow(this->m_edTanColorBlue, bTan);
	COLORREF tanColor = g_currChar.m_cardData.GetTanColor();
	BOOL visible = g_currChar.m_cardData.HasTanColor();
	m_bRefreshingColorBoxes = true;
	SendMessage(this->m_cbTanColor, BM_SETCHECK, visible ? BST_CHECKED : BST_UNCHECKED, 0);
	_itow_s(GetRValue(tanColor), text, 10);
	SendMessage(this->m_edTanColorRed, WM_SETTEXT, 0, (LPARAM)text);
	_itow_s(GetGValue(tanColor), text, 10);
	SendMessage(this->m_edTanColorGreen, WM_SETTEXT, 0, (LPARAM)text);
	_itow_s(GetBValue(tanColor), text, 10);
	SendMessage(this->m_edTanColorBlue, WM_SETTEXT, 0, (LPARAM)text);
	m_bRefreshingColorBoxes = false;
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
				if (g_currChar.Editable()) {


					if (AAEdit::g_currChar.m_char->m_charData->m_gender == 1) {

						BYTE kind;
						for (kind = 0; kind < 4; kind++) {
							if (SendMessage(thisPtr->m_rbKind[kind], BM_GETCHECK, 0, 0) == BST_CHECKED) break;
						}
						if (kind == 4) kind = 0;
						TCHAR buf[256];
						SendMessage(thisPtr->m_edSlot, WM_GETTEXT, 256, (LPARAM)buf);
						BYTE slot = _wtoi(buf);
						SendMessage(thisPtr->m_edAdjustment, WM_GETTEXT, 256, (LPARAM)buf);
						BYTE adjustment = _wtoi(buf);
						bool flip = SendMessage(thisPtr->m_cbFlip, BM_GETCHECK, 0, 0) == BST_CHECKED;
						g_currChar.m_cardData.AddHair(kind, slot, adjustment, flip);
						thisPtr->Refresh();
						//redraw hair of added king
						using namespace ExtVars::AAEdit;
						if (kind == 0) RedrawBodyPart(HAIR, HAIR_FRONT);
						else if (kind == 1) RedrawBodyPart(HAIR, HAIR_SIDE);
						else if (kind == 2) RedrawBodyPart(HAIR, HAIR_BACK);
						else if (kind == 3) RedrawBodyPart(HAIR, HAIR_EXTENSION);
						return TRUE;
					}
					else {
						MessageBox(NULL, TEXT("Extra hair does not work on boys."), TEXT("Error"), 0);
						return TRUE;
					}
				}
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
		thisPtr->m_edOutlineColorHue = GetDlgItem(hwndDlg, IDC_BD_EDOUTLINECOLOR_HUE);
		thisPtr->m_edOutlineColorSat = GetDlgItem(hwndDlg, IDC_BD_EDOUTLINECOLOR_SAT);
		thisPtr->m_edOutlineColorVal = GetDlgItem(hwndDlg, IDC_BD_EDOUTLINECOLOR_VAL);

		thisPtr->m_bmBtnAdd = GetDlgItem(hwndDlg,IDC_BD_BM_BTNADD);
		thisPtr->m_bmCbXXFile = GetDlgItem(hwndDlg,IDC_BD_BM_CBXXFILE);
		thisPtr->m_bmCbBone = GetDlgItem(hwndDlg, IDC_BD_BM_CBBONE);
		thisPtr->m_bmCbMaterial = GetDlgItem(hwndDlg,IDC_BD_BM_CBMAT);
		thisPtr->m_bmList = GetDlgItem(hwndDlg,IDC_BD_BM_LIST);
		thisPtr->m_bmSMList = GetDlgItem(hwndDlg, IDC_BD_SM_LIST);
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
		thisPtr->m_bmRbSMOL = GetDlgItem(hwndDlg, IDC_BD_BM_RBSMOL);
		thisPtr->m_bmRbSMSH = GetDlgItem(hwndDlg, IDC_BD_BM_RBSMSH);
		thisPtr->m_edSubmeshColorRed = GetDlgItem(hwndDlg, IDC_BD_EDSMCOLOR_RED);
		thisPtr->m_edSubmeshColorGreen = GetDlgItem(hwndDlg, IDC_BD_EDSMCOLOR_GREEN);
		thisPtr->m_edSubmeshColorBlue = GetDlgItem(hwndDlg, IDC_BD_EDSMCOLOR_BLUE);
		thisPtr->m_edSubmeshColorHue = GetDlgItem(hwndDlg, IDC_BD_EDSMCOLOR_HUE);
		thisPtr->m_edSubmeshColorSat = GetDlgItem(hwndDlg, IDC_BD_EDSMCOLOR_SAT);
		thisPtr->m_edSubmeshColorVal = GetDlgItem(hwndDlg, IDC_BD_EDSMCOLOR_VAL);
		thisPtr->m_edSubmeshColorAT = GetDlgItem(hwndDlg, IDC_BD_EDSMCOLOR_AT);

		thisPtr->m_edSubmeshColorSH1 = GetDlgItem(hwndDlg, IDC_BD_EDSMCOLOR_SH1);
		thisPtr->m_edSubmeshColorSH2 = GetDlgItem(hwndDlg, IDC_BD_EDSMCOLOR_SH2);

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				SendMessage(thisPtr->m_bmEdMatrix[i][j],WM_SETTEXT,0,(LPARAM)TEXT("0"));
			}
		}
		
		SendMessage(GetDlgItem(hwndDlg,IDC_BD_SPINRED),UDM_SETRANGE,0,MAKELPARAM(255,0));
		SendMessage(GetDlgItem(hwndDlg,IDC_BD_SPINGREEN),UDM_SETRANGE,0,MAKELPARAM(255,0));
		SendMessage(GetDlgItem(hwndDlg,IDC_BD_SPINBLUE),UDM_SETRANGE,0,MAKELPARAM(255,0));

		SendMessage(GetDlgItem(hwndDlg, IDC_BD_SPINSMRED), UDM_SETRANGE, 0, MAKELPARAM(255, 0));
		SendMessage(GetDlgItem(hwndDlg, IDC_BD_SPINSMGREEN), UDM_SETRANGE, 0, MAKELPARAM(255, 0));
		SendMessage(GetDlgItem(hwndDlg, IDC_BD_SPINSMBLUE), UDM_SETRANGE, 0, MAKELPARAM(255, 0));

		SendMessage(GetDlgItem(hwndDlg, IDC_BD_SPINSMAT), UDM_SETRANGE, 0, MAKELPARAM(1, 0));
		SendMessage(GetDlgItem(hwndDlg, IDC_BD_SPINSMSH1), UDM_SETRANGE, 0, MAKELPARAM(1, 0));
		SendMessage(GetDlgItem(hwndDlg, IDC_BD_SPINSMSH2), UDM_SETRANGE, 0, MAKELPARAM(1, 0));

		SendMessage(GetDlgItem(hwndDlg,IDC_BD_BM_RBFRAME),BM_SETCHECK,BST_CHECKED,0);

		return TRUE; }
	case WM_VKEYTOITEM: {
		//DEL-key was pressed while the list box had the focus.
		BDDialog* thisPtr = (BDDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		if (thisPtr == NULL) return FALSE;
		HWND wnd = (HWND)lparam;
		if (LOWORD(wparam) == VK_DELETE) {
			if (wnd == thisPtr->m_bmList) {
				//get current selection text
				int sel = SendMessage(thisPtr->m_bmList, LB_GETCURSEL, 0, 0);
				if (sel == LB_ERR) return TRUE;
				//remove this rule
				g_currChar.m_cardData.RemoveBoneRule(sel);
				thisPtr->Refresh();
				return TRUE;
			}
			else if (wnd == thisPtr->m_bmSMList) {
				//get current selection text
				int sel = SendMessage(thisPtr->m_bmSMList, LB_GETCURSEL, 0, 0);
				if (sel == LB_ERR) return TRUE;
				//remove this rule

				TCHAR text[10];
				std::wstring listEntry;
				int strLen = (int)SendMessage(thisPtr->m_bmSMList, LB_GETTEXTLEN, (WPARAM)sel, 0);
				TCHAR * textBuffer = new TCHAR[strLen + 1];
				SendMessage(thisPtr->m_bmSMList, LB_GETTEXT, (WPARAM)sel, (LPARAM)textBuffer);
				listEntry = textBuffer;
				int flag = 0;
				if (General::StartsWith(listEntry, L"[SMOL]")) {
					flag |= AAUCardData::SUBMESH_OUTLINE;
				}
				else if (General::StartsWith(listEntry, L"[SMSH]")) {
					flag |= AAUCardData::SUBMESH_SHADOW;
				}
				g_currChar.m_cardData.RemoveSubmeshRule(sel, (AAUCardData::MeshModFlag)flag);
				thisPtr->Refresh();
				return TRUE;
			}
		}
		break; }
	case WM_COMMAND: {
		BDDialog* thisPtr = (BDDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
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
			return TRUE; }
		case EN_CHANGE: {
			HWND ed = (HWND)lparam;
			if (ed == thisPtr->m_edOutlineColorBlue
				|| ed == thisPtr->m_edOutlineColorGreen
				|| ed == thisPtr->m_edOutlineColorRed)
			{
				int newval = General::GetEditInt(ed);
				if (newval < 0) {
					SendMessage(ed, WM_SETTEXT, 0, (LPARAM)TEXT("0"));
				}
				else if (newval > 255) {
					SendMessage(ed, WM_SETTEXT, 0, (LPARAM)TEXT("255"));
				}
				else {
					int red = General::GetEditInt(thisPtr->m_edOutlineColorRed);
					int green = General::GetEditInt(thisPtr->m_edOutlineColorGreen);
					int blue = General::GetEditInt(thisPtr->m_edOutlineColorBlue);
					g_currChar.m_cardData.SetOutlineColor(RGB(red, green, blue));
				}
			}
			else if (ed == thisPtr->m_edSubmeshColorRed
				|| ed == thisPtr->m_edSubmeshColorGreen
				|| ed == thisPtr->m_edSubmeshColorBlue)
			{
				int newval = General::GetEditInt(ed);
				if (newval < 0) {
					SendMessage(ed, WM_SETTEXT, 0, (LPARAM)TEXT("0"));
				}
				else if (newval > 255) {
					SendMessage(ed, WM_SETTEXT, 0, (LPARAM)TEXT("255"));
				}
				//else {
				//	int red = General::GetEditInt(thisPtr->m_edSubmeshColorRed);
				//	int green = General::GetEditInt(thisPtr->m_edSubmeshColorGreen);
				//	int blue = General::GetEditInt(thisPtr->m_edSubmeshColorBlue);
				//	std::vector<BYTE> color{(BYTE)red, (BYTE)green, (BYTE)blue, 255};
				//}
			}
			else if (ed == thisPtr->m_edSubmeshColorAT || 
				ed == thisPtr->m_edSubmeshColorSH1 ||
				ed == thisPtr->m_edSubmeshColorSH2) {
				auto selection = Edit_GetSel(ed);
				TCHAR num[128];
				SendMessage(ed, WM_GETTEXT, 128, (LPARAM)num);
				float f = wcstof(num, NULL); //returns 0 on errornous string, so invalid stuff will just turn to a 0
				if (f < 0) f = 0;
				TCHAR str[128];
				swprintf_s(str, L"%g", f);
				//if the value was changed, take the new one
				int unequal = strcmp(General::CastToString(str).c_str(), General::CastToString(num).c_str());
				//and do nothing with all of it
				//if (unequal) {
				//	SendMessage(ed, WM_SETTEXT, 0, (LPARAM)str);
				//}
				Edit_SetSel(ed, LOWORD(selection), HIWORD(selection));
			}
			return TRUE;
		}
		case EN_KILLFOCUS: {
			HWND ed = (HWND)lparam;
			if (ed == thisPtr->m_edSubmeshColorAT ||
				ed == thisPtr->m_edSubmeshColorSH1 ||
				ed == thisPtr->m_edSubmeshColorSH2) {
				auto selection = Edit_GetSel(ed);
				TCHAR num[128];
				SendMessage(ed, WM_GETTEXT, 128, (LPARAM)num);
				float f = wcstof(num, NULL); //returns 0 on errornous string, so invalid stuff will just turn to a 0
				if (f < 0) f = 0;
				TCHAR str[128];
				swprintf_s(str, L"%g", f);
				//if the value was changed, take the new one
				int unequal = strcmp(General::CastToString(str).c_str(), General::CastToString(num).c_str());
				if (unequal) {
					SendMessage(ed, WM_SETTEXT, 0, (LPARAM)str);
				}
				Edit_SetSel(ed, LOWORD(selection), HIWORD(selection));
			}
			else {
				auto selection = Edit_GetSel(ed);
				TCHAR num[128];
				SendMessage(ed, WM_GETTEXT, 128, (LPARAM)num);
				float f = wcstof(num, NULL); //returns 0 on errornous string, so invalid stuff will just turn to a 0
				TCHAR str[128];
				swprintf_s(str, L"%g", f);
				//if the value was changed, take the new one
				int unequal = strcmp(General::CastToString(str).c_str(), General::CastToString(num).c_str());
				if (unequal) {
					SendMessage(ed, WM_SETTEXT, 0, (LPARAM)str);
				}
				Edit_SetSel(ed, LOWORD(selection), HIWORD(selection));
			}
			
			return TRUE; }
		case LBN_SELCHANGE: {
			HWND wnd = (HWND)lparam;
			if(wnd == thisPtr->m_bmList) {
				int sel = SendMessage(thisPtr->m_bmList,LB_GETCURSEL,0,0);
				if(sel != LB_ERR) {
					thisPtr->LoadMatrixData(sel);
				}
				return TRUE;
			}
			else if (wnd == thisPtr->m_bmSMList) {
				int sel = SendMessage(thisPtr->m_bmSMList, LB_GETCURSEL, 0, 0);
				if (sel != LB_ERR) {
					thisPtr->LoadColorData(sel);
				}
				return TRUE;
			}
			break; }
		};
		break; }
	case WM_NOTIFY: {
		BDDialog* thisPtr = (BDDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		if (thisPtr == NULL) return FALSE;
		auto ncode = ((LPNMHDR)lparam)->code;
		auto sender = HWND(lparam);
		switch (ncode) {
		case UDN_DELTAPOS:
			switch (wparam) {
			case IDC_BD_SPINSMAT: {
				auto lpnmud = (LPNMUPDOWN)lparam;
				float value = General::GetEditFloat(thisPtr->m_edSubmeshColorAT);
				float tick = SendMessage(thisPtr->m_bmRbSMOL, BM_GETCHECK, 0, 0) == BST_CHECKED ? 0.0001f : 0.01f;
				value += lpnmud->iDelta * tick;
				if (value < 0) value = 0;
				if (value > 0.01 && SendMessage(thisPtr->m_bmRbSMOL, BM_GETCHECK, 0, 0) == BST_CHECKED) value = 0.01f;
				TCHAR num[128];
				auto formatted = swprintf_s(num, L"%g", value);
				SendMessage(thisPtr->m_edSubmeshColorAT, WM_SETTEXT, 0, (LPARAM)num);
				break; }
			case IDC_BD_SPINSMSH1: {
				auto lpnmud = (LPNMUPDOWN)lparam;
				float value = General::GetEditFloat(thisPtr->m_edSubmeshColorSH1);
				float tick = SendMessage(thisPtr->m_bmRbSMOL, BM_GETCHECK, 0, 0) == BST_CHECKED ? 0.01f : 0.01f;
				value += lpnmud->iDelta * tick;
				if (value < 0) value = 0;
				TCHAR num[128];
				auto formatted = swprintf_s(num, L"%g", value);
				SendMessage(thisPtr->m_edSubmeshColorSH1, WM_SETTEXT, 0, (LPARAM)num);
				break; }
			case IDC_BD_SPINSMSH2: {
				auto lpnmud = (LPNMUPDOWN)lparam;
				float value = General::GetEditFloat(thisPtr->m_edSubmeshColorSH2);
				float tick = SendMessage(thisPtr->m_bmRbSMOL, BM_GETCHECK, 0, 0) == BST_CHECKED ? 0.0001f : 0.0001f;
				value += lpnmud->iDelta * tick;
				if (value < 0) value = 0;
				TCHAR num[128];
				auto formatted = swprintf_s(num, L"%g", value);
				SendMessage(thisPtr->m_edSubmeshColorSH2, WM_SETTEXT, 0, (LPARAM)num);
				break; }
			}
			if (thisPtr->IsSubmeshRuleSelected()) {
				thisPtr->ApplySubmeshRule(true);
			}
		}

		break; }

	};
	return FALSE;
}

bool UnlimitedDialog::BDDialog::IsSubmeshRuleSelected() {
	int sel = SendMessage(this->m_bmSMList, LB_GETCURSEL, 0, 0);

	return (sel != LB_ERR);
}

void UnlimitedDialog::BDDialog::LoadMatrixData(int listboxId) {
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

void UnlimitedDialog::BDDialog::LoadColorData(int listboxId) {
	TCHAR text[10];
	std::wstring listEntry;
	int strLen = (int) SendMessage(m_bmSMList, LB_GETTEXTLEN, (WPARAM)listboxId, 0);
	TCHAR * textBuffer = new TCHAR[strLen + 1];
	SendMessage(m_bmSMList, LB_GETTEXT, (WPARAM)listboxId, (LPARAM)textBuffer);

	listEntry = textBuffer;

	//first.first.first - file name
	//first.first.second - frame name
	//first.second - material name
	//second - color
	union {
		DWORD i;
		float f;
	} floatyDWORD;
	if (General::StartsWith(listEntry, L"[SMOL]")) {
		SendMessage(m_bmRbBoneMod, BM_SETCHECK, BST_UNCHECKED, 0);
		SendMessage(m_bmRbFrameMod, BM_SETCHECK, BST_UNCHECKED, 0);
		SendMessage(m_bmRbSMSH, BM_SETCHECK, BST_UNCHECKED, 0);
		SendMessage(m_bmRbSMOL, BM_SETCHECK, BST_CHECKED, 0);

		if (listboxId >= g_currChar.m_cardData.m_styles[g_currChar.GetCurrentStyle()].m_submeshOutlines.size()) return;
		auto rule = g_currChar.m_cardData.m_styles[g_currChar.GetCurrentStyle()].m_submeshOutlines[listboxId];
		SendMessage(m_bmCbXXFile, WM_SETTEXT, 0, (LPARAM)rule.first.first.first.c_str());
		SendMessage(m_bmCbBone, WM_SETTEXT, 0, (LPARAM)rule.first.first.second.c_str());
		SendMessage(m_bmCbMaterial, WM_SETTEXT, 0, (LPARAM)rule.first.second.c_str());

		auto submeshOutlineColor = rule.second;
		_itow_s(submeshOutlineColor[0], text, 10);
		SendMessage(this->m_edSubmeshColorRed, WM_SETTEXT, 0, (LPARAM)text);
		_itow_s(submeshOutlineColor[1], text, 10);
		SendMessage(this->m_edSubmeshColorGreen, WM_SETTEXT, 0, (LPARAM)text);
		_itow_s(submeshOutlineColor[2], text, 10);
		SendMessage(this->m_edSubmeshColorBlue, WM_SETTEXT, 0, (LPARAM)text);
		floatyDWORD.i = submeshOutlineColor[3];
		swprintf_s(text, L"%g", floatyDWORD.f);
		SendMessage(this->m_edSubmeshColorAT, WM_SETTEXT, 0, (LPARAM)text);
	}
	else if (General::StartsWith(listEntry, L"[SMSH]")) {
		SendMessage(m_bmRbBoneMod, BM_SETCHECK, BST_UNCHECKED, 0);
		SendMessage(m_bmRbFrameMod, BM_SETCHECK, BST_UNCHECKED, 0);
		SendMessage(m_bmRbSMOL, BM_SETCHECK, BST_UNCHECKED, 0);
		SendMessage(m_bmRbSMSH, BM_SETCHECK, BST_CHECKED, 0);
		int offst = g_currChar.m_cardData.m_styles[g_currChar.GetCurrentStyle()].m_submeshOutlines.size();
		if (listboxId >= offst + g_currChar.m_cardData.m_styles[g_currChar.GetCurrentStyle()].m_submeshShadows.size()) return;
		auto rule = g_currChar.m_cardData.m_styles[g_currChar.GetCurrentStyle()].m_submeshShadows[listboxId - offst];
		SendMessage(m_bmCbXXFile, WM_SETTEXT, 0, (LPARAM)rule.first.first.first.c_str());
		SendMessage(m_bmCbBone, WM_SETTEXT, 0, (LPARAM)rule.first.first.second.c_str());
		SendMessage(m_bmCbMaterial, WM_SETTEXT, 0, (LPARAM)rule.first.second.c_str());

		auto submeshShadowColor = rule.second;		
		if (submeshShadowColor.size() < 5) {	//couple safeguards
			floatyDWORD.f = 0.6;	//took it from the body mesh
			submeshShadowColor.push_back(floatyDWORD.i);
		}
		if (submeshShadowColor.size() < 6) {
			floatyDWORD.f = 0.0015;	//took it from the body mesh
			submeshShadowColor.push_back(floatyDWORD.i);
		}
		_itow_s(submeshShadowColor[0], text, 10);
		SendMessage(this->m_edSubmeshColorRed, WM_SETTEXT, 0, (LPARAM)text);
		_itow_s(submeshShadowColor[1], text, 10);
		SendMessage(this->m_edSubmeshColorGreen, WM_SETTEXT, 0, (LPARAM)text);
		_itow_s(submeshShadowColor[2], text, 10);
		SendMessage(this->m_edSubmeshColorBlue, WM_SETTEXT, 0, (LPARAM)text);
		floatyDWORD.i = submeshShadowColor[3];
		swprintf_s(text, L"%g", floatyDWORD.f);
		SendMessage(this->m_edSubmeshColorAT, WM_SETTEXT, 0, (LPARAM)text);
		floatyDWORD.i = submeshShadowColor[4];
		swprintf_s(text, L"%g", floatyDWORD.f);
		SendMessage(this->m_edSubmeshColorSH1, WM_SETTEXT, 0, (LPARAM)text);
		floatyDWORD.i = submeshShadowColor[5];
		swprintf_s(text, L"%g", floatyDWORD.f);
		SendMessage(this->m_edSubmeshColorSH2, WM_SETTEXT, 0, (LPARAM)text);
	}

	delete[] textBuffer;
	textBuffer = nullptr;
	//
}

void UnlimitedDialog::BDDialog::ApplySubmeshRule(bool lightRefresh)
{
	int flags = 0;
	bool submeshMod = false;
	if (SendMessage(m_bmRbBoneMod, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		flags |= AAUCardData::MODIFY_BONE;
		submeshMod = false;
	}
	if (SendMessage(m_bmRbFrameMod, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		flags |= AAUCardData::MODIFY_FRAME;
		submeshMod = false;
	}
	if (SendMessage(m_bmRbSMOL, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		flags |= AAUCardData::SUBMESH_OUTLINE;
		submeshMod = true;
	}
	if (SendMessage(m_bmRbSMSH, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		flags |= AAUCardData::SUBMESH_SHADOW;
		submeshMod = true;
	}
	if (!submeshMod) return;

	TCHAR xxname[128];
	SendMessage(m_bmCbXXFile, WM_GETTEXT, 128, (LPARAM)xxname);
	TCHAR bonename[128];
	SendMessage(m_bmCbBone, WM_GETTEXT, 128, (LPARAM)bonename);
	TCHAR materialName[128];
	SendMessage(m_bmCbMaterial, WM_GETTEXT, 128, (LPARAM)materialName);

	int red = General::GetEditInt(m_edSubmeshColorRed);
	int green = General::GetEditInt(m_edSubmeshColorGreen);
	int blue = General::GetEditInt(m_edSubmeshColorBlue);
	union {
		DWORD i;
		float f;
	} floatyDWORDAT;
	union {
		DWORD i;
		float f;
	} floatyDWORDSH1;
	union {
		DWORD i;
		float f;
	} floatyDWORDSH2;
	floatyDWORDAT.f = General::GetEditFloat(m_edSubmeshColorAT);
	floatyDWORDSH1.f = General::GetEditFloat(m_edSubmeshColorSH1);
	floatyDWORDSH2.f = General::GetEditFloat(m_edSubmeshColorSH2);
	std::vector<DWORD> color{ (DWORD)red, (DWORD)green, (DWORD)blue, floatyDWORDAT.i, floatyDWORDSH1.i, floatyDWORDSH2.i };
	if (lightRefresh) {
		ExtClass::CharacterStruct* curr = ExtVars::AAEdit::GetCurrentCharacter();
		if (curr == nullptr) return;
		ExtClass::XXFile* xxlist[] = {
			curr->m_xxFace, curr->m_xxGlasses, curr->m_xxFrontHair, curr->m_xxSideHair,
			curr->m_xxBackHair, curr->m_xxHairExtension, curr->m_xxTounge, curr->m_xxSkeleton,
			curr->m_xxBody, curr->m_xxLegs, curr->m_xxSkirt
		};
		if (curr->m_charData->m_gender == 0) {
			xxlist[10] = NULL;
		}

		for (ExtClass::XXFile* file : xxlist) {
			if (file == NULL) continue;
			{
				auto mesh = std::wstring(xxname);
				auto frame = std::wstring(bonename);
				auto material = std::wstring(materialName);
				auto newMeshSize = mesh.size() % 2 == 0 ? mesh.size() : (mesh.size() + 1);
				auto newFrameSize = frame.size() % 2 == 0 ? frame.size() : (frame.size() + 1);
				auto newMaterialSize = material.size() % 2 == 0 ? material.size() : (material.size() + 1);
				mesh.resize(newMeshSize);
				frame.resize(newFrameSize);
				material.resize(newMaterialSize);
				std::pair<std::pair<std::wstring, std::wstring>, std::wstring> key{ { mesh, frame }, material };

				auto currStyle = (Shared::g_currentChar)->GetCurrentStyle();
				if (flags & AAUCardData::SUBMESH_OUTLINE) {
					auto size = Shared::g_currentChar->m_cardData.m_styles[currStyle].m_submeshOutlines.size();
					for (int i = 0; i < size; i++)
					{
						if (key == (&Shared::g_currentChar->m_cardData.m_styles[currStyle].m_submeshOutlines[i])->first)
						{
							(&Shared::g_currentChar->m_cardData.m_styles[currStyle].m_submeshOutlines[i])->second = color;
							break;
						}
					}
					Shared::FrameSubmeshOutlineOverride(file, false);
				}
				if (flags & AAUCardData::SUBMESH_SHADOW) {
					auto size = Shared::g_currentChar->m_cardData.m_styles[currStyle].m_submeshShadows.size();
					auto shadowsRules = Shared::g_currentChar->m_cardData.m_styles[currStyle].m_submeshShadows;
					for (int i = 0; i < shadowsRules.size(); i++)
					{
						if (key == (&Shared::g_currentChar->m_cardData.m_styles[currStyle].m_submeshShadows[i])->first)
						{
							(&Shared::g_currentChar->m_cardData.m_styles[currStyle].m_submeshShadows[i])->second = color;
							break;
						}
					}
					Shared::FrameSubmeshShadowOverride(file, false);
				}
			}
		}
	}
	else {
		g_currChar.m_cardData.AddSubmeshRule((AAUCardData::MeshModFlag)flags, xxname, bonename, materialName, color);
	}
}

void UnlimitedDialog::BDDialog::ApplyInput() {
	int flags = 0;
	bool submeshMod = false;
	if (SendMessage(m_bmRbBoneMod, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		flags |= AAUCardData::MODIFY_BONE;
		submeshMod = false;
	}
	if (SendMessage(m_bmRbFrameMod, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		flags |= AAUCardData::MODIFY_FRAME;
		submeshMod = false;
	}
	if (SendMessage(m_bmRbSMOL, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		flags |= AAUCardData::SUBMESH_OUTLINE;
		submeshMod = true;
	}
	if (SendMessage(m_bmRbSMSH, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		flags |= AAUCardData::SUBMESH_SHADOW;
		submeshMod = true;
	}
	
	if (submeshMod){
		ApplySubmeshRule(false);
	}
	else {
		TCHAR xxname[128];
		SendMessage(m_bmCbXXFile, WM_GETTEXT, 128, (LPARAM)xxname);
		TCHAR bonename[128];
		SendMessage(m_bmCbBone, WM_GETTEXT, 128, (LPARAM)bonename);
		//TCHAR materialName[128];
		//SendMessage(m_bmCbMaterial, WM_GETTEXT, 128, (LPARAM)materialName);

		//remove transformation if it allready exists
		const auto& vec = g_currChar.m_cardData.GetMeshRuleList();
		unsigned int match;
		for (match = 0; match < vec.size(); match++) {
			if (vec[match].first.first == flags && vec[match].first.second.first == xxname && vec[match].first.second.second == bonename) break;
		}
		if (match < vec.size()) g_currChar.m_cardData.RemoveBoneRule(match);

		//get matrix
		AAUCardData::BoneMod mod;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				TCHAR num[128];
				SendMessage(m_bmEdMatrix[i][j], WM_GETTEXT, 128, (LPARAM)num);
				float f = wcstof(num, NULL);
				mod.mods[i][j] = f;
			}
		}
		//save
		if (g_currChar.Editable()) {
			g_currChar.m_cardData.AddBoneRule((AAUCardData::MeshModFlag)flags, xxname, bonename, mod);
			auto clip = g_currChar.m_char->m_xxSkeleton->m_poseNumber;
			g_currChar.m_char->Spawn(g_currChar.m_char->m_clothState, g_currChar.m_char->m_materialSlot, 0, 1);
			auto pp = L"data\\jg2e01_00_00.pp";
			wchar_t xa[255];
			auto strGender = g_currChar.m_char->m_charData->m_gender == 0 ? L"S" : L"A";
			int figure = g_currChar.m_char->m_charData->m_figure.height;
			if (g_currChar.m_char->m_charData->m_gender == 0) {	//male
				figure = g_currChar.m_char->m_charData->m_figure.height + g_currChar.m_char->m_charData->m_figure.figure - 1;
				//switch (g_currChar.m_char->m_charData->m_figure.height)
				//{
				//case 1:
				//	if (g_currChar.m_char->m_charData->m_figure.figure == 0) {
				//		//Delicate
				//		figure = 00;
				//	}
				//	if (g_currChar.m_char->m_charData->m_figure.figure == 1) {
				//		//Normal
				//		figure = 01;
				//	}
				//case 2:
				//	if (g_currChar.m_char->m_charData->m_figure.figure == 1) {
				//		//Tall
				//		figure = 02;
				//	}
				//	if (g_currChar.m_char->m_charData->m_figure.figure == 2) {
				//		//Fat
				//		figure = 03;
				//	}
				//}
			}
			swprintf(xa, L"data\\H%sE00_00_%02d_00.xa", strGender, figure);
			g_currChar.m_char->LoadXA(pp, xa, clip, 0, 0);
		}
	}
	Refresh();

}

void UnlimitedDialog::BDDialog::Refresh() {
	TCHAR text[10];

	bool bGlobalOutline = g_currChar.m_cardData.HasOutlineColor();	
	EnableWindow(this->m_edOutlineColorRed,bGlobalOutline);
	EnableWindow(this->m_edOutlineColorGreen,bGlobalOutline);
	EnableWindow(this->m_edOutlineColorBlue,bGlobalOutline);
	COLORREF globalOutlineColor = g_currChar.m_cardData.GetOutlineColor();
	_itow_s(GetRValue(globalOutlineColor),text,10);
	SendMessage(this->m_edOutlineColorRed,WM_SETTEXT,0,(LPARAM)text);
	_itow_s(GetGValue(globalOutlineColor),text,10);
	SendMessage(this->m_edOutlineColorGreen,WM_SETTEXT,0,(LPARAM)text);
	_itow_s(GetBValue(globalOutlineColor),text,10);
	SendMessage(this->m_edOutlineColorBlue,WM_SETTEXT,0,(LPARAM)text);



	TCHAR xxname[128];
	SendMessage(m_bmCbXXFile, WM_GETTEXT, 128, (LPARAM)xxname);
	TCHAR bonename[128];
	SendMessage(m_bmCbBone, WM_GETTEXT, 128, (LPARAM)bonename);
	TCHAR materialName[128];
	SendMessage(m_bmCbMaterial, WM_GETTEXT, 128, (LPARAM)materialName);
	bool bSubmeshOutline = SendMessage(m_bmRbSMOL, BM_GETCHECK, 0, 0) == BST_CHECKED;
	bool bSubmeshShadow = SendMessage(m_bmRbSMSH, BM_GETCHECK, 0, 0) == BST_CHECKED;
	//EnableWindow(this->m_edSubmeshColorRed, bSubmesh);
	//EnableWindow(this->m_edSubmeshColorGreen, bSubmesh);
	//EnableWindow(this->m_edSubmeshColorBlue, bSubmesh);
	union {
		DWORD i;
		float f;
	} floatyDWORD;
	if (bSubmeshOutline) {
		auto submeshOutlineColor = g_currChar.m_cardData.GetSubmeshOutlineColor(xxname, bonename, materialName);
		_itow_s(submeshOutlineColor[0], text, 10);
		SendMessage(this->m_edSubmeshColorRed, WM_SETTEXT, 0, (LPARAM)text);
		_itow_s(submeshOutlineColor[1], text, 10);
		SendMessage(this->m_edSubmeshColorGreen, WM_SETTEXT, 0, (LPARAM)text);
		_itow_s(submeshOutlineColor[2], text, 10);
		SendMessage(this->m_edSubmeshColorBlue, WM_SETTEXT, 0, (LPARAM)text);
		floatyDWORD.i = submeshOutlineColor[3];
		swprintf_s(text, L"%g", floatyDWORD.f);
		SendMessage(this->m_edSubmeshColorAT, WM_SETTEXT, 0, (LPARAM)text);
	}
	else if (bSubmeshShadow) {
		auto submeshShadowColor = g_currChar.m_cardData.GetSubmeshShadowColor(xxname, bonename, materialName);
		_itow_s(submeshShadowColor[0], text, 10);
		SendMessage(this->m_edSubmeshColorRed, WM_SETTEXT, 0, (LPARAM)text);
		_itow_s(submeshShadowColor[1], text, 10);
		SendMessage(this->m_edSubmeshColorGreen, WM_SETTEXT, 0, (LPARAM)text);
		_itow_s(submeshShadowColor[2], text, 10);
		SendMessage(this->m_edSubmeshColorBlue, WM_SETTEXT, 0, (LPARAM)text);
		
		floatyDWORD.i = submeshShadowColor[3];
		swprintf_s(text, L"%g", floatyDWORD.f);
		SendMessage(this->m_edSubmeshColorAT, WM_SETTEXT, 0, (LPARAM)text);
		floatyDWORD.i = submeshShadowColor[4];
		swprintf_s(text, L"%g", floatyDWORD.f);
		SendMessage(this->m_edSubmeshColorSH1, WM_SETTEXT, 0, (LPARAM)text);
		floatyDWORD.i = submeshShadowColor[5];
		swprintf_s(text, L"%g", floatyDWORD.f);
		SendMessage(this->m_edSubmeshColorSH2, WM_SETTEXT, 0, (LPARAM)text);
	}
	
	//submesh mods listbox
	SendMessage(this->m_bmSMList, LB_RESETCONTENT, 0, 0);
	const auto& submeshOutlinesList = AAEdit::g_currChar.m_cardData.m_styles[AAEdit::g_currChar.m_cardData.m_currCardStyle].m_submeshOutlines;
	for (size_t i = 0; i < submeshOutlinesList.size(); i++) {
		//first.first.first - file name
		//first.first.second - frame name
		//first.second - material name
		//second - color
		std::wstring listEntry(TEXT("["));
		listEntry += TEXT("SMOL");
		listEntry += TEXT("] Fi: ");
		wchar_t terminator = '\0';
		auto clnFilename = submeshOutlinesList[i].first.first.first.substr(0, submeshOutlinesList[i].first.first.first.find_first_of(terminator));
		auto clnFramename = submeshOutlinesList[i].first.first.second.substr(0, submeshOutlinesList[i].first.first.second.find_first_of(terminator));
		auto clnMaterialname = submeshOutlinesList[i].first.second.substr(0, submeshOutlinesList[i].first.second.find_first_of(terminator));
		listEntry += clnFilename + TEXT("| Fr: ") + clnFramename + TEXT("| Ma:") + clnMaterialname;
		SendMessage(this->m_bmSMList, LB_ADDSTRING, 0, (LPARAM)listEntry.c_str());
	}
	const auto& submeshShadowsList = AAEdit::g_currChar.m_cardData.m_styles[AAEdit::g_currChar.m_cardData.m_currCardStyle].m_submeshShadows;
	for (size_t i = 0; i < submeshShadowsList.size(); i++) {
		//first.first.first - file name
		//first.first.second - frame name
		//first.second - material name
		//second - color
		std::wstring listEntry(TEXT("["));
		listEntry += TEXT("SMSH");
		listEntry += TEXT("] Fi: ");
		wchar_t terminator = '\0';
		auto clnFilename = submeshShadowsList[i].first.first.first.substr(0, submeshShadowsList[i].first.first.first.find_first_of(terminator));
		auto clnFramename = submeshShadowsList[i].first.first.second.substr(0, submeshShadowsList[i].first.first.second.find_first_of(terminator));
		auto clnMaterialname = submeshShadowsList[i].first.second.substr(0, submeshShadowsList[i].first.second.find_first_of(terminator));
		listEntry += clnFilename + TEXT("| Fr: ") + clnFramename + TEXT("| Ma: ") + clnMaterialname;
		SendMessage(this->m_bmSMList, LB_ADDSTRING, 0, (LPARAM)listEntry.c_str());
	}

	//frame mods listbox
	SendMessage(this->m_bmList,LB_RESETCONTENT,0,0);
	const auto& list = AAEdit::g_currChar.m_cardData.GetMeshRuleList();
	for (size_t i = 0; i < list.size(); i++) {
		std::wstring listEntry(TEXT("["));
		if (list[i].first.first & AAUCardData::MODIFY_BONE) listEntry += TEXT("BONE");
		else listEntry += TEXT("FRAME");
		listEntry += TEXT("] Fi: ");
		listEntry += list[i].first.second.first + TEXT("| Fr: ") + list[i].first.second.second;
		SendMessage(this->m_bmList,LB_INSERTSTRING,i,(LPARAM)listEntry.c_str());
	}
	//list possible bones
	ExtClass::CharacterStruct* curr = ExtVars::AAEdit::GetCurrentCharacter();
	if(curr != NULL) {
		SendMessage(m_bmCbBone, CB_RESETCONTENT, 0, 0);
		ExtClass::XXFile* xxlist[] = {
			curr->m_xxFace, curr->m_xxGlasses, curr->m_xxFrontHair, curr->m_xxSideHair,
			curr->m_xxBackHair, curr->m_xxHairExtension, curr->m_xxTounge, curr->m_xxSkeleton,
			curr->m_xxBody, curr->m_xxLegs, curr->m_xxSkirt
		};
		if (curr->m_charData->m_gender == 0) {
			xxlist[10] = NULL;
		}
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

				if (SendMessage(m_bmCbBone, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)tmpBuff) == CB_ERR) {	//if a frame with this name is not already present in the list
					SendMessage(m_bmCbBone, CB_ADDSTRING, 0, (LPARAM)tmpBuff);
				}
				for (unsigned int i = 0; i < bone->m_nChildren; i++) {
					fileQueue.push(bone->m_children + i);
				}
			}
		}
		auto selIdx = SendMessage(m_bmCbBone, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)bonename);
		if (selIdx == CB_ERR) {
			SendMessage(m_bmCbBone, CB_ADDSTRING, 0, (LPARAM)bonename);
			SendMessage(m_bmCbBone, CB_SELECTSTRING, -1, (LPARAM)bonename);
		}
		else {
			SendMessage(m_bmCbBone, CB_SETCURSEL, (WPARAM)selIdx, 0);
		}
	}
	
	//list loaded files
	if (curr != NULL) {
		SendMessage(m_bmCbXXFile, CB_RESETCONTENT, 0, 0);
		ExtClass::XXFile* xxlist[] = {
			curr->m_xxFace, curr->m_xxGlasses, curr->m_xxFrontHair, curr->m_xxSideHair,
			curr->m_xxBackHair, curr->m_xxHairExtension, curr->m_xxTounge, curr->m_xxSkeleton,
			curr->m_xxBody, curr->m_xxLegs, curr->m_xxSkirt
		};
		if (curr->m_charData->m_gender == 0) {
			xxlist[10] = NULL;
		}
		TCHAR tmpBuff[512];
		for (ExtClass::XXFile* file : xxlist) {
			if (file == NULL) continue;
			size_t conv;
			mbstowcs_s(&conv, tmpBuff, file->m_name, 512);

			if (SendMessage(m_bmCbXXFile, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)tmpBuff) == CB_ERR) {	//if a file with this name is not already present in the list
				SendMessage(m_bmCbXXFile, CB_ADDSTRING, 0, (LPARAM)tmpBuff);
			}

			Shared::FrameSubmeshOutlineOverride(file, false);
			Shared::FrameSubmeshShadowOverride(file, false);
		}
		auto selIdx = SendMessage(m_bmCbXXFile, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)xxname);
		if (selIdx == CB_ERR) {
			SendMessage(m_bmCbXXFile, CB_ADDSTRING, 0, (LPARAM)xxname);
			SendMessage(m_bmCbXXFile, CB_SELECTSTRING, -1, (LPARAM)xxname);
		}
		else {
			SendMessage(m_bmCbXXFile, CB_SETCURSEL, (WPARAM)selIdx, 0);
		}
	}

	//list loaded materials
	if (curr != NULL) {
		SendMessage(m_bmCbMaterial, CB_RESETCONTENT, 0, 0);
		ExtClass::XXFile* xxlist[] = {
			curr->m_xxFace, curr->m_xxGlasses, curr->m_xxFrontHair, curr->m_xxSideHair,
			curr->m_xxBackHair, curr->m_xxHairExtension, curr->m_xxTounge, curr->m_xxSkeleton,
			curr->m_xxBody, curr->m_xxLegs, curr->m_xxSkirt
		};
		if (curr->m_charData->m_gender == 0) {
			xxlist[10] = NULL;
		}

		for (ExtClass::XXFile* file : xxlist) {
			if (file == NULL) continue;
			for (int i = 0; i < file->m_materialCount; i++) {
				TCHAR tmpBuff[512];
				size_t conv;
				mbstowcs_s(&conv, tmpBuff, file->m_materialArray[i].m_name, file->m_materialArray[i].m_nameLength);
				if (SendMessage(m_bmCbMaterial, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)tmpBuff) == CB_ERR) {	//if a material with this name is not already present in the list
					SendMessage(m_bmCbMaterial, CB_ADDSTRING, 0, (LPARAM)tmpBuff);
				}
			}
		}
		auto selIdx = SendMessage(m_bmCbMaterial, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)materialName);
		if (selIdx == CB_ERR) {
			SendMessage(m_bmCbMaterial, CB_ADDSTRING, 0, (LPARAM)materialName);
			SendMessage(m_bmCbMaterial, CB_SELECTSTRING, -1, (LPARAM)materialName);
		}
		else {
			SendMessage(m_bmCbMaterial, CB_SETCURSEL, (WPARAM)selIdx, 0);
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
		ShowWindow(GetDlgItem(hwndDlg,IDC_BS_BTNRST), SW_HIDE);

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
			{ TEXT("Body Size"),
				{ { CharacterStruct::SKELETON, 55 } },
				-0.9f, 1.0f
			},
			{ TEXT("Chest Size"),
				{ { CharacterStruct::BODY, 14 }, { CharacterStruct::SKELETON, 42 },
				{ CharacterStruct::SKELETON, 43 } ,{ CharacterStruct::SKELETON, 44 },
				{ CharacterStruct::SKIRT, 1 }, },
				-0.6f, 0.6f
			},
			{ TEXT("Chest Width"), //Split from Chest Size
				{ { CharacterStruct::BODY, 39 },{ CharacterStruct::SKELETON, 68 },
				{ CharacterStruct::SKELETON, 69 } ,{ CharacterStruct::SKIRT, 39 }, },
				-0.6f, 0.6f
			},
			{ TEXT("Chest Thickness"), //Split from Chest Size
				{ { CharacterStruct::BODY, 40 },{ CharacterStruct::SKELETON, 70 },
				{ CharacterStruct::SKELETON, 71 } ,{ CharacterStruct::SKELETON, 72 },
				{ CharacterStruct::SKIRT, 40 }, },
				-0.6f, 0.6f
			},
			{ TEXT("Chest Height"),
				{ { CharacterStruct::BODY, 20 }, { CharacterStruct::SKIRT, 6 }, },
				-1, 1
			},
			{ TEXT("Neck Thickness"),
				{ { CharacterStruct::BODY, 2 },{ CharacterStruct::BODY, 3 },
				{ CharacterStruct::SKIRT, 24 },{ CharacterStruct::SKIRT, 25 } },
				-0.5f, 0.5f
			},
			{ TEXT("Neck Height"),
				{ { CharacterStruct::SKELETON, 66 },{ CharacterStruct::SKELETON, 67 }, },
				-1, 1
			},
			{ TEXT("Shoulders Width"),
				{ { CharacterStruct::SKELETON, 36 },{ CharacterStruct::SKELETON, 37 } },
				-0.5f, 0.5f
			},
			{ TEXT("Shoulders Size"),
				{ { CharacterStruct::SKELETON, 21 },{ CharacterStruct::SKELETON, 22 } },
				-0.5f, 0.5f
			},
			{ TEXT("Shoulders Height"),
				{ { CharacterStruct::SKELETON, 23 },{ CharacterStruct::SKELETON, 24 } },
				-0.5f, 0.5f
			},
			{ TEXT("Arms Size"),
				{ { CharacterStruct::SKELETON, 40 },
				{ CharacterStruct::SKELETON, 41 }, },
				-0.4f, 0.3f
			},
			{ TEXT("Arms Distance"),
				{ { CharacterStruct::SKELETON, 38 },
				{ CharacterStruct::SKELETON, 39 }, },
				-0.4f, 0.3f
			},
			{ TEXT("Arms Thickness"),
				{ { CharacterStruct::BODY, 4 },{ CharacterStruct::BODY, 5 },{ CharacterStruct::BODY, 6 },
				{ CharacterStruct::BODY,  7 },{ CharacterStruct::BODY,  8 },{ CharacterStruct::BODY, 9 },
				{ CharacterStruct::BODY, 10 },{ CharacterStruct::BODY, 11 },{ CharacterStruct::BODY, 12 },
				{ CharacterStruct::BODY, 13 },
				{ CharacterStruct::SKIRT, 14 },{ CharacterStruct::SKIRT, 15 },{ CharacterStruct::SKIRT, 16 },
				{ CharacterStruct::SKIRT, 17 },{ CharacterStruct::SKIRT, 18 },{ CharacterStruct::SKIRT, 19 },
				{ CharacterStruct::SKIRT, 20 },{ CharacterStruct::SKIRT, 21 },{ CharacterStruct::SKIRT, 32 },
				{ CharacterStruct::SKIRT, 33 } },
				-0.5f, 0.5f
			},
			{ TEXT("Upper Arm Thickness"), //Split from Arm Thickness.
				{ { CharacterStruct::BODY, 41 },{ CharacterStruct::BODY, 42 },{ CharacterStruct::BODY, 43 },
				{ CharacterStruct::BODY,  44 },{ CharacterStruct::BODY,  45 },{ CharacterStruct::BODY, 46 },
				{ CharacterStruct::SKIRT, 41 },{ CharacterStruct::SKIRT, 42 },{ CharacterStruct::SKIRT, 43 },
				{ CharacterStruct::SKIRT, 44 },{ CharacterStruct::SKIRT, 45 },{ CharacterStruct::SKIRT, 46 }, },
				-0.5f, 0.5f
			},
			{ TEXT("Under Arm Thickness"), //Split from Arm Thickness.
				{ { CharacterStruct::BODY, 47 },{ CharacterStruct::BODY, 48 },{ CharacterStruct::BODY, 49 },
				{ CharacterStruct::BODY,  50 },
				{ CharacterStruct::SKIRT, 47 },{ CharacterStruct::SKIRT, 48 },{ CharacterStruct::SKIRT, 49 },
				{ CharacterStruct::SKIRT, 50 }, },
				-0.5f, 0.5f
			},
			{ TEXT("Breasts Width"),
				{ { CharacterStruct::BODY, 30 },{ CharacterStruct::BODY, 31 },
				{ CharacterStruct::BODY, 32 },{ CharacterStruct::BODY, 33 },
				{ CharacterStruct::SKIRT, 28 },{ CharacterStruct::SKIRT, 29 },
				{ CharacterStruct::SKIRT, 30 },{ CharacterStruct::SKIRT, 31 }, },
				-1, 1
			},
			{ TEXT("Breasts Fullness"),
				{ { CharacterStruct::SKELETON, 56 },{ CharacterStruct::SKELETON, 57 },
				{ CharacterStruct::BODY, 34 },{ CharacterStruct::BODY, 35 },
				{ CharacterStruct::SKIRT, 34 },{ CharacterStruct::SKIRT, 35 },
				{ CharacterStruct::SKIRT, 36 },{ CharacterStruct::SKIRT, 37 }, },
				0, 1
			},
			{ TEXT("Breasts Size"),
			{ { CharacterStruct::SKELETON, 75 },{ CharacterStruct::SKELETON, 76 },
			{ CharacterStruct::SKELETON, 77 },{ CharacterStruct::SKELETON, 78 }, },
			0, 2
			},
			{ TEXT("Breasts Pushup"),
				{ { CharacterStruct::SKELETON, 64 },{ CharacterStruct::SKELETON, 65 }, },
				-1, 1
			},
			{ TEXT("Breasts Distance"),
			{ { CharacterStruct::SKELETON, 81 },{ CharacterStruct::SKELETON, 82 }, },
			-1, 1
			},
			{ TEXT("Breast Perkiness"),
			{ { CharacterStruct::SKELETON, 79 },{ CharacterStruct::SKELETON, 80 }, },
			-1, 1
			},
			{ TEXT("Nipple Size"),
				{ { CharacterStruct::BODY, 37 },{ CharacterStruct::BODY, 38 },{ CharacterStruct::SKIRT, 64 },{ CharacterStruct::SKIRT, 65 }, },
				-0.2, 1
			},
			{ TEXT("Waist Thickness"),
				{ { CharacterStruct::BODY, 26 },{ CharacterStruct::BODY, 27 },
				{ CharacterStruct::SKIRT, 12 },{ CharacterStruct::SKIRT, 13 },
				{ CharacterStruct::LEGS, 11 },{ CharacterStruct::LEGS, 12 } },
				-1, 1
			},
			{ TEXT("Waist Height"),
				{ { CharacterStruct::BODY, 22 },{ CharacterStruct::SKIRT, 8 },
				{ CharacterStruct::LEGS, 13 } },
				-1, 1
			},
			{ TEXT("Waist Position"),
				{ { CharacterStruct::BODY, 23 },{ CharacterStruct::SKIRT, 9 },
				{ CharacterStruct::LEGS, 14 } },
				-1, 1
			},
			{ TEXT("Hip Size"),
				{ { CharacterStruct::BODY,   15 }, { CharacterStruct::SKIRT, 0 },
				{ CharacterStruct::LEGS, 10 }, 
				{ CharacterStruct::SKELETON, 45 },
				{ CharacterStruct::SKELETON, 46 }, { CharacterStruct::SKELETON, 47 } },
				-0.6f, 0.6f
			},
			{ TEXT("Hip Height"),
				{	{ CharacterStruct::BODY, 21 },
				//	{ CharacterStruct::SKIRT, 7 },
					{ CharacterStruct::LEGS, 16 }
				},
				-1, 1
			},
			{ TEXT("Hip Width"),
				{ { CharacterStruct::BODY, 0 },{ CharacterStruct::SKIRT, 26 },
				{ CharacterStruct::LEGS, 8 }
				},
				-0.5f, 0.5f
			},
			{ TEXT("Hip Thickness"),
				{ { CharacterStruct::BODY, 1 },{ CharacterStruct::SKIRT, 27 },
				{ CharacterStruct::LEGS, 9 }
				},
				-0.5f, 0.5f
			},
			{ TEXT("Hip Depth Correction"),
				{ { CharacterStruct::BODY, 36 },{ CharacterStruct::SKIRT, 38 },
				{ CharacterStruct::LEGS, 15 }
				},
				-0.5f, 0.5f
			},
			{ TEXT("Butt Size"), //Split from Hip Size, same thing - Vertical scaling.
				{ { CharacterStruct::BODY, 51 },{ CharacterStruct::SKIRT, 51 },
				{ CharacterStruct::LEGS, 17 },
				{ CharacterStruct::SKELETON, 73 },{ CharacterStruct::SKELETON, 74 } },
				-0.5f, 1.0f
			},
			{ TEXT("Ass Cheeks Width"),
				{ { CharacterStruct::SKELETON, 9 },{ CharacterStruct::SKELETON, 10 } },
				-0.5f, 0.5f
			},
			{ TEXT("Ass Cheeks Thickness"),
				{ { CharacterStruct::SKELETON, 11 },{ CharacterStruct::SKELETON, 12 } },
				-0.5f, 0.5f
			},
			{ TEXT("Legs Distance"),
				{ { CharacterStruct::SKELETON, 48 },{ CharacterStruct::SKELETON, 49 },
				{ CharacterStruct::SKELETON, 50 } ,{ CharacterStruct::SKELETON, 51 },
				{ CharacterStruct::SKELETON, 52 } ,{ CharacterStruct::SKELETON, 53 }, },
				-0.5f, 0.5f
			},
			{ TEXT("Leg Depth Correction"),
				{ { CharacterStruct::SKELETON, 58 },{ CharacterStruct::SKELETON, 59 },
				{ CharacterStruct::SKELETON, 60 } ,{ CharacterStruct::SKELETON, 61 },
				{ CharacterStruct::SKELETON, 62 } ,{ CharacterStruct::SKELETON, 63 }, },
				-0.5f, 0.5f
			},
			{ TEXT("Upper Thigh Size"),
				{ { CharacterStruct::BODY, 16 },{ CharacterStruct::BODY, 18 },
				{ CharacterStruct::LEGS, 2 },{ CharacterStruct::LEGS, 3 },
				{ CharacterStruct::SKIRT, 2 },{ CharacterStruct::SKIRT, 4 } },
				-1.0f, 1.0f
			},
			{ TEXT("Upper Thigh Width"),
				{ { CharacterStruct::BODY, 52 },{ CharacterStruct::BODY, 53 },
				{ CharacterStruct::LEGS, 18 },{ CharacterStruct::LEGS, 19 },
				{ CharacterStruct::SKIRT, 52 },{ CharacterStruct::SKIRT, 53 } },
				-1.0f, 1.0f
			},
			{ TEXT("Upper Thigh Thick"),
				{ { CharacterStruct::BODY, 54 },{ CharacterStruct::BODY, 55 },
				{ CharacterStruct::LEGS, 20 },{ CharacterStruct::LEGS, 21 },
				{ CharacterStruct::SKIRT, 54 },{ CharacterStruct::SKIRT, 55 } },
					-1.0f, 1.0f
			},
			{ TEXT("Lower Thigh Size"),
				{ { CharacterStruct::BODY, 17 },{ CharacterStruct::BODY, 19 },
				{ CharacterStruct::LEGS, 0 },{ CharacterStruct::LEGS, 1 },
				{ CharacterStruct::SKIRT, 3 },{ CharacterStruct::SKIRT, 5 } },
				-1.0f, 1.0f
			},
			{ TEXT("Lower Thigh Width"),
				{ { CharacterStruct::BODY, 56 },{ CharacterStruct::BODY, 57 },
				{ CharacterStruct::LEGS, 22 },{ CharacterStruct::LEGS, 23 },
				{ CharacterStruct::SKIRT, 56 },{ CharacterStruct::SKIRT, 57 } },
				-1.0f, 1.0f
			},
			{ TEXT("Lower Thigh Thick"),
				{ { CharacterStruct::BODY, 58 },{ CharacterStruct::BODY, 59 },
				{ CharacterStruct::LEGS, 24 },{ CharacterStruct::LEGS, 25 },
				{ CharacterStruct::SKIRT, 58 },{ CharacterStruct::SKIRT, 59 } },
				-1.0f, 1.0f
			},
			{ TEXT("Zetthigh Ryouiki"),
				{ { CharacterStruct::LEGS, 6 },{ CharacterStruct::LEGS, 7 },
				{ CharacterStruct::BODY, 24 },{ CharacterStruct::BODY, 25 },
				/*{ CharacterStruct::SKIRT, 10 },{ CharacterStruct::SKIRT, 11 }*/ },
				-3.0f, 3.0f
			},
			{ TEXT("Calves Size"),
				{ { CharacterStruct::LEGS, 4 },{ CharacterStruct::LEGS, 5 },
				{ CharacterStruct::SKIRT, 22 },{ CharacterStruct::SKIRT, 23 } },
				-0.5f, 0.5f
			},
			{ TEXT("Calves Width"),
				{ { CharacterStruct::LEGS, 26 },{ CharacterStruct::LEGS, 27 },
				{ CharacterStruct::SKIRT, 60 },{ CharacterStruct::SKIRT, 61 } },
				-0.5f, 0.5f
			},
			{ TEXT("Calves Thickness"),
				{ { CharacterStruct::LEGS, 28 },{ CharacterStruct::LEGS, 29 },
				{ CharacterStruct::SKIRT, 62 },{ CharacterStruct::SKIRT, 63 } },
				-0.5f, 0.5f
			},
			{ TEXT("Head Size"),
				{ { CharacterStruct::SKELETON, 54 } },
				0, 3
			},
			{ TEXT("Hair Size"),
				{ { CharacterStruct::HAIR_FRONT, 0 },{ CharacterStruct::HAIR_FRONT, 1 },{ CharacterStruct::HAIR_FRONT, 6 },
				{ CharacterStruct::HAIR_SIDE, 0 },{ CharacterStruct::HAIR_SIDE, 1 },
				{ CharacterStruct::HAIR_BACK, 0 },{ CharacterStruct::HAIR_BACK, 1 },{ CharacterStruct::HAIR_BACK, 2 },
				{ CharacterStruct::HAIR_EXT,  0 },{ CharacterStruct::HAIR_EXT,  1 } },
				-1, 1
			},
			{ TEXT("Hair Width"),
				{ { CharacterStruct::HAIR_FRONT, 4 },{ CharacterStruct::HAIR_FRONT, 5 },{ CharacterStruct::HAIR_FRONT, 8 },
				{ CharacterStruct::HAIR_SIDE, 4 },{ CharacterStruct::HAIR_SIDE, 5 },
				{ CharacterStruct::HAIR_BACK, 6 },{ CharacterStruct::HAIR_BACK, 7 },{ CharacterStruct::HAIR_BACK, 8 },
				{ CharacterStruct::HAIR_EXT,  4 },{ CharacterStruct::HAIR_EXT,  5 } },
				-1, 1
			},
			{ TEXT("Hair Height"),
				{ { CharacterStruct::HAIR_FRONT, 2 },{ CharacterStruct::HAIR_FRONT, 3 },{ CharacterStruct::HAIR_FRONT, 7 },
				{ CharacterStruct::HAIR_SIDE, 2 },{ CharacterStruct::HAIR_SIDE, 3 },
				{ CharacterStruct::HAIR_BACK, 3 },{ CharacterStruct::HAIR_BACK, 4 },{ CharacterStruct::HAIR_BACK, 5 },
				{ CharacterStruct::HAIR_EXT,  2 },{ CharacterStruct::HAIR_EXT,  3 } },
				-2, 2
			},
			{ TEXT("Eyebrow Height"),
				{ { CharacterStruct::FACE, 3 },{ CharacterStruct::FACE, 4 } },
				-0.1f, 0.1f
			},
			{ TEXT("Eye Depth"),
				{ { CharacterStruct::FACE, 5 } },
				-0.1f, 0.1f
			},
			{ TEXT("Ear Height"),
				{ { CharacterStruct::FACE, 1 } },
				-0.5f, 0.5f
			},
			{ TEXT("Ear (Split) Spacing"),
				{ { CharacterStruct::FACE, 15 },{ CharacterStruct::FACE, 16 } },
				-0.1f, 0.1f
			},
			{ TEXT("Ear (Split) Depth"),
				{ { CharacterStruct::FACE, 17 },{ CharacterStruct::FACE, 18 } },
				-0.1f, 0.1f
			},
			{ TEXT("Ear (Split) Scale X"),
				{ { CharacterStruct::FACE, 6 },{ CharacterStruct::FACE, 7 } },
				-0.3f, 0.3f
			},
			{ TEXT("Ear (Split) Scale Y"),
				{ { CharacterStruct::FACE, 8 },{ CharacterStruct::FACE, 9 } },
				-0.3f, 0.3f
			},
			{ TEXT("Ear (Split) Scale Z"),
			{ { CharacterStruct::FACE, 13 },{ CharacterStruct::FACE, 14 } },
				-0.3f, 0.3f
			},
			{ TEXT("Mouth Width"),
				{ { CharacterStruct::FACE, 0 } },
				-0.9f, 0.5f
			},
			{ TEXT("Mouth Height"),
				{ { CharacterStruct::FACE, 2 } },
				-0.05f, 0.05f
			},
			{ TEXT("Glasses vertical"),
				{ { CharacterStruct::FACE, 10 } },
				-1.0f, 1.0f
			},
			{ TEXT("Glasses horizontal"),
				{ { CharacterStruct::FACE, 11 } },
				-1.0f, 1.0f
			},
			{ TEXT("Glasses rotation"),
				{ { CharacterStruct::FACE, 12 } },
				-1.0f, 1.0f
			},
			{ TEXT("Ball Size"),
				{ { CharacterStruct::SKELETON, 29 } },
				0.5f, 1.5f
			},
			{ TEXT("Strapon/Dick Length"),
				{ { CharacterStruct::SKELETON, 30 },{ CharacterStruct::SKELETON, 31 } },
				-0.5f, 0.5f
			},
			{ TEXT("Strapon/Dick Girth"),
				{ { CharacterStruct::SKELETON, 32 },{ CharacterStruct::SKELETON, 33 } },
				0.5f, 1.5f
			},
			{ TEXT("Glans Size"),
				{ { CharacterStruct::SKELETON, 34 },{ CharacterStruct::SKELETON, 35 } },
				0.5f, 1.5f
			},


			//Tot Sliders
			{ TEXT("Nose Tilt"),
			{ { CharacterStruct::FACE, 19 } },
					-0.05f, 0.05f
			},
			{ TEXT("Nose Width"),
			{ { CharacterStruct::FACE, 20 } },
					-0.2f, 0.2f
			},
			{ TEXT("Nose Height"),
			{ { CharacterStruct::FACE, 21 } },
					-0.05f, 0.05f
			},
			{ TEXT("Nose Length"),
			{ { CharacterStruct::FACE, 22 } },
					-0.05f, 0.05f
			},
			{ TEXT("Nose Bridge Depth"),
			{ { CharacterStruct::FACE, 23 } },
					-0.05f, 0.05f
			},
			{ TEXT("Chin Height"),
			{ { CharacterStruct::FACE, 24 } },
					-0.20f, 0.05f
			},
			{ TEXT("Chin Depth"),
			{ { CharacterStruct::FACE, 25 } },
					-0.1f, 0.1f
			},
			{ TEXT("Chin Width"),
			{ { CharacterStruct::FACE, 49 } },
				-1, 0
			},
			{ TEXT("Jaw Width"),
			{ { CharacterStruct::FACE, 26 } },
					-0.1f, 0.1f
			},
			{ TEXT("Jaw Shape 1"),
			{ { CharacterStruct::FACE, 27 } },
					-0.05f, 0.3f
			},
			{ TEXT("Jaw Shape 2"),
			{ { CharacterStruct::FACE, 28 } },
					-0.1f, 0
			},
			{ TEXT("Cheek Height"),
			{ { CharacterStruct::FACE, 29 } },
					-0.1f, 0.1f
			},
			{ TEXT("Cheek Depth"),
			{ { CharacterStruct::FACE, 30 } },
					-0.1f, 0.1f
			},
			{ TEXT("Cheek Width"),
			{ { CharacterStruct::FACE, 31 } },
					-0.1f, 0.1f
			},
			{ TEXT("Face Bottom Position?"),
			{ { CharacterStruct::FACE, 32 } },
					-0.3f, 0
			},

			//Tot Teeth Sliders.
			{ TEXT("Upper Shark Teeth"),
			{ { CharacterStruct::FACE, 33 },{ CharacterStruct::FACE, 34 } },
					-0.03f, 0.03f
			},
			{ TEXT("Lower Shark Teeth"),
			{ { CharacterStruct::FACE, 35 },{ CharacterStruct::FACE, 36 } },
					-0.03f, 0.03f
			},
			//{ TEXT("Upper Left Incisor"),
			//	{ { CharacterStruct::FACE, 37 } },
			//	-0.05f, 0.05f
			//},
			//{ TEXT("Upper Right Incisor"),
			//	{ { CharacterStruct::FACE, 38 } },
			//	-0.05f, 0.05f
			//},
			//{ TEXT("Lower Left Incisor"),
			//	{ { CharacterStruct::FACE, 39 } },
			//	-0.05f, 0.05f
			//},
			//{ TEXT("Lower Right Incisor"),
			//	{ { CharacterStruct::FACE, 40 } },
			//	-0.05f, 0.05f
			//},
			{ TEXT("Upper Left Canine"),
			{ { CharacterStruct::FACE, 41 } },
					-0.05f, 0.05f
			},
			{ TEXT("Upper Right Canine"),
			{ { CharacterStruct::FACE, 42 } },
					-0.05f, 0.05f
			},
			{ TEXT("Lower Left Canine"),
			{ { CharacterStruct::FACE, 43 } },
					-0.05f, 0.05f
			},
			{ TEXT("Lower Right Canine"),
			{ { CharacterStruct::FACE, 44 } },
					-0.05f, 0.05f
			},
			//{ TEXT("Upper Left Premolar"),
			//		{ { CharacterStruct::FACE, 45 } },
			//	-0.05f, 0.05f
			//},
			//{ TEXT("Upper Right Premolar"),
			//		{ { CharacterStruct::FACE, 46 } },
			//	-0.05f, 0.05f
			//},
			//{ TEXT("Lower Left Premolar"),
			//		{ { CharacterStruct::FACE, 47 } },
			//	-0.05f, 0.05f
			//},
			//{ TEXT("Lower Right Premolar"),
			//		{ { CharacterStruct::FACE, 48 } },
			//	-0.03f, 0.03f
			//},


			
			
			//Obsolete Sliders
			{ TEXT("*Body Thickness"),
				{ { CharacterStruct::SKELETON, 2 } },
				-0.9f, 1.0f
			},
			{ TEXT("*Total Body Height"),
				{ { CharacterStruct::SKELETON, 0 } },
				-0.9f, 1.0f
			},
			{ TEXT("*Total Body Width"),
				{ { CharacterStruct::SKELETON, 1 } },
				-0.9f, 1.0f
			},
			{ TEXT("*Bottom Width"),
				{ { CharacterStruct::SKELETON, 7 } },
				0.5f, 1.5f
			},
			{ TEXT("*Bottom Thickness"),
				{ { CharacterStruct::SKELETON, 8 } },
				0.5f, 1.5f
			},
			{ TEXT("*Thighs Width"),
				{ { CharacterStruct::SKELETON, 19 },{ CharacterStruct::SKELETON, 20 } },
				-0.5f, 0.5f
			},
			{ TEXT("*Hand Length"),
				{ { CharacterStruct::SKELETON, 5 },{ CharacterStruct::SKELETON, 6 },
				{ CharacterStruct::SKELETON, 13 },{ CharacterStruct::SKELETON, 14 } },
				-0.4f, 0.2f
			},
			{ TEXT("*Foot Length"),
				{ { CharacterStruct::SKELETON, 15 },{ CharacterStruct::SKELETON, 16 } },
				-0.15f, 0.2f
			},
			{ TEXT("*Foot Width"),
				{ { CharacterStruct::SKELETON, 17 },{ CharacterStruct::SKELETON, 18 } },
				-0.2f, 0.2f
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
				if (thisPtr->m_sliders[i].isModified) break;
				ignoreNextSlider = true;
				thisPtr->m_sliders[i].Sync(false);
				thisPtr->ApplySlider(i);
				break;
			}
		}
		break; }
	case WM_MOUSEWHEEL: {
		BSDialog* thisPtr = (BSDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		if (thisPtr == NULL) return FALSE;

		WPARAM type = GET_WHEEL_DELTA_WPARAM(wparam) < 0 ? SB_PAGEDOWN : SB_PAGEUP;
		General::ScrollWindow(hwndDlg, type);
		return TRUE;
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
					if (thisPtr->m_sliders[i].isModified) break;
					ignoreNextSlider = true;
					thisPtr->m_sliders[i].Sync(true);
					thisPtr->ApplySlider(i);
					break;
				}
			}
			return TRUE; }
		case BN_CLICKED: {
			if (ignoreNextSlider) {
				ignoreNextSlider = false;
				return TRUE;
			}
			HWND ed = (HWND)lparam;
			for (size_t i = 0; i < thisPtr->m_sliders.size(); i++) {
				if (ed == thisPtr->m_sliders[i].btnReset) {
					if (thisPtr->m_sliders[i].isModified) break;
					ignoreNextSlider = true;
					thisPtr->m_sliders[i].Reset();
					thisPtr->ApplySlider(i);
					break;
				}
			}
			return TRUE;
		}
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
					ExtClass::XXFile* loadedXXFile = ExtVars::AAEdit::GetCurrentCharacter()->GetXXFile(slider->target);
					if (elem.xxFile != loadedXXFile)
						continue;
					ExtClass::Frame* frame = elem.frame;
					if (!frame->m_name)
						continue;
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
						for(auto& frameParent : elem.parents) {
							ExtClass::XXFile* loadedXXFile = ExtVars::AAEdit::GetCurrentCharacter()->GetXXFile(slider->target);
							if (frameParent.first != loadedXXFile)
								continue;
							for(int i = 0; i < frameParent.second->m_nBones; i++) {
								ExtClass::Bone* bone = &frameParent.second->m_bones[i];
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
			ExtClass::XXFile* loadedXXFile = ExtVars::AAEdit::GetCurrentCharacter()->GetXXFile(model);
			if (elem.xxFile != loadedXXFile)
				continue;
			ExtClass::Frame* frame = elem.frame;
			if (!frame->m_name)
				continue;
			TCHAR buff[256];
			size_t written;
			mbstowcs_s(&written,buff,frame->m_name+5,256);
			std::wstring str(buff);
			auto* rule = Shared::g_currentChar->m_cardData.GetSliderFrameRule(model,str);
			if(rule != NULL) {
				D3DMATRIX& mat = elem.matrix;
				D3DXVECTOR3 scale = { mat._11, mat._12, mat._13 };
				D3DXVECTOR3 rot = { mat._21, mat._22, mat._23 };
				D3DXVECTOR3 trans = { mat._31, mat._32, mat._33 };
				for(auto& elem : *rule) {
					Shared::Slider::ModifySRT(&scale,&rot,&trans,elem.first->op,elem.second);
				}
				auto res = General::MatrixFromSRT(scale,rot,trans);
				frame->m_matrix1 = res;
				frame->m_matrix5 = res;
			}
		}
		for (auto& elem : Shared::g_xxBoneParents[model]) {
			ExtClass::XXFile* loadedXXFile = ExtVars::AAEdit::GetCurrentCharacter()->GetXXFile(model);
			auto* rule = Shared::g_currentChar->m_cardData.GetSliderBoneRule(model,elem.boneName);
			if(rule != NULL) {
				std::string strBoneName(elem.boneName.begin(), elem.boneName.end());
				ExtClass::Frame* frame;
				for(auto& parent : elem.parents) {
					if (parent.first != loadedXXFile)
						continue;
					frame = parent.second;
					for(int i = 0; i < frame->m_nBones; i++) {
						ExtClass::Bone* bone = &frame->m_bones[i];
						if(bone->m_name == strBoneName) {
							D3DMATRIX& mat = elem.srtMatrix;
							D3DMATRIX& origMat = elem.origMatrix;
							D3DXVECTOR3 scale = { mat._11, mat._12, mat._13 };
							D3DXVECTOR3 rot = { mat._21, mat._22, mat._23 };
							D3DXVECTOR3 trans = { mat._31, mat._32, mat._33 };
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

void UnlimitedDialog::BSDialog::BodySlider::Reset() {
	currVal = sliderData[0]->GetNeutralValue();
	isModified = true;
	int ret = SendMessage(slider, TBM_SETPOS, TRUE, Val2Sld(currVal));
	TCHAR number[52];
	swprintf_s(number, TEXT("%f"), currVal);
	SendMessage(edit, WM_SETTEXT, 0, (LPARAM)number);
	isModified = false;
	ret++;
}


UnlimitedDialog::BSDialog::BodySlider::BodySlider()  {

}

UnlimitedDialog::BSDialog::BodySlider::BodySlider(HWND dialog, const TCHAR* label, int xStart, int yStart,
												std::vector<const Shared::Slider*> sliderData, float min, float max)
	: sliderData(sliderData) {	
	HWND templateStatic = GetDlgItem(dialog,IDC_BS_STEXAMPLE);
	HWND templateSlider = GetDlgItem(dialog,IDC_BS_SLDEXAMPLE);
	HWND templateEdit = GetDlgItem(dialog,IDC_BS_EDEXAMPLE);
	HWND templateReset = GetDlgItem(dialog, IDC_BS_BTNRST);

	this->stLabel = CreateWindowEx(0,TEXT("STATIC"),label,WS_CHILD | WS_VISIBLE,
		0,0,0,0,
		dialog,0,General::DllInst,0);
	this->slider = CreateWindowEx(0,TEXT("msctls_trackbar32"),label,WS_CHILD | WS_VISIBLE | TBS_BOTH | TBS_NOTICKS | WS_TABSTOP,
		0,0,0,0,
		dialog,0,General::DllInst,0);
	this->edit = CreateWindowEx(WS_EX_CLIENTEDGE,TEXT("EDIT"),label,WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
		0,0,0,0,
		dialog,0,General::DllInst,0);
	this->btnReset = CreateWindowEx(0, TEXT("BUTTON"), L"Reset", WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		dialog, 0, General::DllInst, 0);

	using namespace General;

	auto CopyStyleFromWindow = [](HWND to, HWND from){
		LONG exStyles = GetWindowLong(from,GWL_EXSTYLE);
		LONG styles = GetWindowLong(from,GWL_STYLE) | WS_VISIBLE;
		SetWindowLong(to,GWL_EXSTYLE,exStyles);
		SetWindowLong(to,GWL_STYLE,styles);
		HFONT font = (HFONT)SendMessage(from,WM_GETFONT,0,0);
		SendMessage(to,WM_SETFONT,(WPARAM)font,FALSE);
	};

	//adjust style from template
	CopyStyleFromWindow(stLabel,templateStatic);
	CopyStyleFromWindow(slider,templateSlider);
	CopyStyleFromWindow(edit,templateEdit);
	CopyStyleFromWindow(btnReset,templateReset);

	//move window according to template
	RECT rctTmplStatic,rctTmplSlider,rctTmplEdit,rctTmplReset;
	GetWindowRect(templateStatic,&rctTmplStatic);
	GetWindowRect(templateSlider,&rctTmplSlider);
	GetWindowRect(templateEdit,&rctTmplEdit);
	GetWindowRect(templateReset,&rctTmplReset);
	//get top left corner
	LONG left = rctTmplStatic.left,top = min(rctTmplStatic.top,min(rctTmplSlider.top,min(rctTmplEdit.top, rctTmplReset.top)));
	//adjust top left corner to (xStart|yStart)
	RectMoveBy(rctTmplStatic,-left+xStart,-top+yStart);
	RectMoveBy(rctTmplSlider,-left+xStart,-top+yStart);
	RectMoveBy(rctTmplEdit,-left+xStart,-top+yStart);
	RectMoveBy(rctTmplReset, -left + xStart, -top + yStart);
	//move actual window
	MoveWindowRect(stLabel,rctTmplStatic,FALSE);
	MoveWindowRect(slider,rctTmplSlider,FALSE);
	MoveWindowRect(edit,rctTmplEdit,FALSE);
	MoveWindowRect(btnReset,rctTmplReset,FALSE);

	isModified = false;

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
			if (elem.first.second >= Shared::g_sliders[elem.first.first].size())
				continue;
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
		case IDC_MD_LBINUSE:
			if (notification == LBN_SELCHANGE) {
				int sel = SendMessage(wnd, LB_GETCURSEL, 0, 0);
				if (sel != LB_ERR) {
					SendMessage(thisPtr->m_edName, WM_SETTEXT, 0, (LPARAM)g_currChar.m_cardData.GetModules()[sel].name.c_str());
					SendMessage(thisPtr->m_edDescr, WM_SETTEXT, 0, (LPARAM)g_currChar.m_cardData.GetModules()[sel].description.c_str());
					return TRUE;
				}
			}
			break;
		case IDC_MD_LBAVAILABLE:
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
		case IDC_MD_BTNUPD:
			if (notification == BN_CLICKED) {
				//save the list of modules in use
				std::vector<Shared::Triggers::Module> noFileModules;
				std::vector<int> modulesList;
				modulesList.reserve(g_currChar.m_cardData.GetModules().size());
				for (int i = 0; i < g_currChar.m_cardData.GetModules().size(); i++) {
					bool hasFile = false;
					for (int j = 0; j < thisPtr->m_modules.size(); j++) {
						if (g_currChar.m_cardData.GetModules()[i].name == thisPtr->m_modules[j].mod.name) {
							modulesList.push_back(j);
							hasFile = true;
							break;
						}
					}
					if (!hasFile) {
						noFileModules.push_back(g_currChar.m_cardData.GetModules()[i]);
					}
				}
				//clear modules in use
				g_currChar.m_cardData.GetModules().clear();
				//readd modules with no file on disk
				for (int i = 0; i < noFileModules.size(); i++) {
					g_currChar.m_cardData.AddModule(noFileModules[i]);
				}
				//readd updated modules
				for (int i = 0; i < modulesList.size(); i++) {
					g_currChar.m_cardData.AddModule(thisPtr->m_modules[modulesList[i]].mod);
				}
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
