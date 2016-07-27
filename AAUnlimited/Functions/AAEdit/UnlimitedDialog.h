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

	void Refresh();

	inline bool IsVisible() const {
		return m_visible;
	}
private:
	struct Dialog {
		virtual void Show(bool state) = 0;
		virtual void Refresh() = 0;
	};
	struct MODialog : public Dialog {
		HWND m_dialog;
		HWND m_cbOverride;
		HWND m_edOverrideWith;
		HWND m_lbOverrides;
		inline void Show(bool state) {
			ShowWindow(m_dialog, state ? SW_SHOW : SW_HIDE);
		}
		void RefreshRuleList();
		void RefreshTextureList();
		void Refresh();
		static INT_PTR CALLBACK DialogProc(_In_ HWND hwndDlg, _In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam);
	} m_moDialog;
	struct AODialog : public Dialog {
		HWND m_dialog;
		HWND m_edArchive;
		HWND m_edArchiveFile;
		HWND m_edOverrideFile;
		HWND m_btBrowse;
		HWND m_btApply;
		HWND m_lbOverrides;
		inline void Show(bool state) {
			ShowWindow(m_dialog, state ? SW_SHOW : SW_HIDE);
		}
		void RefreshRuleList();
		void Refresh();
		static INT_PTR CALLBACK DialogProc(_In_ HWND hwndDlg, _In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam);
	} m_aoDialog;


	HWND m_dialog;
	HWND m_tabs;

	bool m_visible;

	LPARAM GetCurrTabItemData();
private:

	static INT_PTR CALLBACK MainDialogProc(_In_ HWND hwndDlg, _In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam);
};

extern UnlimitedDialog g_AAUnlimitDialog;
/*
 * Fed every time the original notification function of the system dialog is called
 */
LRESULT __stdcall SystemDialogNotification(void* internclass, HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);

}