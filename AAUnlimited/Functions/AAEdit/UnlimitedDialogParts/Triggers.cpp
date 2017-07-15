#include "Triggers.h"

#include <Windows.h>
#include <CommCtrl.h>
#include <queue>

#pragma comment(lib, "comctl32.lib")

#include "resource.h"
#include "Files\ModuleFile.h"
#include "Functions\Serialize.h"
#include "General\Util.h"
#include "Functions\AAEdit\Globals.h"
#include "Functions\Shared\Triggers\Value.h"
#include "Functions\Shared\Triggers\NamedConstant.h"
#include "Functions\Shared\Triggers\Expressions.h"
#include "Functions\Shared\Triggers\Actions.h"
#include "Functions\Shared\Triggers\Triggers.h"


namespace AAEdit {

using namespace Shared::Triggers;

/****************/
/* Get name box */
/****************/

INT_PTR CALLBACK GetNamePopupProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam);
INT_PTR CALLBACK ExportModuleDialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam);

std::wstring GetNameFromPopup(HWND parent, std::wstring initValue) {
	int result = DialogBoxParam(General::DllInst,MAKEINTRESOURCE(IDD_TRIGGERS_ENTERNAME),parent,&GetNamePopupProc,(LPARAM)&initValue);
	if (result == 0) return TEXT("");
	else return initValue;
}

INT_PTR CALLBACK GetNamePopupProc(_In_ HWND hwndDlg,_In_ UINT msg,	_In_ WPARAM wparam,_In_ LPARAM lparam) {
	switch (msg) {
	case WM_INITDIALOG: {
		//initialize dialog members from the loaded dialog
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lparam); //register wstring to hwnd
		std::wstring* str = (std::wstring*)lparam;
		if(str->size() != 0) {
			SendMessage(GetDlgItem(hwndDlg,IDC_TR_IN_EDNAME),WM_SETTEXT,0,(LPARAM)str->c_str());
			EnableWindow(GetDlgItem(hwndDlg,IDC_TR_IN_BTNOK),TRUE);
		}
		else {
			EnableWindow(GetDlgItem(hwndDlg,IDC_TR_IN_BTNOK),FALSE);
		}
		return TRUE;
		break; }
	case WM_COMMAND: {
		DWORD identifier = LOWORD(wparam);
		DWORD notification = HIWORD(wparam);
		HWND wnd = (HWND)lparam;
		switch (identifier) {
		case IDC_TR_IN_EDNAME:
			if(notification == EN_CHANGE) {
				std::wstring* str = (std::wstring*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
				*str = General::GetEditString(wnd);
				if(str->size() != 0) {
					EnableWindow(GetDlgItem(hwndDlg,IDC_TR_IN_BTNOK),TRUE);
				}
				else {
					EnableWindow(GetDlgItem(hwndDlg,IDC_TR_IN_BTNOK),FALSE);
				}
				return TRUE;
			}
		case IDC_TR_IN_BTNOK:
			if (notification == BN_CLICKED) {
				EndDialog(hwndDlg,1);
				return TRUE;
			}
			break;
		case IDC_TR_IN_BTNCANCLE:
			if (notification == BN_CLICKED) {
				EndDialog(hwndDlg,0);
				return TRUE;
			}
			break;
		}
		break; }
	}
	return FALSE;
}


std::wstring UnlimitedDialog::TRDialog::ExpressionToString(const ParameterisedExpression& param) {
	if (param.expression == NULL) return std::wstring(TEXT("(invalid)"));
	else if (param.expression->id == EXPR_CONSTANT) {
		//constant
		std::wstring retVal;
		switch (param.constant.type) {
		case TYPE_INT:
			retVal = std::to_wstring(param.constant.iVal);
			break;
		case TYPE_FLOAT:
			retVal = std::to_wstring(param.constant.fVal);
			break;
		case TYPE_BOOL:
			if (param.constant.bVal) retVal = TEXT("true");
			else retVal = TEXT("false");
			break;
		case TYPE_STRING:
			if (param.constant.strVal) {
				retVal = *param.constant.strVal;
			}
			break;
		default:
			retVal = TEXT("invalid");
			break;
		}
		return retVal;
	}
	else if (param.expression->id == EXPR_NAMEDCONSTANT) {
		if (param.namedConstant == NULL) return TEXT("error");
		return param.namedConstant->name;
	}
	else if (param.expression->id == EXPR_VAR) {
		//variable
		//find in global vars
		auto& list = g_currChar.m_cardData.GetGlobalVariables();
		for(auto& var : list) {
			if(var.name == param.varName) {
				return var.name;
			}
		}
		//find in local vars
		if(m_currentTrigger == NULL) {
			return TEXT("Error");
		}
		Variable* var = m_currentTrigger->FindVar(param.varName);
		if(var == NULL) {
			return TEXT("Error");
		}
		return var->name;
	}
	else if (param.expression->id == EXPR_NAMEDCONSTANT) {
		if(param.namedConstant == NULL) {
			return TEXT("Error");
		}
		return param.namedConstant->interactiveName;
	}
	else {
		//function call
		const std::wstring& name = param.expression->interactiveName;
		return EVANameToString(name,param.actualParameters);
	}

	return TEXT("()");
}

std::wstring UnlimitedDialog::TRDialog::EVANameToString(const std::wstring& name,const std::vector<ParameterisedExpression>& actualParameters) {
	std::wstring retVal;
	//split string at whitespaces
	int n = 0;
	int last = 0,i = 1;
	for (i = 1; i < name.size(); i++) {
		if (isspace(name[i]) && i != last) {
			std::wstring part = name.substr(last,i-last);
			if (retVal.size() != 0) retVal.push_back(L' ');
			if (part == TEXT("%p")) {
				std::wstring paramString = ExpressionToString(actualParameters[n++]);
				retVal.push_back(L'(');
				retVal += paramString;
				retVal.push_back(L')');
			}
			else {
				retVal += part;
			}
			last = i+1;
		}
	}
	std::wstring part = name.substr(last,i-last);
	if (retVal.size() != 0) retVal.push_back(L' ');
	if (part == TEXT("%p")) {
		std::wstring paramString = ExpressionToString(actualParameters[n++]);
		retVal.push_back(L'(');
		retVal += paramString;
		retVal.push_back(L')');
	}
	else {
		retVal += part;
	}
	last = i+1;
	return retVal;
}

/*************************
 * Trigger (main) window *
 *************************/

void UnlimitedDialog::TRDialog::SetCurrentTrigger(int index) {
	m_currentTriggerIndex = index;
	auto& trigList = g_currChar.m_cardData.GetTriggers();
	Trigger* trg = NULL;
	if(index >= 0 && (unsigned int)index < trigList.size()) {
		trg = &trigList[index];
		m_allowTriggerChange = true;
	}
	else if(index >= 0) {
		//look inside module triggers
		int cnt = index - trigList.size();
		for(auto& mod : g_currChar.m_cardData.GetModules()) {
			if(cnt >= mod.triggers.size()) {
				cnt -= mod.triggers.size();
				continue;
			}
			else {
				trg = (Trigger*)&mod.triggers[cnt];
				m_allowTriggerChange = false;
				break;
			}
		}
	}
	
	
	m_currentTrigger = trg;
	
	for(auto* item : m_actions) {
		delete item;
	}
	m_actions.clear();
	m_variables.clear();
	m_events.clear();
	if (trg == NULL) return;
	InitializeTriggers();

	TVINSERTSTRUCT telem;

	//events
	telem.hParent = m_tiEvents;
	telem.hInsertAfter = TVI_LAST;
	telem.item.mask = TVIF_TEXT | TVIF_PARAM;
	for(int i = 0; i < m_currentTrigger->events.size(); i++) {
		auto& elem = m_currentTrigger->events[i];
		std::wstring str = EVANameToString(elem.event->interactiveName,elem.actualParameters);
		telem.item.pszText = (LPWSTR)str.c_str();
		telem.item.cchTextMax = str.size();
		telem.item.lParam = -1;
		HTREEITEM ti = TreeView_InsertItem(m_tvTrigger,&telem);
		m_events.push_back(ti);
	}
	TreeView_Expand(m_tvTrigger,m_tiEvents,TVE_EXPAND);

	//variables
	telem.hParent = m_tiVariables;
	for (int i = 0; i < m_currentTrigger->vars.size(); i++) {
		auto& elem = m_currentTrigger->vars[i];
		std::wstring str = g_Types[elem.type].name + TEXT(" ") + elem.name;
		str += TEXT(" = ");
		str += ExpressionToString(elem.defaultValue);
		telem.item.pszText = (LPWSTR)str.c_str();
		telem.item.cchTextMax = str.size();
		telem.item.lParam = -1;
		HTREEITEM ti = TreeView_InsertItem(m_tvTrigger,&telem);
		m_variables.push_back(ti);
	}
	TreeView_Expand(m_tvTrigger,m_tiVariables,TVE_EXPAND);

	//actions
	struct LoopData {
		HTREEITEM parent;
		ActionTreeItem* guiParent;
		std::vector<Trigger::GUIAction*>& cardActions;
		std::vector<ActionTreeItem*>& guiActions;
	};
	std::queue<LoopData> guiActionQueue;
	guiActionQueue.push({ m_tiActions, NULL, m_currentTrigger->guiActions, m_actions});
	
	while(!guiActionQueue.empty()) {
		//each element represents a plain of actions, both its card and its gui parts, and their parents
		auto queueElem = guiActionQueue.front();
		guiActionQueue.pop();

		std::vector<Trigger::GUIAction*>& actions = queueElem.cardActions;
		std::vector<ActionTreeItem*>& guiActions = queueElem.guiActions;
		HTREEITEM parent = queueElem.parent;
		//for each action in this plain, add an action item to the tree view
		for(auto& elem : actions) {
			std::wstring str = EVANameToString(elem->action.action->interactiveName,elem->action.actualParameters);
			telem.hParent = parent;
			telem.item.pszText = (LPWSTR)str.c_str();
			telem.item.cchTextMax = str.size();
			telem.item.lParam = -1;
			HTREEITEM ti = TreeView_InsertItem(m_tvTrigger,&telem);

			//register that tree view item in gui vector
			ActionTreeItem* item = new ActionTreeItem;
			item->parent = queueElem.guiParent;
			item->tree = ti;
			item->subActionLabel = GenerateActionSubLabel(ti,elem->action.action->id);
			TreeView_Expand(m_tvTrigger,ti,TVE_EXPAND);
			guiActions.push_back(item);

			//add subactions to the queue
			LoopData ldata {ti, item, elem->subactions, item->subactions };
			guiActionQueue.push(std::move(ldata));
		}
	}
	TreeView_Expand(m_tvTrigger,m_tiActions,TVE_EXPAND);
		
	
}

//returns index of selected action in array, or -1 if no event is selected
UnlimitedDialog::TRDialog::SelectedAction_Data UnlimitedDialog::TRDialog::GetSelectedAction(HTREEITEM tvSel) {
	//find selected action, if action is selected
	struct LoopData {
		std::vector<Trigger::GUIAction*>& cardActions;
		std::vector<ActionTreeItem*>& guiActions;
	};

	SelectedAction_Data retVal;

	std::queue<LoopData> actionQueue;
	actionQueue.push({ m_currentTrigger->guiActions, m_actions });
	while (!actionQueue.empty()) {
		auto elem = actionQueue.front();
		actionQueue.pop();

		for (int i = 0; i < elem.guiActions.size(); i++) {
			if (elem.guiActions[i]->tree == tvSel) {
				//found it
				retVal.cardActions = &elem.cardActions;
				retVal.guiActions = &elem.guiActions;
				retVal.cardActionsInt = i;
				retVal.isSubLabel = false;
				break;
			}
			else if (elem.guiActions[i]->subActionLabel == tvSel) {
				retVal.cardActions = &elem.cardActions;
				retVal.guiActions = &elem.guiActions;
				retVal.cardActionsInt = i;
				retVal.isSubLabel = true;
				break;
			}
			//abort if found
			if (retVal.cardActionsInt != -1) break;
			//add subactions to queue
			if (elem.cardActions[i]->subactions.size() > 0) {
				actionQueue.push({ elem.cardActions[i]->subactions, elem.guiActions[i]->subactions });
			}
		}
	}
	return retVal;
}
UnlimitedDialog::TRDialog::SelectedAction_Data UnlimitedDialog::TRDialog::GetSelectedAction() {
	HTREEITEM tvSel = TreeView_GetSelection(m_tvTrigger);
	return GetSelectedAction(tvSel);
}

HTREEITEM UnlimitedDialog::TRDialog::GenerateActionSubLabel(HTREEITEM parent, int actionId) {
	TVINSERTSTRUCT telem;
	telem.item.mask = TVIF_TEXT;
	telem.item.pszText = NULL;
	telem.hParent = parent;
	telem.hInsertAfter = TVI_FIRST;

	switch (actionId) {
	case ACTION_IF:
	case ACTION_ELSEIF:
	case ACTION_ELSE:
	case ACTION_LOOP:
	case ACTION_FORLOOP:
		telem.item.pszText = TEXT("Actions:");
		break;
	default:
		break;
	};

	if(telem.item.pszText != NULL) {
		return TreeView_InsertItem(m_tvTrigger,&telem);
	}
	return NULL;
}

UnlimitedDialog::TRDialog::ActionTreeItem* UnlimitedDialog::TRDialog::AddTriggerAction(const ParameterisedAction& action) {
	if (m_currentTrigger == NULL) return NULL;
	
	SelectedAction_Data sel = GetSelectedAction();

	//first, determine where to add the tree item
	TVINSERTSTRUCT telem;
	if (sel.cardActionsInt == -1) {
		//nothing selected, add to start
		telem.hParent = m_tiActions;
		telem.hInsertAfter = TVI_FIRST;
	}
	else if(!sel.isSubLabel) {
		//normal action selected, add after the selected action, parent being the parent of the selected action
		telem.hParent = (*sel.guiActions)[sel.cardActionsInt]->parent == NULL ? m_tiActions : (*sel.guiActions)[sel.cardActionsInt]->parent->tree;
		telem.hInsertAfter = (*sel.guiActions)[sel.cardActionsInt]->tree;
	}
	else {
		//action label, add as child of the selected action
		telem.hParent = (*sel.guiActions)[sel.cardActionsInt]->tree;
		telem.hInsertAfter = (*sel.guiActions)[sel.cardActionsInt]->subActionLabel;
	}
	
	telem.item.mask = TVIF_TEXT | TVIF_PARAM;
	std::wstring str = EVANameToString(action.action->interactiveName,action.actualParameters);
	telem.item.pszText = (LPWSTR)str.c_str();
	telem.item.cchTextMax = str.size();
	telem.item.lParam = -1;
	HTREEITEM ti = TreeView_InsertItem(m_tvTrigger,&telem);
	TreeView_Expand(m_tvTrigger,ti,TVE_EXPAND);

	//now, add the action to the data (both gui and card)
	ActionTreeItem* tItem = new ActionTreeItem;
	tItem->tree = ti;
	tItem->subActionLabel = GenerateActionSubLabel(ti,action.action->id);
	if (sel.cardActionsInt == -1) {
		tItem->parent = NULL;
		m_actions.insert(m_actions.begin(), tItem);
		Trigger::GUIAction* newCardAction = new Trigger::GUIAction;
		newCardAction->parent = NULL;
		newCardAction->action = action;
		m_currentTrigger->guiActions.insert(m_currentTrigger->guiActions.begin(), newCardAction);
		TreeView_Expand(m_tvTrigger,m_tiActions,TVE_EXPAND);
	}
	else if(!sel.isSubLabel) {
		tItem->parent = (*sel.guiActions)[sel.cardActionsInt]->parent;
		sel.guiActions->insert(sel.guiActions->begin() + sel.cardActionsInt + 1,tItem);
		Trigger::GUIAction* newCardAction = new Trigger::GUIAction;
		newCardAction->parent = (*sel.cardActions)[sel.cardActionsInt]->parent;
		newCardAction->action = action;
		sel.cardActions->insert(sel.cardActions->begin() + sel.cardActionsInt + 1,newCardAction);
	}
	else {
		tItem->parent = (*sel.guiActions)[sel.cardActionsInt];
		(*sel.guiActions)[sel.cardActionsInt]->subactions.insert((*sel.guiActions)[sel.cardActionsInt]->subactions.begin(),tItem);
		Trigger::GUIAction* newCardAction = new Trigger::GUIAction;
		newCardAction->parent = (*sel.cardActions)[sel.cardActionsInt];
		newCardAction->action = action;
		(*sel.cardActions)[sel.cardActionsInt]->subactions.insert((*sel.cardActions)[sel.cardActionsInt]->subactions.begin(),newCardAction);
	}
	return tItem;
}

void UnlimitedDialog::TRDialog::AddTriggerGuiActions(std::vector<Shared::Triggers::Trigger::GUIAction*> actions) {

	//save starting selection

	SelectedAction_Data actionSel = GetSelectedAction();
	HTREEITEM startSel;
	if(actionSel.cardActionsInt != -1) {
		startSel = TreeView_GetSelection(m_tvTrigger);
	}
	else {
		startSel = m_tiActions;
	}
	

	struct LoopData {
		HTREEITEM after;
		std::vector<Trigger::GUIAction*>& actions;
	};
	std::queue<LoopData> guiActionQueue;
	guiActionQueue.push({ startSel, actions });

	while (!guiActionQueue.empty()) {
		auto queueElem = guiActionQueue.front();
		guiActionQueue.pop();
		std::vector<Trigger::GUIAction*>& actions = queueElem.actions;
		HTREEITEM insertAfter = queueElem.after;
		TreeView_SelectItem(m_tvTrigger,insertAfter);
		
		for (auto& elem : actions) {
			ActionTreeItem* newItem = AddTriggerAction(elem->action);
			TreeView_SelectItem(m_tvTrigger,newItem->tree);
			//add subactions to the queue if applicable
			if(elem->subactions.size()) {
				guiActionQueue.push({ newItem->subActionLabel, elem->subactions });
			}
			
		}
	}

	//restore selection afterwards
	TreeView_SelectItem(m_tvTrigger,startSel);
}

void UnlimitedDialog::TRDialog::AddTriggerEvent(const Shared::Triggers::ParameterisedEvent& event,int insertAfter) {
	if (m_currentTrigger == NULL) return;
	m_currentTrigger->InsertEvent(event,insertAfter);

	HTREEITEM ti;
	if (insertAfter == -1) {
		ti = m_tiEvents;
	}
	else {
		ti = m_events[insertAfter];
	}
	TVINSERTSTRUCT telem;
	telem.hParent = m_tiEvents;
	telem.hInsertAfter = ti;
	telem.item.mask = TVIF_TEXT | TVIF_PARAM;
	std::wstring str = EVANameToString(event.event->interactiveName,event.actualParameters);
	telem.item.pszText = (LPWSTR)str.c_str();
	telem.item.cchTextMax = str.size();
	telem.item.lParam = -1;
	ti = TreeView_InsertItem(m_tvTrigger,&telem);
	m_events.insert(m_events.begin() + (insertAfter+1),ti);
	TreeView_Expand(m_tvTrigger,ti,TVE_EXPAND);
}

//returns index of selected event in array, or -1 if no event is selected
int UnlimitedDialog::TRDialog::GetSelectedEvent() {
	HTREEITEM tv = TreeView_GetSelection(m_tvTrigger);
	int i;
	for (i = 0; i < m_events.size(); i++) {
		HTREEITEM hEvent = m_events[i];
		if (hEvent == tv) {
			break;
		}
	}
	if (i == m_events.size()) {
		//not found
		return -1;
	}
	else {
		return i;
	}
}

void UnlimitedDialog::TRDialog::AddTriggerVariable(const Shared::Triggers::Variable& var,int insertAfter) {
	if (m_currentTrigger == NULL) return;
	m_currentTrigger->InsertVariable(var,insertAfter);

	HTREEITEM ti;
	if (insertAfter == -1) {
		ti = m_tiVariables;
	}
	else {
		ti = m_variables[insertAfter];
	}
	TVINSERTSTRUCT telem;
	telem.hParent = m_tiVariables;
	telem.hInsertAfter = ti;
	telem.item.mask = TVIF_TEXT | TVIF_PARAM;
	std::wstring str = g_Types[var.type].name + TEXT(" ") + var.name;
	str += TEXT(" = ");
	str += ExpressionToString(var.defaultValue);
	telem.item.pszText = (LPWSTR)str.c_str();
	telem.item.cchTextMax = str.size();
	telem.item.lParam = -1;
	ti = TreeView_InsertItem(m_tvTrigger,&telem);
	m_variables.insert(m_variables.begin() + (insertAfter+1),ti);
	TreeView_Expand(m_tvTrigger,ti,TVE_EXPAND);
}

//returns index of selected variable in array, or -1 if no event is selected
int UnlimitedDialog::TRDialog::GetSelectedVariable() {
	HTREEITEM tv = TreeView_GetSelection(m_tvTrigger);
	int i;
	for (i = 0; i < m_variables.size(); i++) {
		HTREEITEM hVar = m_variables[i];
		if (hVar== tv) {
			break;
		}
	}
	if (i == m_variables.size()) {
		//not found
		return -1;
	}
	else {
		return i;
	}
}

INT_PTR CALLBACK UnlimitedDialog::TRDialog::DialogProc(_In_ HWND hwndDlg,_In_ UINT msg,
	_In_ WPARAM wparam,_In_ LPARAM lparam)
{
	switch (msg) {
	case WM_INITDIALOG: {
		//initialize dialog members from the loaded dialog
		TRDialog* thisPtr = (TRDialog*)lparam;
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lparam); //register class to this hwnd
		thisPtr->m_dialog = hwndDlg;
		thisPtr->m_lbTriggers = GetDlgItem(hwndDlg,IDC_TR_LBTRIGGERLIST);
		thisPtr->m_tvTrigger = GetDlgItem(hwndDlg,IDC_TR_TVTRIGGER);
		MakeTreeViewMultiselect(thisPtr->m_tvTrigger);

		thisPtr->Refresh();
		return TRUE;
		break; }
	case WM_VKEYTOITEM: {
		//DEL-key was pressed while the list box had the focus
		TRDialog* thisPtr = (TRDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		if (LOWORD(wparam) == VK_DELETE) {
			
			
		}
		break; }

	case WM_COMMAND: {
		TRDialog* thisPtr = (TRDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		switch (HIWORD(wparam)) {
		case BN_CLICKED: {
			DWORD identifier = LOWORD(wparam);
			switch(identifier) {
			case IDC_TR_BTNGLOBALS: {
				int result = DialogBoxParam(General::DllInst,MAKEINTRESOURCE(IDD_TRIGGERS_GLOBALVARLIST),hwndDlg,&AddGlobalVariableDialogProc,(LPARAM)thisPtr);
				break; }
			}
			break; }
		case LBN_SELCHANGE: {
			HWND wnd = (HWND)lparam;
			if(wnd == thisPtr->m_lbTriggers) {
				int sel = SendMessage(thisPtr->m_lbTriggers,LB_GETCURSEL,0,0);
				if(sel != LB_ERR) {
					thisPtr->SetCurrentTrigger(sel);
					EnableWindow(thisPtr->m_tvTrigger,TRUE);
					return TRUE;
				}
				else {
					EnableWindow(thisPtr->m_tvTrigger,FALSE);
					thisPtr->SetCurrentTrigger(NULL);
				}
			}
			break; }
		}
		break; }
	case WM_CONTEXTMENU: {
		TRDialog* thisPtr = (TRDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		HWND wnd = (HWND)wparam;
		if(wnd == thisPtr->m_lbTriggers) {
			POINT cursor;
			GetCursorPos(&cursor);
			RECT lbRect;
			GetWindowRect(GetDlgItem(hwndDlg,IDC_TR_LBTRIGGERLIST),&lbRect);
			if (PtInRect(&lbRect,cursor)) {
				HMENU menu;
				HMENU subMenu;

				menu = LoadMenu(General::DllInst,MAKEINTRESOURCE(IDR_TRIGGER_LIST));

				subMenu = GetSubMenu(menu,0);
				if(!thisPtr->m_allowTriggerChange) {
					EnableMenuItem(subMenu,ID_TRM_DELETETRIGGER,MF_GRAYED);
					EnableMenuItem(subMenu,ID_TRM_EXPORT,MF_GRAYED);
					EnableMenuItem(subMenu,ID_TRM_RENAMETRIGGER,MF_GRAYED);
				}

				BOOL ret = TrackPopupMenu(subMenu,TPM_LEFTALIGN | TPM_RETURNCMD,
					cursor.x,cursor.y,0,thisPtr->m_dialog,NULL);
				switch (ret) {
				case 0: //no selection
					break;
				case ID_TRM_ADDTRIGGER: {
					Trigger trg;
					trg.name = TEXT("new trigger");
					int ind = AAEdit::g_currChar.m_cardData.GetTriggers().size();
					AAEdit::g_currChar.m_cardData.GetTriggers().push_back(trg);
					int currSel = SendMessage(thisPtr->m_lbTriggers,LB_GETCURSEL,0,0);
					SendMessage(thisPtr->m_lbTriggers,LB_INSERTSTRING,ind,(LPARAM)trg.name.c_str());
					if (currSel <= ind) currSel++;
					SendMessage(thisPtr->m_lbTriggers,LB_SETCURSEL,currSel,0);
					break; }
				case ID_TRM_DELETETRIGGER: {
					int sel = SendMessage(thisPtr->m_lbTriggers,LB_GETCURSEL,0,0);
					if(sel != LB_ERR) {
						auto& triggerList = g_currChar.m_cardData.GetTriggers();
						if(sel >= 0 && sel < triggerList.size()) {
							triggerList.erase(triggerList.begin() + sel);
							SendMessage(thisPtr->m_lbTriggers,LB_DELETESTRING,sel,0);
						}
					}
					break; }
				case ID_TRM_RENAMETRIGGER: {
					int sel = SendMessage(thisPtr->m_lbTriggers,LB_GETCURSEL,0,0);
					if(sel != LB_ERR) {
						auto& triggerList = g_currChar.m_cardData.GetTriggers();
						if (sel >= 0 && sel < triggerList.size()) {
							std::wstring newName = GetNameFromPopup(hwndDlg,triggerList[sel].name);
							if(newName.size() != 0) {
								triggerList[sel].name = newName;
								SendMessage(thisPtr->m_lbTriggers,LB_DELETESTRING,sel,0); //wierd workaround
								SendMessage(thisPtr->m_lbTriggers,LB_INSERTSTRING,sel,(LPARAM)newName.c_str());
							}
						}
					}
					break; }
				case ID_TRM_COPYSEL: {
					int nSel = SendMessage(thisPtr->m_lbTriggers, LB_GETSELCOUNT,0,0);
					if(nSel > 0) {
						std::vector<int> itemBuffer(nSel);
						SendMessage(thisPtr->m_lbTriggers,LB_GETSELITEMS,nSel,(LPARAM)itemBuffer.data());
						std::vector<Trigger*> triggers;
						for (int item : itemBuffer) triggers.push_back(&g_currChar.m_cardData.GetTriggers()[item]);
						char* buffer = NULL;
						int size = 0;
						int at = 0;
						Serialize::WriteData(&buffer,&size,at,triggers,true);
						if (buffer != NULL) {
							std::wstring stringVersion = Serialize::WStringFromBuffer(buffer,at);
							OpenClipboard(hwndDlg);
							HGLOBAL globalMem = (wchar_t*)GlobalAlloc(GMEM_MOVEABLE,stringVersion.size()*2 + 2);
							wchar_t* lockedMem = (wchar_t*)GlobalLock(globalMem);
							memcpy(lockedMem,stringVersion.c_str(),stringVersion.size()*2 + 2);
							GlobalUnlock(globalMem);
							SetClipboardData(CF_UNICODETEXT,globalMem);
							CloseClipboard();
						}
						delete[] buffer;
					}
					break; }
				case ID_TRM_PASTE: {
					if (thisPtr->m_currentTrigger != NULL) {
						thisPtr->SetCurrentTrigger(-1);
					}
					OpenClipboard(hwndDlg);
					HANDLE h = GetClipboardData(CF_UNICODETEXT);
					if (h == NULL) {
						CloseClipboard();
						break;
					}
					int memSize = GlobalSize(h);
					int strLength = 0;
					bool valid = false;
					wchar_t* ptr = (wchar_t*)GlobalLock(h);
					if (ptr != NULL) {
						for (int i = 0; i < memSize/2; i++) {
							if (ptr[i] == L'\0') {
								valid = true;
								break;
							}
							strLength++;
						}
					}
					if (valid) {
						char* buffer = Serialize::BufferFromWString(ptr,strLength);
						if (buffer != NULL) {
							char* tmpBuffer = buffer;
							int tmpLength = strLength;
							try {
								std::vector<Trigger*> actions = Serialize::ReadData<decltype(actions)>(tmpBuffer,tmpLength);
								auto& trigList = g_currChar.m_cardData.GetTriggers();
								for (Trigger* action : actions) {
									trigList.push_back(*action);
									delete action;
								}
								thisPtr->RefreshTriggerList();
							}
							catch (Serialize::InsufficientBufferException e) {

							}
							catch (std::exception e) {

							}

						}
						delete[] buffer;
					}
					GlobalUnlock(h);
					CloseClipboard();
					
					break; }
				case ID_TRM_EXPORT: {
					
					int selCount = SendMessage(thisPtr->m_lbTriggers,LB_GETSELCOUNT,0,0);
					if(selCount > 0) {
						loc_ModuleInfo info;
						info.thisPtr = thisPtr;
						int* selBuffer = new int[selCount];
						SendMessage(thisPtr->m_lbTriggers,LB_GETSELITEMS,selCount,(LPARAM)selBuffer);
						auto& triggers = g_currChar.m_cardData.GetTriggers();
						for(int i = 0; i < selCount; i++) {
							info.saveTriggers.push_back(triggers[selBuffer[i]].name);
						}
						
						std::vector<Trigger*> trigs;
						for (int i = 0; i < selCount; i++) {
							trigs.push_back(&triggers[selBuffer[i]]);
						}

						Module mod;
						mod.GenerateTrigGlobalDepend(trigs,g_currChar.m_cardData.GetGlobalVariables(),g_currChar.m_cardData.GetModules());
						
						info.dependencies = mod.dependencies;
						for(auto& var : mod.globals) {
							info.saveGlobals.push_back(&var);
						}
						
						int result = DialogBoxParam(General::DllInst,MAKEINTRESOURCE(IDD_TRIGGERS_EXPORT),hwndDlg,&ExportModuleDialogProc,(LPARAM)&info);
						if (result == 1 && info.name.size() > 0) {
							std::wstring path = General::BuildOverridePath(MODULE_PATH, info.name.c_str());
							mod.name = info.name;
							mod.description = info.description;

							ModuleFile file(mod);
							file.WriteToFile(path.c_str());
						}

						delete[] selBuffer;
					}
					break; }
				default:
					break;
				}
				DestroyMenu(menu);
				return TRUE;
			}
		}
		break; }
	case WM_NOTIFY: {
		TRDialog* thisPtr = (TRDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		NMHDR* info = (NMHDR*)lparam;
		if (info->code == NM_RCLICK && info->hwndFrom == thisPtr->m_tvTrigger) {
			//first, select the targeted tree item
			HTREEITEM target = TreeView_GetDropHilight(thisPtr->m_tvTrigger);
			if(target != NULL) {
				TreeView_SelectItem(thisPtr->m_tvTrigger,target);
			}

			HMENU menu;
			HMENU subMenu;
			POINT cursor; GetCursorPos(&cursor);

			menu = LoadMenu(General::DllInst,MAKEINTRESOURCE(IDR_TRIGGER_EVA));

			subMenu = GetSubMenu(menu,0);
			if(!thisPtr->m_allowTriggerChange) {
				EnableMenuItem(subMenu,ID_TRM_ADDEVENT,MF_GRAYED);
				EnableMenuItem(subMenu,ID_TRM_ADDVARIABLE,MF_GRAYED);
				EnableMenuItem(subMenu,ID_TRM_ADDACTION,MF_GRAYED);
				EnableMenuItem(subMenu,ID_TRM_DELETESELECTION,MF_GRAYED);
				EnableMenuItem(subMenu,ID_TRM_PASTE,MF_GRAYED);
			}

			BOOL ret = TrackPopupMenu(subMenu,TPM_LEFTALIGN | TPM_RETURNCMD,
				cursor.x,cursor.y,0,thisPtr->m_dialog,NULL);
			switch (ret) {
			case 0: //no selection
				break;
			case ID_TRM_ADDEVENT:
				thisPtr->DoAddEvent();
				break;
			case ID_TRM_ADDVARIABLE:
				thisPtr->DoAddVariable();
				break;
			case ID_TRM_ADDACTION:
				thisPtr->DoAddAction();
				break;
			case ID_TRM_DELETESELECTION: {
				if (thisPtr->m_currentTrigger != NULL) {
					int sel;
					if ((sel = thisPtr->GetSelectedEvent()) != -1) {
						HTREEITEM item = thisPtr->m_events[sel];
						SendMessage(thisPtr->m_tvTrigger,TVM_DELETEITEM,0,(LPARAM)item);
						thisPtr->m_currentTrigger->events.erase(thisPtr->m_currentTrigger->events.begin() + sel);
						thisPtr->m_events.erase(thisPtr->m_events.begin() + sel);
						return TRUE;
					}
					else if ((sel = thisPtr->GetSelectedVariable()) != -1) {
						HTREEITEM item = thisPtr->m_variables[sel];
						SendMessage(thisPtr->m_tvTrigger,TVM_DELETEITEM,0,(LPARAM)item);
						thisPtr->m_currentTrigger->vars.erase(thisPtr->m_currentTrigger->vars.begin() + sel);
						thisPtr->m_variables.erase(thisPtr->m_variables.begin() + sel);
						return TRUE;
					}
					else if (SelectedAction_Data selData = thisPtr->GetSelectedAction()) {
						std::vector<HTREEITEM>* multiSel = (std::vector<HTREEITEM>*)SendMessage(thisPtr->m_tvTrigger,TVM_GETMULTISEL,0,0);
						if(multiSel != NULL) {
							std::vector<HTREEITEM> copy = *multiSel;
							for(HTREEITEM item : copy) {
								selData = thisPtr->GetSelectedAction(item);
								if (selData.isSubLabel) break;
								HTREEITEM item = (*selData.guiActions)[selData.cardActionsInt]->tree;
								SendMessage(thisPtr->m_tvTrigger,TVM_DELETEITEM,0,(LPARAM)item);
								delete (*selData.guiActions)[selData.cardActionsInt];
								delete (*selData.cardActions)[selData.cardActionsInt];
								selData.guiActions->erase(selData.guiActions->begin() + selData.cardActionsInt);
								selData.cardActions->erase(selData.cardActions->begin() + selData.cardActionsInt);

							}
							
						}						
						return TRUE;
					}
				}
				break; }
			case ID_TRM_COPYSEL: {
				if (thisPtr->m_currentTrigger != NULL) {
					std::vector<Trigger::GUIAction*> actions;
					std::vector<HTREEITEM>* sel = (std::vector<HTREEITEM>*)SendMessage(thisPtr->m_tvTrigger,TVM_GETMULTISEL,0,0);
					for(auto item : *sel) {
						if (SelectedAction_Data selData = thisPtr->GetSelectedAction(item)) {
							actions.push_back((*selData.cardActions)[selData.cardActionsInt]);
						}
					}
					char* buffer = NULL;
					int size = 0;
					int at = 0;
					Serialize::WriteData(&buffer,&size,at,actions,true);
					if (buffer != NULL) {
						std::wstring stringVersion = Serialize::WStringFromBuffer(buffer,at);
						OpenClipboard(hwndDlg);
						HGLOBAL globalMem = (wchar_t*)GlobalAlloc(GMEM_MOVEABLE,stringVersion.size()*2 + 2);
						wchar_t* lockedMem = (wchar_t*)GlobalLock(globalMem);
						memcpy(lockedMem,stringVersion.c_str(),stringVersion.size()*2 + 2);
						GlobalUnlock(globalMem);
						SetClipboardData(CF_UNICODETEXT,globalMem);
						CloseClipboard();
					}
					delete[] buffer;
				}
				break; }
			case ID_TRM_PASTE: {
				if (thisPtr->m_currentTrigger != NULL) {
					OpenClipboard(hwndDlg);
					HANDLE h = GetClipboardData(CF_UNICODETEXT);
					if(h == NULL) {
						CloseClipboard();
						break;
					}
					int memSize = GlobalSize(h);
					int strLength = 0;
					bool valid = false;
					wchar_t* ptr = (wchar_t*)GlobalLock(h);
					if (ptr != NULL) {
						for(int i = 0; i < memSize/2; i++) {
							if(ptr[i] == L'\0') {
								valid = true;
								break;
							}
							strLength++;
						}
					}
					if(valid) {
						char* buffer = Serialize::BufferFromWString(ptr,strLength);
						if(buffer != NULL) {
							char* tmpBuffer = buffer;
							int tmpLength = strLength;
							try {
								std::vector<Trigger::GUIAction*> actions = Serialize::ReadData<decltype(actions)>(tmpBuffer,tmpLength);
								thisPtr->AddTriggerGuiActions(actions);
								for (Trigger::GUIAction* action : actions) delete action;
							}
							catch(Serialize::InsufficientBufferException e) {

							}
							catch(std::exception e) {

							}

						}
						delete[] buffer;
					}
					GlobalUnlock(h);
					CloseClipboard();
				}
				break; }
			default:
				break;
			}
			DestroyMenu(menu);
		}
		else if(info->code == NM_DBLCLK && info->hwndFrom == thisPtr->m_tvTrigger) {
			SelectedAction_Data sel = thisPtr->GetSelectedAction();
			if(thisPtr->m_allowTriggerChange && sel) {
				loc_AddActionData data;
				data.thisPtr = thisPtr;
				data.action = (*sel.cardActions)[sel.cardActionsInt]->action;
				data.valid.resize(data.action.actualParameters.size());
				for (int i = 0; i < data.valid.size(); i++) data.valid[i] = true;
				int result = DialogBoxParam(General::DllInst,MAKEINTRESOURCE(IDD_TRIGGERS_ADDACTION),hwndDlg,&AddActionDialogProc,(LPARAM)&data);
				if (result == 1) {
					if (thisPtr->m_currentTrigger) {
						(*sel.cardActions)[sel.cardActionsInt]->action = data.action;
						std::wstring str = thisPtr->EVANameToString(data.action.action->interactiveName,data.action.actualParameters);
						TVITEM item;
						item.mask = TVIF_TEXT;
						item.hItem = (*sel.guiActions)[sel.cardActionsInt]->tree;
						item.pszText = (LPWSTR)str.c_str();
						TreeView_SetItem(thisPtr->m_tvTrigger,&item);
					}
				}
			}
		}
		else if(info->code == TVN_MULTISELCHANGED && info->hwndFrom == thisPtr->m_tvTrigger) {
			NMTVMULTI* minfo = (NMTVMULTI*)info;
			if(minfo->selectBoth) {
				//check if old and new item can be selected together
				HTREEITEM p1 = TreeView_GetParent(thisPtr->m_tvTrigger,minfo->oldItem);
				HTREEITEM p2 = TreeView_GetParent(thisPtr->m_tvTrigger,minfo->newItem);
				if(p1 == p2) {
					minfo->selectBoth = true;
					return TRUE;
				}
				minfo->selectBoth = false;
				return TRUE;
			}
			return TRUE;
		}
		break; }
	}
	return FALSE;
}

void UnlimitedDialog::TRDialog::RefreshTriggerList() {
	int currSel = SendMessage(this->m_lbTriggers,LB_GETCURSEL,0,0);

	SendMessage(this->m_lbTriggers,LB_RESETCONTENT,0,0);
	const auto& list = AAEdit::g_currChar.m_cardData.GetTriggers();
	for (size_t i = 0; i < list.size(); i++) {
		SendMessage(this->m_lbTriggers,LB_INSERTSTRING,i,(LPARAM)list[i].name.c_str());
	}
	const auto& modList = AAEdit::g_currChar.m_cardData.GetModules();
	for(auto& mod : modList) {
		std::wstring modprefix = mod.name + TEXT("::");
		for(auto& trg : mod.triggers) {
			std::wstring name = modprefix + trg.name;
			SendMessage(this->m_lbTriggers,LB_ADDSTRING,0,(LPARAM)name.c_str());
		}
	}
	if (list.size() == 0) {
		EnableWindow(m_tvTrigger,FALSE);
	}
	else {
		EnableWindow(m_tvTrigger,TRUE);
		SendMessage(this->m_lbTriggers,LB_SETCURSEL,currSel == LB_ERR ? 0 : currSel,0);
		if (currSel == LB_ERR) {
			SetCurrentTrigger(0);
		}
	}
}

void UnlimitedDialog::TRDialog::RefreshTriggerActions() {
	SetCurrentTrigger(m_currentTriggerIndex);
}

void UnlimitedDialog::TRDialog::Refresh() {
	RefreshTriggerList();
	RefreshTriggerActions();
}

void UnlimitedDialog::TRDialog::InitializeTriggers() {
	//add generic event/variables/actions nodes
	const TCHAR LabelEvents[]{ TEXT("Events") };
	const TCHAR LabelVariables[]{ TEXT("Variables") };
	const TCHAR LabelActions[]{ TEXT("Actions") };

	TreeView_DeleteAllItems(m_tvTrigger);

	//the 3 base items
	TVINSERTSTRUCT telem;
	telem.hParent = TVI_ROOT;
	telem.hInsertAfter = TVI_LAST;
	telem.item.mask = TVIF_TEXT | TVIF_PARAM;

	telem.item.pszText = (LPWSTR)LabelEvents;
	telem.item.lParam = 0;
	m_tiEvents = TreeView_InsertItem(m_tvTrigger,&telem);

	telem.item.pszText = (LPWSTR)LabelVariables;
	telem.item.lParam = 1;
	m_tiVariables = TreeView_InsertItem(m_tvTrigger,&telem);

	telem.item.pszText = (LPWSTR)LabelActions;
	telem.item.lParam = 2;
	m_tiActions = TreeView_InsertItem(m_tvTrigger,&telem);
}

/******************************/
/* Add Global Variable dialog */
/******************************/

INT_PTR CALLBACK UnlimitedDialog::TRDialog::AddGlobalVariableDialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam) {
	switch (msg) {
	case WM_INITDIALOG: {
		//initialize dialog members from the loaded dialog
		TRDialog* thisPtr = (TRDialog*)lparam;
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lparam); //register class to this hwnd

		HWND lb = GetDlgItem(hwndDlg,IDC_TR_GV_LBLIST);
		//put current global vars into lb
		auto& list = g_currChar.m_cardData.GetGlobalVariables();
		for(int i = 0; i < list.size(); i++) {
			auto& elem = list[i];
			std::wstring str = g_Types[elem.type].name + TEXT(" ") + elem.name;
			str += TEXT(" = ");
			ParameterisedExpression dummy(elem.type,elem.defaultValue);
			str += thisPtr->ExpressionToString(dummy);
			int ret = SendMessage(lb,LB_ADDSTRING,0,(LPARAM)str.c_str());

		}
		
		
		return TRUE;
		break; }
	case WM_COMMAND: {
		DWORD identifier = LOWORD(wparam);
		DWORD notification = HIWORD(wparam);
		HWND wnd = (HWND)lparam;
		TRDialog* thisPtr = (TRDialog*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		switch (identifier) {
		case IDOK:
			EndDialog(hwndDlg,1);
			break;
		case IDC_TR_GV_BTNADD:
			if(notification == BN_CLICKED) {
				loc_AddVariableData data;
				data.thisPtr = thisPtr;
				data.var.defaultValue.expression = NULL;
				data.onlyConstantInit = true;
				data.checkGlobalNameConflict = true;
				data.var.type = TYPE_INVALID;
				int result = DialogBoxParam(General::DllInst,MAKEINTRESOURCE(IDD_TRIGGERS_ADDVARIABLE),hwndDlg,&AddVariableDialogProc,(LPARAM)&data);
				if(result == 1) {
					auto& list = g_currChar.m_cardData.GetGlobalVariables();
					list.push_back(data.var);
					std::wstring str = g_Types[data.var.type].name + TEXT(" ") + data.var.name;
					str += TEXT(" = ");
					str += thisPtr->ExpressionToString(data.var.defaultValue);
					int ret = SendMessage(GetDlgItem(hwndDlg,IDC_TR_GV_LBLIST),LB_ADDSTRING,0,(LPARAM)str.c_str());
				}
				return TRUE;
			}
			break;
		case IDC_TR_GV_BTNEDIT:
			if (notification == BN_CLICKED) {

			}
			break;
		case IDC_TR_GV_BTNDELETE:
			if (notification == BN_CLICKED) {
				int sel = SendMessage(GetDlgItem(hwndDlg,IDC_TR_GV_LBLIST),LB_GETCURSEL,0,0);
				if(sel != LB_ERR) {
					auto& list = g_currChar.m_cardData.GetGlobalVariables();
					list.erase(list.begin() + sel);
					SendMessage(GetDlgItem(hwndDlg,IDC_TR_GV_LBLIST),LB_DELETESTRING,sel,0);
				}
				return TRUE;
			}
			break;
		}

		break; }
	}
	return FALSE;
}

/*********************************/
/* Add Action (and event) dialog */
/*********************************/


struct loc_AddData;
static void DrawName(HWND wnd,DRAWITEMSTRUCT* dis,loc_AddData* data);

/*
* The add dialog gets one of these structs,
* depending on what type of thing he is supposed to
* select.
*/
loc_AddData::loc_AddData() {
	allowChange = true;
}

void loc_AddData::AddExpression(ParameterisedExpression& exp,int paramN) {
	GetActualParameters()[paramN] = exp;
	valid[paramN] = true;
}
void loc_AddData::RemoveExpression(int paramN) {
	GetActualParameters()[paramN].expression = NULL;
	valid[paramN] = false;
}

std::wstring loc_AddData::ToString() {
	std::wstring retVal;
	for (int i = 0; i < parts.size(); i++) {
		std::wstring& part = parts[i];
		if (retVal.size() != 0) retVal.push_back(L' ');
		if (part == TEXT("%p")) {
			retVal += GetParamString(i,NULL);
		}
		else {				
			retVal += part;
		}
	}
	return retVal;
}

//opens a window to add a parameter. returns true if the parameter was added successfully
bool loc_AddData::PromptAddParameter(HWND parentDialog, int i, Types retType) {
	//generic expression
	loc_AddExpressionData edata;
	edata.thisPtr = this->thisPtr;
	const auto& params = this->GetParameters();
	if(retType == TYPE_INVALID) {
		edata.retType = params[i];
	}
	else {
		edata.retType = retType;
	}
		

	//check if this expression allready is valid to initialize the window accordingly
	if (this->valid[i]) {
		edata.expression = this->GetActualParameters()[i];
		edata.valid.resize(edata.expression.actualParameters.size());
		for (int i = 0; i < edata.valid.size(); i++) edata.valid[i] = true;
	}
		
	int result = DialogBoxParam(General::DllInst,MAKEINTRESOURCE(IDD_TRIGGERS_ADDACTION),parentDialog,&UnlimitedDialog::TRDialog::AddActionDialogProc,(LPARAM)&edata);
	if (result == 1) {
		this->AddExpression(edata.expression,i);
		return true;
	}
	else {
		return false;
	}
}

void loc_AddData::SelectFromComboBox(HWND dlg,int index,const std::wstring* name,const std::wstring* descr) {
	//Regular entry with name and description
	ShowWindow(GetDlgItem(dlg,IDC_TR_AA_CBVAR),FALSE);
	ShowWindow(GetDlgItem(dlg,IDC_TR_AA_EDCONSTANT),FALSE);
	ShowWindow(GetDlgItem(dlg,IDC_TR_AA_STDESCR),TRUE);
	ShowWindow(GetDlgItem(dlg,IDC_TR_AA_STNAME),TRUE);
	auto& oldActualParams = GetActualParameters();
	auto& newParams = GetParameters();
	//clear parameters that can not be reused
	for (int i = 0; i < min(oldActualParams.size(),newParams.size()); i++) {
		if (!valid[i] || newParams[i] != oldActualParams[i].expression->returnType) {
			oldActualParams[i].actualParameters.clear();
			oldActualParams[i].expression = NULL;
			valid[i] = false;
		}
	}
	oldActualParams.resize(newParams.size());
	valid.resize(newParams.size());
	parts.clear();

	if (descr != NULL) {
		HWND stDescr = GetDlgItem(dlg,IDC_TR_AA_STDESCR);
		SendMessage(stDescr,WM_SETTEXT,0,(LPARAM)descr->c_str());
	}

	if (name != NULL) {
		HWND stName = GetDlgItem(dlg,IDC_TR_AA_STNAME);
		//split string at whitespaces
		int last = 0,i = 1;
		for (i = 1; i < name->size(); i++) {
			if (isspace((*name)[i]) && i != last) {
				std::wstring part = name->substr(last,i-last);
				parts.push_back(std::move(part));
				last = i+1;
			}
		}
		std::wstring part = name->substr(last,i-last);
		parts.push_back(std::move(part));
		last = i+1;

		DrawName(stName,NULL,this);

		bool valid = true;
		for (bool v : this->valid) {
			if (!v) { valid = false; break; }
		}
		EnableWindow(GetDlgItem(dlg,IDC_TR_AA_BTNADD),valid);
	}
}

loc_SelectVariableData::loc_SelectVariableData() {
	type = loc_AddData::VARIABLE;
	retType = TYPE_INVALID;
}
std::vector<ParameterisedExpression>& loc_SelectVariableData::GetActualParameters() {
	return variable.actualParameters;
}
const std::vector<Types>& loc_SelectVariableData::GetParameters() {
	return variable.expression->parameters;
}
DWORD loc_SelectVariableData::GetId() {
	if (variable.expression == NULL) return 0;
	return variable.expression->id;
}
void loc_SelectVariableData::InitializeComboBox(HWND box) {
	for (int i = 0; i < thisPtr->m_currentTrigger->vars.size(); i++) {
		auto& var = thisPtr->m_currentTrigger->vars[i];
		int index = SendMessage(box,CB_ADDSTRING,0,(LPARAM)var.name.c_str());
		SendMessage(box,CB_SETITEMDATA,index,i);
	}
	//globals get a flag set
	auto& list = g_currChar.m_cardData.GetGlobalVariables();
	for(int i = 0; i < list.size(); i++) {
		auto& var = list[i];
		int index = SendMessage(box,CB_ADDSTRING,0,(LPARAM)var.name.c_str());
		SendMessage(box,CB_SETITEMDATA,index,i | GLOBAL_VAR_FLAG);
	}

	if (variable.expression != NULL) {
		int ret = SendMessage(box,CB_SELECTSTRING,-1,(LPARAM)variable.varName.c_str());
		if (ret != CB_ERR) {
			DrawName(box,NULL,this);
		}
	}
}
void loc_SelectVariableData::SelectFromComboBox(HWND dlg,int index) {
	ShowWindow(GetDlgItem(dlg,IDC_TR_AA_CBVAR),FALSE);
	ShowWindow(GetDlgItem(dlg,IDC_TR_AA_EDCONSTANT),FALSE);
	ShowWindow(GetDlgItem(dlg,IDC_TR_AA_STDESCR),FALSE);
	ShowWindow(GetDlgItem(dlg,IDC_TR_AA_STNAME),FALSE);

	Types varType;
	if (index & GLOBAL_VAR_FLAG) {
		//global variable
		index &= ~GLOBAL_VAR_FLAG;
		const GlobalVariable* var = &g_currChar.m_cardData.GetGlobalVariables()[index];
		variable.expression = Expression::FromId(var->type,EXPR_VAR);
		variable.varName = var->name;
		varType = var->type;
	}
	else {
		//local variable
		const Variable* var; var = &thisPtr->m_currentTrigger->vars[index];
		variable.expression = Expression::FromId(var->type,EXPR_VAR);
		variable.varName = var->name;
		varType = var->type;
	}
	
	valid.resize(2);
	valid[0] = true;
	//invalidate expression if type doesnt fit
	if (valid[1] && variable.actualParameters[1].expression->returnType != varType) {
		valid[1] = false;
	}

	EnableWindow(GetDlgItem(dlg,IDC_TR_AA_BTNADD),TRUE);

	
}
std::wstring loc_SelectVariableData::GetParamString(int paramN,bool* retValid) {
	if (retValid) *retValid = valid[paramN];
	return thisPtr->ExpressionToString(variable.actualParameters[paramN]);
}


loc_AddActionData::loc_AddActionData() {
	type = loc_AddData::ACTION;
}
std::vector<ParameterisedExpression>& loc_AddActionData::GetActualParameters() {
	return action.actualParameters;
}
const std::vector<Types>& loc_AddActionData::GetParameters() {
	return action.action->parameters;
}
DWORD loc_AddActionData::GetId() {
	if (action.action == NULL) return 0;
	return action.action->id;
}
void loc_AddActionData::InitializeComboBox(HWND box) {
	for (int i = 0; i < Shared::Triggers::g_Actions.size(); i++) {
		const auto& elem = Shared::Triggers::g_Actions[i];
		std::wstring entryName = g_ActionCategories[elem.category] + TEXT(" - ") + elem.name;
		int index = SendMessage(box,CB_ADDSTRING,0,(LPARAM)entryName.c_str());
		SendMessage(box,CB_SETITEMDATA,index,i);
	}
	if(action.action != NULL) {
		std::wstring entryName = g_ActionCategories[action.action->category] + TEXT(" - ") + action.action->name;
		int ret = SendMessage(box,CB_SELECTSTRING,-1,(LPARAM)entryName.c_str());
		if(ret != CB_ERR) {
			DrawName(box,NULL,this);
		}
	}
}
void loc_AddActionData::SelectFromComboBox(HWND dlg,int index) {
	const std::wstring* name = NULL,*descr = NULL;
	const auto& elem = g_Actions[index];
	name = &elem.interactiveName;
	descr = &elem.description;
	action.action = &elem;

	if(GetId() == ACTION_SETVAR) {
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_CBVAR),FALSE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_EDCONSTANT),FALSE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_STDESCR),TRUE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_STNAME),TRUE);
		auto& oldActualParams = GetActualParameters();
		
		//check for variable parameters
		Types type = TYPE_INVALID;
		if(oldActualParams.size() >= 1) {
			if(valid[0] && oldActualParams[0].expression->id == EXPR_VAR) {
				type = oldActualParams[0].expression->returnType;
			}
			else {
				valid[0] = false;
				oldActualParams[0].expression = NULL;
			}
			
		}
		if(oldActualParams.size() >= 2 && type != TYPE_INVALID) {
			//second parameter must be expression of variable type
			if(!valid[1] || oldActualParams[1].expression->returnType != type) {
				valid[1] = false;
				oldActualParams[1].expression = NULL;
			}
		}
		

		oldActualParams.resize(2);
		valid.resize(2);
		parts.clear();

		if (descr != NULL) {
			HWND stDescr = GetDlgItem(dlg,IDC_TR_AA_STDESCR);
			SendMessage(stDescr,WM_SETTEXT,0,(LPARAM)descr->c_str());
		}

		if (name != NULL) {
			HWND stName = GetDlgItem(dlg,IDC_TR_AA_STNAME);
			//split string at whitespaces
			int last = 0,i = 1;
			for (i = 1; i < name->size(); i++) {
				if (isspace((*name)[i]) && i != last) {
					std::wstring part = name->substr(last,i-last);
					parts.push_back(std::move(part));
					last = i+1;
				}
			}
			std::wstring part = name->substr(last,i-last);
			parts.push_back(std::move(part));
			last = i+1;

			DrawName(stName,NULL,this);

			bool valid = true;
			for (bool v : this->valid) {
				if (!v) { valid = false; break; }
			}
			EnableWindow(GetDlgItem(dlg,IDC_TR_AA_BTNADD),valid);
		}
	}
	else {
		loc_AddData::SelectFromComboBox(dlg,index,name,descr);
	}
}
std::wstring loc_AddActionData::GetParamString(int paramN, bool* retValid) {
	if (retValid) *retValid = valid[paramN];
	return thisPtr->ExpressionToString(action.actualParameters[paramN]);
}

//considers special actions that require variables instead
bool loc_AddActionData::PromptAddParameter(HWND parentDialog,int i, Types retType) {
	//for loop 0 and set var 0 are variables
	if(action.action->id == ACTION_SETVAR && i == 0 || action.action->id == ACTION_FORLOOP && i == 0) {
		loc_SelectVariableData vdata;
		vdata.thisPtr = this->thisPtr;
		if (action.action->id == ACTION_FORLOOP) vdata.retType = TYPE_INT;
		int result = DialogBoxParam(General::DllInst,MAKEINTRESOURCE(IDD_TRIGGERS_ADDACTION),parentDialog,&UnlimitedDialog::TRDialog::AddActionDialogProc,(LPARAM)&vdata);
		if (result == 1) {
			this->action.actualParameters[0] = vdata.variable;
			this->valid[0] = true;
			return true;
		}
	}
	else if(action.action->id == ACTION_SETVAR && i == 1) {
		if (!valid[0]) return false;
		return loc_AddData::PromptAddParameter(parentDialog,i,this->GetActualParameters()[0].expression->returnType);
	}

	//normal expression
	return loc_AddData::PromptAddParameter(parentDialog,i, retType);
}

loc_AddEventData::loc_AddEventData() {
	type = loc_AddData::EVENT;
}
std::vector<ParameterisedExpression>& loc_AddEventData::GetActualParameters() {
	return event.actualParameters;
}
const std::vector<Types>& loc_AddEventData::GetParameters() {
	return event.event->parameters;
}
DWORD loc_AddEventData::GetId() {
	if (event.event == NULL) return 0;
	return event.event->id;
}
void loc_AddEventData::InitializeComboBox(HWND box) {
	for (int i = 0; i < Shared::Triggers::g_Events.size(); i++) {
		const auto& elem = Shared::Triggers::g_Events[i];
		std::wstring entryName = g_EventCategories[elem.category] + TEXT(" - ") + elem.name;
		int index = SendMessage(box,CB_ADDSTRING,0,(LPARAM)entryName.c_str());
		SendMessage(box,CB_SETITEMDATA,index,i);
	}
	if (event.event != NULL) {
		int ret = SendMessage(box,CB_SELECTSTRING,-1,(LPARAM)event.event->name.c_str());
		if (ret != CB_ERR) {
			DrawName(box,NULL,this);
		}
	}
}
void loc_AddEventData::SelectFromComboBox(HWND dlg,int index) {
	const std::wstring* name = NULL,*descr = NULL;
	const auto& elem = g_Events[index];
	name = &elem.interactiveName;
	descr = &elem.description;
	event.event = &elem;

	loc_AddData::SelectFromComboBox(dlg,index,name,descr);
}
std::wstring loc_AddEventData::GetParamString(int paramN,bool* retValid) {
	if (retValid) *retValid = valid[paramN];
	return thisPtr->ExpressionToString(event.actualParameters[paramN]);
}


loc_AddExpressionData::loc_AddExpressionData() {
	type = loc_AddData::EXPRESSION;
}
std::vector<ParameterisedExpression>& loc_AddExpressionData::GetActualParameters() {
	return expression.actualParameters;
}
const std::vector<Types>& loc_AddExpressionData::GetParameters() {
	return expression.expression->parameters;
}
DWORD loc_AddExpressionData::GetId() {
	if (expression.expression == NULL) return 0;
	return expression.expression->id;
}
void loc_AddExpressionData::InitializeComboBox(HWND box) {
	for (int i = 0; i < Shared::Triggers::g_Expressions[retType].size(); i++) {
		const auto& elem = Shared::Triggers::g_Expressions[retType][i];
		std::wstring entryName = g_ExpressionCategories[elem.category] + TEXT(" - ") + elem.name;
		int index = SendMessage(box,CB_ADDSTRING,0,(LPARAM)entryName.c_str());
		SendMessage(box,CB_SETITEMDATA,index,i);
	}
	if (expression.expression != NULL) {
		std::wstring entryName = g_ExpressionCategories[expression.expression->category] + TEXT(" - ") + expression.expression->name;
		int ret = SendMessage(box,CB_SELECTSTRING,-1, (LPARAM)entryName.c_str());
		if (ret != CB_ERR) {
			DrawName(box,NULL,this);
		}
	}
}
void loc_AddExpressionData::SelectFromComboBox(HWND dlg,int index) {
	const std::wstring* name = NULL,*descr = NULL;
	const auto& elem = g_Expressions[retType][index];
	name = &elem.interactiveName;
	descr = &elem.description;
	expression.expression = &elem;

	if (GetId() == EXPR_CONSTANT) {
		//constant
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_CBVAR),FALSE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_EDCONSTANT),TRUE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_STDESCR),FALSE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_STNAME),FALSE);
		EnableWindow(GetDlgItem(dlg,IDC_TR_AA_BTNADD),false);

		//insert constant from data if applicable
		HWND cb = GetDlgItem(dlg,IDC_TR_AA_EDCONSTANT);
		switch (expression.constant.type) {
		case TYPE_INT:
			SendMessage(cb,WM_SETTEXT,0,(LPARAM)std::to_wstring(expression.constant.iVal).c_str());
			break;
		case TYPE_FLOAT:
			SendMessage(cb,WM_SETTEXT,0,(LPARAM)std::to_wstring(expression.constant.fVal).c_str());
			break;
		case TYPE_BOOL:
			SendMessage(cb,WM_SETTEXT,0,(LPARAM)std::to_wstring(expression.constant.bVal).c_str());
			break;
		case TYPE_STRING:
			SendMessage(cb,WM_SETTEXT,0,(LPARAM)expression.constant.strVal);
			break;
		default:
			break;
		}
	}
	else if (GetId() == EXPR_VAR) {
		//variable
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_CBVAR),TRUE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_EDCONSTANT),FALSE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_STDESCR),FALSE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_STNAME),FALSE);
		EnableWindow(GetDlgItem(dlg,IDC_TR_AA_BTNADD),FALSE);

		HWND cb = GetDlgItem(dlg,IDC_TR_AA_CBVAR);
		SendMessage(cb,CB_RESETCONTENT,0,0);
		Types type = expression.expression->returnType;
		for (auto& elem : thisPtr->m_currentTrigger->vars) {
			if (elem.type == type) {
				SendMessage(cb,CB_ADDSTRING,0,(LPARAM)elem.name.c_str());
			}
		}
		//add globals as well
		auto& list = g_currChar.m_cardData.GetGlobalVariables();
		for(auto& elem : list) {
			if (elem.type == type) {
				SendMessage(cb,CB_ADDSTRING,0,(LPARAM)elem.name.c_str());
			}
		}
		//try to use old name if possible
		Variable* oldVar = thisPtr->m_currentTrigger->FindVar(expression.varName);
		if (oldVar) {
			int ret = SendMessage(cb,CB_SELECTSTRING,-1,(LPARAM)oldVar->name.c_str());
		}

	}
	else if (GetId() == EXPR_NAMEDCONSTANT) {
		//named constant
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_CBVAR),TRUE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_EDCONSTANT),FALSE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_STDESCR),FALSE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_STNAME),FALSE);
		EnableWindow(GetDlgItem(dlg,IDC_TR_AA_BTNADD),FALSE);


		HWND cb = GetDlgItem(dlg,IDC_TR_AA_CBVAR);
		SendMessage(cb,CB_RESETCONTENT,0,0);
		Types type = expression.expression->returnType;
		for (auto& elem : g_NamedConstants[type]) {
			std::wstring entryName;
			entryName += g_NamedConstantCategories[elem.category] + TEXT(" - ");
			entryName += elem.name;
			int i = SendMessage(cb,CB_ADDSTRING,0,(LPARAM)entryName.c_str());
			SendMessage(cb,CB_SETITEMDATA,i,elem.id);
		}
		//try to use old name if possible
		const NamedConstant* oldConstant = expression.namedConstant;
		if (oldConstant != NULL) {
			int ret = SendMessage(cb,CB_SELECTSTRING,-1,(LPARAM)oldConstant->name.c_str());
		}
	}
	else {
		loc_AddData::SelectFromComboBox(dlg,index,name,descr);
	}
}
std::wstring loc_AddExpressionData::GetParamString(int paramN,bool* retValid) {
	if (retValid) *retValid = valid[paramN];
	return thisPtr->ExpressionToString(expression.actualParameters[paramN]);
}


void UnlimitedDialog::TRDialog::DoAddAction() {
	loc_AddActionData data;
	data.thisPtr = this;
	int result = DialogBoxParam(General::DllInst,MAKEINTRESOURCE(IDD_TRIGGERS_ADDACTION),m_dialog,&AddActionDialogProc,(LPARAM)&data);
	if(result == 1) {
		if(m_currentTrigger) {
			AddTriggerAction(data.action);
		}
	}
	
}

void UnlimitedDialog::TRDialog::DoAddEvent() {
	loc_AddEventData data;
	data.thisPtr = this;
	int result = DialogBoxParam(General::DllInst,MAKEINTRESOURCE(IDD_TRIGGERS_ADDACTION),m_dialog,&AddActionDialogProc,(LPARAM)&data);
	if (result == 1) {
		if (m_currentTrigger) {
			AddTriggerEvent(data.event,GetSelectedEvent());
		}
	}

}

bool IsInputValid(loc_AddData* data) {
	for (bool v : data->valid) {
		if (!v) return false;
	}
	return true;
}

static void DrawName(HWND wnd,DRAWITEMSTRUCT* dis,loc_AddData* data) {
	data->clickRects.clear();

	HDC dc;
	if (dis) dc = dis->hDC;
	else  	 dc = GetDC(wnd);
	HFONT origFont = (HFONT)SendMessage(wnd,WM_GETFONT,0,0);
	HFONT underlined;
	HFONT old = (HFONT)SelectObject(dc,origFont);
	LOGFONT fontdata;
	GetObject(origFont,sizeof(fontdata),&fontdata);
	fontdata.lfUnderline = TRUE;
	fontdata.lfItalic = TRUE;
	underlined = CreateFontIndirect(&fontdata);
	SIZE size;
	GetTextExtentPoint32(dc,TEXT(" "),1,&size);
	int spacewidth = size.cx;
	COLORREF defColor = GetSysColor(COLOR_WINDOWTEXT);
	COLORREF background = GetSysColor(COLOR_MENU);
	COLORREF blueColor = RGB(0,0,255);
	COLORREF redColor = RGB(255,0,0);
	SetBkColor(dc,background);

	RECT textRect;
	GetClientRect(wnd,&textRect);

	//clear previous text
	FillRect(dc,&textRect,(HBRUSH)(COLOR_MENU+1));

	int x = 0,y = 0,yheight = 0;
	int maxx = textRect.right - textRect.left;
	int maxy = textRect.bottom - textRect.top;
	int paramCount = 0;

	//draw every word
	for (auto& elem : data->parts) {
		bool savePos = false;
		std::wstring drawstring;
		if (elem == TEXT("%p")) {
			//replace with underlined expression representation
			bool valid;
			std::wstring rep = data->GetParamString(paramCount++, &valid);
			drawstring = rep;

			savePos = true;
			SelectObject(dc,underlined);
			SetTextColor(dc,valid ? blueColor : redColor);
		}
		else {
			drawstring = elem;
			SelectObject(dc,origFont);
			SetTextColor(dc,defColor);
		}
		SIZE size;
		GetTextExtentPoint32(dc,drawstring.c_str(),drawstring.size(),&size);
		if (yheight < size.cy) yheight = size.cy;
		if (x + size.cx > maxx) {
			x = 0;
			y += size.cy + yheight;
			yheight = 0;
		}
		if (savePos) {
			RECT rct{ x,y, x+size.cx, y+size.cy };
			data->clickRects.push_back(rct);
		}
		TextOut(dc,x,y,drawstring.c_str(),drawstring.size());
		x += size.cx + spacewidth;
	}
	SelectObject(dc,old);
	DeleteObject(underlined);
	if (!dis) ReleaseDC(wnd,dc);
}

INT_PTR CALLBACK UnlimitedDialog::TRDialog::AddActionDialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam) {
	switch (msg) {
	case WM_INITDIALOG: {
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,(LONG)lparam);
		EnableWindow(GetDlgItem(hwndDlg,IDC_TR_AA_BTNADD),FALSE);
		HWND cb = GetDlgItem(hwndDlg,IDC_TR_AA_CBSELECTION);
		loc_AddData* data = (loc_AddData*)lparam;
		data->InitializeComboBox(cb);
		int sel = SendMessage(cb,CB_GETCURSEL,0,0);
		if(sel != CB_ERR) {
			int index = SendMessage(cb,CB_GETITEMDATA,sel,0);
			data->SelectFromComboBox(hwndDlg,index);
		}
		if(!data->allowChange) {
			EnableWindow(GetDlgItem(hwndDlg,IDC_TR_AA_CBSELECTION),FALSE);
		}
		if(sel == CB_ERR) {
			//SendMessage(GetDlgItem(hwndDlg,IDC_TR_AA_CBSELECTION),CB_SHOWDROPDOWN,TRUE,0);
		}
		return TRUE;
		break; }
	case WM_DRAWITEM: {
		DWORD identifier = LOWORD(wparam);
		if (identifier == IDC_TR_AA_STNAME) {
			DRAWITEMSTRUCT* draw = (DRAWITEMSTRUCT*)(lparam);
			loc_AddData* data = (loc_AddData*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
			DrawName(draw->hwndItem,draw,data);
			return TRUE;
		}
		break; }
	case WM_COMMAND: {
		DWORD identifier = LOWORD(wparam);
		DWORD notification = HIWORD(wparam);
		HWND wnd = (HWND)lparam;
		switch (identifier) {
		case IDOK:
			//enter was pressed
			if(IsWindowEnabled(GetDlgItem(hwndDlg, IDC_TR_AA_BTNADD))) {
				EndDialog(hwndDlg,1);
				return TRUE;
			}
			break;
		case IDC_TR_AA_BTNADD:
			if (notification == BN_CLICKED) {
				EndDialog(hwndDlg,1);
				return TRUE;
			}
			break;
		case IDC_TR_AA_BTNCANCEL:
			if (notification == BN_CLICKED) {
				EndDialog(hwndDlg,0);
				return TRUE;
			}
			break;
		case IDC_TR_AA_STNAME:
			if (notification == STN_CLICKED) {
				loc_AddData* data = (loc_AddData*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
				POINT mouse;
				GetCursorPos(&mouse);
				RECT stRect;
				GetWindowRect(wnd,&stRect);
				for (int i = 0; i < data->clickRects.size(); i++) {
					auto& elem = data->clickRects[i];
					RECT totalRect = elem;
					OffsetRect(&totalRect,stRect.left,stRect.top);
					if (PtInRect(&totalRect,mouse)) {
						//clicked an expression
						bool succ = data->PromptAddParameter(hwndDlg,i);
						if (succ) {
							bool valid = true;
							for (bool v : data->valid) {
								if (!v) { valid = false; break; }
							}
							EnableWindow(GetDlgItem(hwndDlg,IDC_TR_AA_BTNADD),valid);
							DrawName(GetDlgItem(hwndDlg,IDC_TR_AA_STNAME),NULL,data);
						}
						break;
					}
				}
			}
			break;
		case IDC_TR_AA_CBSELECTION:
			if (notification == CBN_SELCHANGE) {
				int index = SendMessage(wnd,CB_GETCURSEL,0,0);
				if (index != CB_ERR) {
					loc_AddData* data = (loc_AddData*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
					index = SendMessage(wnd,CB_GETITEMDATA,index,0);
					data->SelectFromComboBox(hwndDlg, index);

					return TRUE;
				}
			}
			break;
		case IDC_TR_AA_CBVAR:
			if (notification == CBN_SELCHANGE) {
				int index = SendMessage(wnd,CB_GETCURSEL,0,0);
				if (index != CB_ERR) {
					loc_AddExpressionData* data = (loc_AddExpressionData*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
					int sel = SendMessage(wnd,CB_GETCURSEL,0,0);
					if(sel != CB_ERR) {
						if(data->expression.expression->id == EXPR_VAR) {
							std::wstring string = General::GetComboBoxString(wnd,sel);
							data->expression.varName = string;
							EnableWindow(GetDlgItem(hwndDlg,IDC_TR_AA_BTNADD),TRUE);
						}
						else if(data->expression.expression->id == EXPR_NAMEDCONSTANT) {
							int id = SendMessage(wnd,CB_GETITEMDATA,sel,0);
							data->expression.namedConstant = NamedConstant::FromId(data->expression.expression->returnType,id);
							EnableWindow(GetDlgItem(hwndDlg,IDC_TR_AA_BTNADD),TRUE);
						}
						
					}
					else {
						EnableWindow(GetDlgItem(hwndDlg,IDC_TR_AA_BTNADD),FALSE);
					}
					return TRUE;
				}
			}
			break;
		case IDC_TR_AA_EDCONSTANT:
			if(notification == EN_CHANGE) {
				loc_AddExpressionData* data = (loc_AddExpressionData*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
				std::wstring text = General::GetEditString(wnd);
				wchar_t* end;
				
				Value val;
				val.type = data->retType;
				bool success = true;
				switch(data->retType) {
				case TYPE_INT:
					val.iVal = wcstol(text.c_str(),&end,10);
					if(end != text.c_str()+text.size()) {
						success = false;
					}
					break;
				case TYPE_FLOAT:
					val.fVal = wcstod(text.c_str(),&end);
					if(end != text.c_str()+text.size()) {
						success = false;
					}
					break;
				case TYPE_BOOL:
					if(text == TEXT("true") || text == TEXT("1")) {
						val.bVal = true;
					}
					else if(text == TEXT("false") || text == TEXT("0")) {
						val.bVal = false;
					}
					else {
						success = false;
					}
					break;
				case TYPE_STRING:
					val.strVal = new std::wstring(text);
					break;
				default:
					break;
				}

				if(success) {
					data->expression.constant = std::move(val);
				}
				EnableWindow(GetDlgItem(hwndDlg,IDC_TR_AA_BTNADD),success);
			}
		}
		break; }
	}
	return FALSE;
}


/***********************/
/* Add Variable Dialog */
/***********************/

void UnlimitedDialog::TRDialog::DoAddVariable() {
	loc_AddVariableData data;
	data.thisPtr = this;
	data.checkLocalNameConflict = true;
	data.var.defaultValue.expression = NULL;
	data.var.type = TYPE_INVALID;
	int result = DialogBoxParam(General::DllInst,MAKEINTRESOURCE(IDD_TRIGGERS_ADDVARIABLE),m_dialog,&AddVariableDialogProc,(LPARAM)&data);
	if (result == 1) {
		if(m_currentTrigger != NULL) {
			AddTriggerVariable(data.var,GetSelectedVariable());
		}
	}

}

INT_PTR CALLBACK UnlimitedDialog::TRDialog::AddVariableDialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam) {
	switch (msg) {
	case WM_INITDIALOG: {
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,(LONG)lparam);
		EnableWindow(GetDlgItem(hwndDlg,IDC_TR_AV_BTNADD),FALSE);
		HWND cb = GetDlgItem(hwndDlg,IDC_TR_AV_CBTYPE);
		//add all types
		for(int i = 0; i < ARRAYSIZE(g_Types); i++) {
			auto& elem = g_Types[i];
			SendMessage(cb,CB_ADDSTRING,0,(LPARAM)elem.name.c_str());
		}
		
		return TRUE;
		break; }
	case WM_COMMAND: {
		DWORD identifier = LOWORD(wparam);
		DWORD notification = HIWORD(wparam);
		HWND wnd = (HWND)lparam;
		switch (identifier) {
		case IDOK:
			//enter was pressed
			if (IsWindowEnabled(GetDlgItem(hwndDlg,IDC_TR_AV_BTNADD))) {
				EndDialog(hwndDlg,1);
				return TRUE;
			}
			break;
		case IDC_TR_AV_BTNADD:
			if (notification == BN_CLICKED) {
				EndDialog(hwndDlg,1);
				return TRUE;
			}
			break;
		case IDC_TR_AV_BTNCANCEL:
			if (notification == BN_CLICKED) {
				EndDialog(hwndDlg,0);
				return TRUE;
			}
			break;
		case IDC_TR_AV_CBTYPE:
			if (notification == CBN_SELCHANGE) {
				int index = SendMessage(wnd,CB_GETCURSEL,0,0);
				if (index != CB_ERR) {
					loc_AddVariableData* data = (loc_AddVariableData*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
					if(index >= 0 && index < Types::N_TYPES) {
						data->var.type = (Types)index;
					}
					data->var.defaultValue.expression = NULL;
					return TRUE;
				}
			}
			break;
		case IDC_TR_AV_EDNAME:
			if (notification == EN_CHANGE) {
				loc_AddVariableData* data = (loc_AddVariableData*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
				std::wstring name = General::GetEditString(wnd);
				data->var.name = name;
				bool valid = false;

				if(data->var.name.size() > 0  && data->var.defaultValue.expression != NULL) {
					valid = true;
				}
				if(data->checkLocalNameConflict) {
					if(data->thisPtr->m_currentTrigger->FindVar(name) != NULL) {
						valid = false;
					}
				}
				if(data->checkGlobalNameConflict) {
					auto& globals = g_currChar.m_cardData.GetGlobalVariables();
					for(auto& var : globals) {
						if(var.name == data->var.name) {
							valid = false;
							break;
						}
					}
				}
				if(valid) {
					EnableWindow(GetDlgItem(hwndDlg,IDC_TR_AV_BTNADD),TRUE);
				}
				else {
					EnableWindow(GetDlgItem(hwndDlg,IDC_TR_AV_BTNADD),FALSE);
				}
				
				return TRUE;
			}
			break;
		case IDC_TR_AV_STINITVAL:
			if (notification == STN_CLICKED) {
				loc_AddVariableData* data = (loc_AddVariableData*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
				if(data->var.type != TYPE_INVALID) {
					loc_AddExpressionData edata;
					edata.retType = data->var.type;
					edata.thisPtr = data->thisPtr;
					if(data->onlyConstantInit) {
						edata.allowChange = false;
						edata.expression = ParameterisedExpression(data->var.type,Value(data->var.type));
					}

					//check if this expression allready is valid to initialize the window accordingly
					if (data->var.defaultValue.expression != NULL) {
						edata.expression = data->var.defaultValue;
						edata.valid.resize(edata.expression.actualParameters.size());
						for (int i = 0; i < edata.valid.size(); i++) edata.valid[i] = true;

					}
					int result = DialogBoxParam(General::DllInst,MAKEINTRESOURCE(IDD_TRIGGERS_ADDACTION),hwndDlg,&AddActionDialogProc,(LPARAM)&edata);
					if(result == 1) {
						data->var.defaultValue = edata.expression;
						auto& existingGlobals = g_currChar.m_cardData.GetGlobalVariables();
						bool found = false;
						for(auto& var : existingGlobals) {
							if(var.name == data->var.name) {
								found = true;
								break;
							}
						}
						if (data->var.name.size() > 0 && !found) {
							EnableWindow(GetDlgItem(hwndDlg,IDC_TR_AV_BTNADD),TRUE);
						}
						std::wstring valueName = data->thisPtr->ExpressionToString(data->var.defaultValue);
						SendMessage(GetDlgItem(hwndDlg,IDC_TR_AV_STINITVAL),WM_SETTEXT,0,(LPARAM)valueName.c_str());
					}
				}
				

			}
			break;
		}
		break; }
	}
	return FALSE;
}



INT_PTR CALLBACK ExportModuleDialogProc(_In_ HWND hwndDlg,_In_ UINT msg,_In_ WPARAM wparam,_In_ LPARAM lparam) {
	switch (msg) {
	case WM_INITDIALOG: {
		//initialize dialog members from the loaded dialog
		loc_ModuleInfo* data = (loc_ModuleInfo*)lparam;
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lparam); //register class to this hwnd
		SendMessage(GetDlgItem(hwndDlg,IDC_TR_EX_EDNAME),WM_SETTEXT,0,(LPARAM)data->name.c_str());
		SendMessage(GetDlgItem(hwndDlg,IDC_TR_EX_EDDESCR),WM_SETTEXT,0,(LPARAM)data->description.c_str());
		for(std::wstring& str : data->saveTriggers) {
			SendMessage(GetDlgItem(hwndDlg,IDC_TR_EX_LBTRIGLIST),LB_ADDSTRING,0,(LPARAM)str.c_str());
		}
		for(std::wstring& str : data->dependencies) {
			SendMessage(GetDlgItem(hwndDlg,IDC_TR_EX_LBDEPEND),LB_ADDSTRING,0,(LPARAM)str.c_str());
		}
		for(auto* var : data->saveGlobals) {
			std::wstring str = g_Types[var->type].name + TEXT(" ") + var->name;
			str += TEXT(" = ");
			ParameterisedExpression dummy(var->type,var->defaultValue);
			str += data->thisPtr->ExpressionToString(dummy);
		}

		return TRUE;
		break; }
	case WM_COMMAND: {
		loc_ModuleInfo* data = (loc_ModuleInfo*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		DWORD identifier = LOWORD(wparam);
		DWORD notification = HIWORD(wparam);
		HWND wnd = (HWND)lparam;
		switch (identifier) {
		case IDOK:
			if (notification == BN_CLICKED) {
				EndDialog(hwndDlg,1);
				return TRUE;
			}
			break;
		case IDCANCEL:
			if (notification == BN_CLICKED) {
				EndDialog(hwndDlg,0);
				return TRUE;
			}
			break;
		case IDC_TR_EX_EDNAME: {
			if(notification == EN_CHANGE) {
				data->name = General::GetEditString(wnd);
			}
			break; }
		case IDC_TR_EX_EDDESCR: {
			if (notification == EN_CHANGE) {
				data->description = General::GetEditString(wnd);
			}
			break; }
		}
		break; }
	}
	return FALSE;
}


/********************/
/* Multiselect tree */
/********************/

LRESULT CALLBACK TreeViewMultiselect(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam,UINT_PTR uIdSubclass,DWORD_PTR dwRefData) {
	switch(msg) {
	case WM_DESTROY: {
		TreeViewMultiselectData* data = (TreeViewMultiselectData*)dwRefData;
		delete data;
		break; }
	case WM_PAINT: {
		TreeViewMultiselectData* data = (TreeViewMultiselectData*)dwRefData;
		if (data->supressPaint) return TRUE;
		break; }
	case TVM_GETMULTISEL:
		return (LRESULT)&((TreeViewMultiselectData*)dwRefData)->selection;
		break;
	case TVM_DELETEITEM: {
		HTREEITEM item = (HTREEITEM)lparam;
		TreeViewMultiselectData* data = (TreeViewMultiselectData*)dwRefData;
		for (int i = 0; i < data->selection.size(); i++) {
			HTREEITEM sel = data->selection[i];
			if(sel == item) {
				data->selection.erase(data->selection.begin() + i);
				break;
			}
		}
		break; }
	case WM_LBUTTONDOWN: {
		TreeViewMultiselectData* data = (TreeViewMultiselectData*)dwRefData;
		data->supressPaint = true;
		HTREEITEM item1 = (HTREEITEM)DefSubclassProc(hwnd,TVM_GETNEXTITEM,TVGN_CARET,NULL);
		LRESULT origRes = DefSubclassProc(hwnd,msg,wparam,lparam);
		HTREEITEM item2 = (HTREEITEM)DefSubclassProc(hwnd,TVM_GETNEXTITEM,TVGN_CARET,NULL);
		if(item1 != item2) {
			NMTVMULTI nmh;
			nmh.hdr.code = TVN_MULTISELCHANGED;
			nmh.hdr.idFrom = GetDlgCtrlID(hwnd);
			nmh.hdr.hwndFrom = hwnd;
			nmh.oldItem = item1;
			nmh.newItem = item2;
			nmh.selectBoth = (wparam & MK_CONTROL) != 0;
			SendMessage(GetParent(hwnd),
				WM_NOTIFY,
				(WPARAM)hwnd,
				(LPARAM)&nmh);
			if (nmh.selectBoth && (wparam & MK_CONTROL)) {
				TVITEM item;
				item.mask = TVIF_STATE | TVIF_HANDLE;
				item.hItem = item1;
				item.stateMask = 0xF;
				BOOL succ = DefSubclassProc(hwnd,TVM_GETITEM,0,(LPARAM)&item);
				item.state |= TVIS_SELECTED;
				succ = DefSubclassProc(hwnd,TVM_SETITEM,0,(LPARAM)&item);
			}
			else {
				for(auto hItem : data->selection) {
					TVITEM item;
					item.mask = TVIF_STATE | TVIF_HANDLE;
					item.hItem = hItem;
					item.stateMask = 0xF;
					BOOL succ = DefSubclassProc(hwnd,TVM_GETITEM,0,(LPARAM)&item);
					item.state &= ~TVIS_SELECTED;
					succ = DefSubclassProc(hwnd,TVM_SETITEM,0,(LPARAM)&item);
				}
				data->selection.clear();
			}
			data->selection.push_back(item2);
		}
		data->supressPaint = false;
		return origRes;
		break; }
	default:
		break;
	}
	return DefSubclassProc(hwnd,msg,wparam,lparam);
}


}
