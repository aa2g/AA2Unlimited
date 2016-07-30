#include "PickExe.h"

#include <Windows.h>
#include <CommCtrl.h>

#include "resource.h"

int ChooseIndex(const std::vector< std::pair<std::string, DWORD>>& params) {
	INT_PTR ret = DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_EXEPICKER), NULL, DialogProc, (LPARAM)&params);
	return ret;
}

INT_PTR CALLBACK DialogProc( _In_ HWND   hwndDlg, _In_ UINT   umsg, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	switch (umsg) {
	case WM_INITDIALOG: {
		HWND listWnd = GetDlgItem(hwndDlg, IDC_EXELIST);
		const std::vector< std::pair<std::string, DWORD>>* list = (const std::vector< std::pair<std::string, DWORD>>*)(lparam);
		for (int i = 0; i < list->size(); i++) {
			SendMessage(listWnd, LB_ADDSTRING, i, (LPARAM)list->at(i).first.c_str());
		}
		return TRUE; }
	case WM_COMMAND:
		switch (HIWORD(wparam)) {
		case BN_CLICKED: 
			if (LOWORD(wparam) == IDOK) {
				int sel = SendMessage(GetDlgItem(hwndDlg, IDC_EXELIST), LB_GETCURSEL, 0, 0);
				EndDialog(hwndDlg, sel);
				return TRUE;
			}
			else if (LOWORD(wparam) == IDCANCEL) {
				EndDialog(hwndDlg, -1);
				return TRUE;
			}
			break;
		}
		break;
	}
	return FALSE;
}