#pragma once

#include <vector>
#include <string>
#include <Windows.h>

//i wish i did mfc


int ChooseIndex(const std::vector< std::pair<std::string, DWORD>>& params);

INT_PTR CALLBACK DialogProc(
	_In_ HWND   hwndDlg,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);


