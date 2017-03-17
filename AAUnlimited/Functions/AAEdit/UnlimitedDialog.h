#pragma once

#include <Windows.h>
#include <CommCtrl.h>
#include <vector>

#include "Files\ModuleFile.h"
#include "Functions\AAUCardData.h"
#include "External\ExternalClasses\XXFile.h"
#include "Functions\Shared\Triggers\Triggers.h"
#include "Functions\Shared\Slider.h"

namespace AAEdit {

	struct loc_AddData;
	struct loc_AddActionData;
	struct loc_AddVariableData;
	struct loc_ModuleInfo;

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

		inline bool IsSaveFilesSet() const {
			return SendMessage(m_gnDialog.m_cbSaveFiles,BM_GETCHECK,0,0) == BST_CHECKED;
		}
		inline void SetSaveFiles(BOOL state) {
			SendMessage(m_gnDialog.m_cbSaveFiles,BM_SETCHECK,state ? BST_CHECKED : BST_UNCHECKED,0);
		}
		inline bool IsSaveEyesSet() const {
			return SendMessage(m_gnDialog.m_cbSaveEyeTexture,BM_GETCHECK,0,0) == BST_CHECKED;
		}
		inline void SetSaveEyes(BOOL state) {
			SendMessage(m_gnDialog.m_cbSaveEyeTexture,BM_SETCHECK,state ? BST_CHECKED : BST_UNCHECKED,0);
		}
		inline bool IsSaveEyeHighlightSet() const {
			return SendMessage(m_gnDialog.m_cbSaveEyeHighlight,BM_GETCHECK,0,0) == BST_CHECKED;
		}
		inline void SetSaveHighlight(BOOL state) {
			SendMessage(m_gnDialog.m_cbSaveEyeHighlight,BM_SETCHECK,state ? BST_CHECKED : BST_UNCHECKED,0);
		}
	private:
	struct Dialog {
		HWND m_dialog;
		inline void Show(bool state) {
			ShowWindow(m_dialog, state ? SW_SHOW : SW_HIDE);
		}
		virtual void Refresh() = 0;
	};
	struct GNDialog : public Dialog {
		HWND m_cbSaveFiles;
		HWND m_cbSaveEyeTexture;
		HWND m_cbSaveEyeHighlight;
		HWND m_lbAAuSets;
		HWND m_edAAuSetName;
		HWND m_btnAAuSetAdd;

		void Refresh();
		void RefreshAAuSetList();
		static INT_PTR CALLBACK DialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam);
	} m_gnDialog;
	struct MODialog : public Dialog {
		HWND m_cbOverride;
		HWND m_edOverrideWith;
		HWND m_lbOverrides;
		
		void RefreshRuleList();
		void RefreshTextureList();
		void Refresh();
		static INT_PTR CALLBACK DialogProc(_In_ HWND hwndDlg, _In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam);
	} m_moDialog;
	struct AODialog : public Dialog {
		HWND m_edArchive;
		HWND m_edArchiveFile;
		HWND m_edOverrideFile;
		HWND m_btBrowse;
		HWND m_btApply;
		HWND m_lbOverrides;
		
		void RefreshRuleList();
		void Refresh();
		static INT_PTR CALLBACK DialogProc(_In_ HWND hwndDlg, _In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam);
	} m_aoDialog;
	struct ARDialog : public Dialog {
		HWND m_edArFrom;
		HWND m_edArTo;
		HWND m_edFileFrom;
		HWND m_edFileTo;
		HWND m_btApply;
		HWND m_lbOverrides;

		void RefreshRuleList();
		void Refresh();
		static INT_PTR CALLBACK DialogProc(_In_ HWND hwndDlg, _In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam);
	} m_arDialog;
	struct OODialog : public Dialog {
		HWND m_edObject;
		HWND m_edFile;
		HWND m_btnApply;
		HWND m_btnBrowse;
		HWND m_lbOverrides;

		void Refresh();
		void RefreshRuleList();
		static INT_PTR CALLBACK DialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam);
	} m_ooDialog;
	struct ETDialog : public Dialog {
		struct {
			HWND cbActive;
			HWND edFile;
			HWND btBrowse;
		} m_eye;

		void RefreshEnableState();
		void Refresh();
		static INT_PTR CALLBACK DialogProc(_In_ HWND hwndDlg, _In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam);
	} m_etDialog;
	struct TSDialog : public Dialog {
		HWND m_cbSelect;

		void LoadTanList();
		void Refresh();
		static INT_PTR CALLBACK DialogProc(_In_ HWND hwndDlg, _In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam);
	} m_tsDialog;
	struct HRDialog : public Dialog {
		HWND m_rbKind[4];
		HWND m_edSlot;
		HWND m_edAdjustment;
		HWND m_cbFlip;
		HWND m_lstHairs;

		HWND m_edHighlight;

		void RefreshHairList();
		void Refresh();
		static INT_PTR CALLBACK DialogProc(_In_ HWND hwndDlg, _In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam);
	} m_hrDialog;
	struct BDDialog : public Dialog {
		HWND m_cbOutlineColor;
		HWND m_edOutlineColorRed;
		HWND m_edOutlineColorGreen;
		HWND m_edOutlineColorBlue;

		HWND m_cbTanColor;
		HWND m_edTanColorRed;
		HWND m_edTanColorGreen;
		HWND m_edTanColorBlue;

		HWND m_bmBtnAdd;
		HWND m_bmCbXXFile;
		HWND m_bmCbBone;
		HWND m_bmList;
		HWND m_bmRbFrameMod;
		HWND m_bmRbBoneMod;
		HWND m_bmEdMatrix[3][3];
		
		void LoadData(int listboxId);
		void ApplyInput();
		void Refresh();
		static INT_PTR CALLBACK DialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam);
	} m_bdDialog;
	struct BSDialog : public Dialog {
		struct BodySlider {
			//windows
			HWND stLabel;
			HWND slider;
			HWND edit;

			//slider data
			float sliderMin;
			float sliderMax;
			std::vector<const Shared::Slider*> sliderData;
			const TCHAR* staticLabel;

			//current selection
			float currVal;

			BodySlider();
			BodySlider(HWND dialog,const TCHAR* label, int xStart,int yStart,
				std::vector<const Shared::Slider*> sliderData, float min, float max);

			float GetCoeffFromMod(AAUCardData::BoneMod);
			void Sync(bool useEdit);
			void FromCard();

			float Sld2Val(int sld);
			int Val2Sld(float val);
		};
		
		std::vector<BodySlider> m_sliders;
			
		void ApplySlider(int index);
		void Refresh();
		static INT_PTR CALLBACK DialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam);
	} m_bsDialog;
	friend AAEdit::loc_AddData;
	friend AAEdit::loc_AddActionData;
	friend AAEdit::loc_AddVariableData;
	friend AAEdit::loc_ModuleInfo;
	struct TRDialog : public Dialog {
		HWND m_lbTriggers;
		HWND m_tvTrigger;

		HTREEITEM m_tiEvents;
		HTREEITEM m_tiVariables;
		HTREEITEM m_tiActions;
		int m_currentTriggerIndex;
		Shared::Triggers::Trigger* m_currentTrigger = NULL;
		bool m_allowTriggerChange;
		std::vector<HTREEITEM> m_events;
		std::vector<HTREEITEM> m_variables;
		struct ActionTreeItem {
			HTREEITEM tree;
			std::vector<ActionTreeItem*> subactions;
			HTREEITEM subActionLabel; //some actions may have a subaction label for easier adding
			ActionTreeItem* parent;
			inline ~ActionTreeItem() {
				for (auto* item : subactions) delete item;
			}
		};
		std::vector<ActionTreeItem*> m_actions;

		void SetCurrentTrigger(int index);
		ActionTreeItem* AddTriggerAction(const Shared::Triggers::ParameterisedAction& action);
		void AddTriggerGuiActions(std::vector<Shared::Triggers::Trigger::GUIAction*> actions);
		struct SelectedAction_Data {
			//so that cardActions[cardActionsInt] is the selected action
			std::vector<Shared::Triggers::Trigger::GUIAction*>* cardActions = NULL;
			int cardActionsInt = -1;
			std::vector<ActionTreeItem*>* guiActions = NULL;
			bool isSubLabel = false;

			operator bool() { return cardActionsInt != -1; }
		};
		SelectedAction_Data GetSelectedAction();
		SelectedAction_Data GetSelectedAction(HTREEITEM selectedItem);
		void AddTriggerEvent(const Shared::Triggers::ParameterisedEvent& event,int insertAfter);
		int GetSelectedEvent();
		void AddTriggerVariable(const Shared::Triggers::Variable& var,int insertAfter);
		int GetSelectedVariable();
		HTREEITEM GenerateActionSubLabel(HTREEITEM parent, int actionId);

		std::wstring EVANameToString(const std::wstring& name,const std::vector<Shared::Triggers::ParameterisedExpression>& actualParameters);
		std::wstring ExpressionToString(const Shared::Triggers::ParameterisedExpression& param);

		void InitializeTriggers();
		void DoAddAction();
		void DoAddVariable();
		void DoAddEvent();
		static INT_PTR CALLBACK AddActionDialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam);
		static INT_PTR CALLBACK AddVariableDialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam);
		static INT_PTR CALLBACK AddGlobalVariableDialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam);

		void RefreshTriggerList();
		void RefreshTriggerActions();
		void Refresh();
		static INT_PTR CALLBACK DialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam);
	} m_trDialog;
	struct MDDialog : public Dialog {
		HWND m_lbModulesAvailable;
		HWND m_lbModulesUsed;
		HWND m_edName;
		HWND m_edDescr;
		std::vector<ModuleFile> m_modules;

		void Refresh();
		static INT_PTR CALLBACK DialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam);
	} m_mdDialog;


	HWND m_dialog;
	HWND m_tabs;

	bool m_visible;

	void AddDialog(int resourceName, Dialog* dialog, int tabN, const TCHAR* tabName, 
			INT_PTR (CALLBACK *DialogProc)(HWND,UINT,WPARAM,LPARAM));
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