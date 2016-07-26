#pragma once

#include <Windows.h>
#include <vector>

namespace AAEdit {

/*
 * The little dialog that shows up in the editor to select stuff.
 */
class UnlimitedDialog
{
public:
	UnlimitedDialog();
	~UnlimitedDialog();

	void Initialize();
	void Hide();
	void Destroy();

	void RefreshRuleList();
	void RefreshTextureList();

	inline bool IsVisible() const {
		return m_visible;
	}
private:
	HWND m_dialog;
	HWND m_cbOverride;
	HWND m_edOverrideWith;
	HWND m_lbOverrides;

	bool m_visible;
private:

	static const char ListSeperator[5];
	static INT_PTR CALLBACK DialogProc(_In_ HWND hwndDlg, _In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam);
};

extern UnlimitedDialog g_AAUnlimitDialog;
/*
 * Fed every time the original notification function of the system dialog is called
 */
LRESULT __stdcall SystemDialogNotification(void* internclass, HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);

}