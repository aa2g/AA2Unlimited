#include "Triggers.h"

#include <Windows.h>
#include <CommCtrl.h>
#include <queue>

#include "resource.h"
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
	else if (param.expression->id == EXPR_VAR) {
		//variable
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
		auto& queueElem = guiActionQueue.back();
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
UnlimitedDialog::TRDialog::SelectedAction_Data UnlimitedDialog::TRDialog::GetSelectedAction() {
	HTREEITEM tvSel = TreeView_GetSelection(m_tvTrigger);
	//find selected action, if action is selected
	struct LoopData {
		std::vector<Trigger::GUIAction*>& cardActions;
		std::vector<ActionTreeItem*>& guiActions;
	};

	SelectedAction_Data retVal;

	std::queue<LoopData> actionQueue;
	actionQueue.push({ m_currentTrigger->guiActions, m_actions });
	while (!actionQueue.empty()) {
		auto& elem = actionQueue.back();
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
			else if(elem.guiActions[i]->subActionLabel == tvSel) {
				retVal.cardActions = &elem.cardActions;
				retVal.guiActions = &elem.guiActions;
				retVal.cardActionsInt = i;
				retVal.isSubLabel = true;
				break;
			}
		}
	}
	return retVal;
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

void UnlimitedDialog::TRDialog::AddTriggerAction(const ParameterisedAction& action) {
	if (m_currentTrigger == NULL) return;
	
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

				BOOL ret = TrackPopupMenu(subMenu,TPM_LEFTALIGN | TPM_RETURNCMD,
					cursor.x,cursor.y,0,thisPtr->m_dialog,NULL);
				switch (ret) {
				case 0: //no selection
					break;
				case ID_TRM_ADDTRIGGER: {
					Trigger trg;
					trg.name = TEXT("new trigger");
					AAEdit::g_currChar.m_cardData.GetTriggers().push_back(trg);
					int currSel = SendMessage(thisPtr->m_lbTriggers,LB_GETCURSEL,0,0);
					SendMessage(thisPtr->m_lbTriggers,LB_ADDSTRING,0,(LPARAM)trg.name.c_str());
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
						if (selData.isSubLabel) break;
						HTREEITEM item = (*selData.guiActions)[selData.cardActionsInt]->tree;
						SendMessage(thisPtr->m_tvTrigger,TVM_DELETEITEM,0,(LPARAM)item);
						delete (*selData.guiActions)[selData.cardActionsInt];
						delete (*selData.cardActions)[selData.cardActionsInt];
						selData.guiActions->erase(selData.guiActions->begin() + selData.cardActionsInt);
						selData.cardActions->erase(selData.cardActions->begin() + selData.cardActionsInt);
						return TRUE;
					}
				}
				break; }
			default:
				break;
			}
			DestroyMenu(menu);
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

/*********************************/
/* Add Action (and event) dialog */
/*********************************/


struct loc_AddData;
static void DrawName(HWND wnd,DRAWITEMSTRUCT* dis,loc_AddData* data);
static void SelectFromComboBox(HWND dlg,int index,loc_AddData* data);

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

	if (variable.expression != NULL) {
		int ret = SendMessage(box,CB_SELECTSTRING,-1,(LPARAM)variable.varName.c_str());
		if (ret != CB_ERR) {
			DrawName(box,NULL,this);
		}
	}
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
		int ret = SendMessage(box,CB_SELECTSTRING,-1,(LPARAM)action.action->name.c_str());
		if(ret != CB_ERR) {
			DrawName(box,NULL,this);
		}
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
		int ret = SendMessage(box,CB_SELECTSTRING,-1, (LPARAM)expression.expression->name.c_str());
		if (ret != CB_ERR) {
			DrawName(box,NULL,this);
		}
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

//TODO: split up into virtual functions
static void SelectFromComboBox(HWND dlg, int index, loc_AddData* data) {
	const std::wstring* name = NULL,*descr = NULL;

	//set to selected event/action/expression
	if (data->type == loc_AddData::EVENT) {
		loc_AddEventData* edata = (loc_AddEventData*)data;
		const auto& elem = g_Events[index];
		name = &elem.interactiveName;
		descr = &elem.description;
		edata->event.event = &elem;
	}
	else if (data->type == loc_AddData::EXPRESSION) {
		loc_AddExpressionData* edata = (loc_AddExpressionData*)data;
		const auto& elem = g_Expressions[edata->retType][index];
		name = &elem.interactiveName;
		descr = &elem.description;
		edata->expression.expression = &elem;
	}
	else if (data->type == loc_AddData::ACTION) {
		loc_AddActionData* adata = (loc_AddActionData*)data;
		const auto& elem = g_Actions[index];
		name = &elem.interactiveName;
		descr = &elem.description;
		adata->action.action = &elem;
	}
	else if(data->type == loc_AddData::VARIABLE) {
		loc_SelectVariableData* edata = (loc_SelectVariableData*)data;
		const Variable* var = &edata->thisPtr->m_currentTrigger->vars[index];
		edata->variable.expression = &g_Expressions[var->type][1];
	}
	if(data->type == loc_AddData::EXPRESSION && data->GetId() == EXPR_CONSTANT) {
		loc_AddExpressionData* edata = (loc_AddExpressionData*)data;
		//constant
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_CBVAR),FALSE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_EDCONSTANT),TRUE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_STDESCR),FALSE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_STNAME),FALSE);
		EnableWindow(GetDlgItem(dlg,IDC_TR_AA_BTNADD),false);

		//insert constant from data if applicable
		HWND cb = GetDlgItem(dlg,IDC_TR_AA_EDCONSTANT);
		switch(edata->expression.constant.type) {
		case TYPE_INT:
			SendMessage(cb,WM_SETTEXT,0,(LPARAM)std::to_wstring(edata->expression.constant.iVal).c_str());
			break;
		case TYPE_FLOAT:
			SendMessage(cb,WM_SETTEXT,0,(LPARAM)std::to_wstring(edata->expression.constant.fVal).c_str());
			break;
		case TYPE_BOOL:
			SendMessage(cb,WM_SETTEXT,0,(LPARAM)std::to_wstring(edata->expression.constant.bVal).c_str());
			break;
		case TYPE_STRING:
			SendMessage(cb,WM_SETTEXT,0,(LPARAM)edata->expression.constant.strVal);
			break;
		default:
			break;
		}
	}
	else if(data->type == loc_AddData::EXPRESSION && data->GetId() == EXPR_VAR) {
		loc_AddExpressionData* edata = (loc_AddExpressionData*)data;
		//variable
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_CBVAR),TRUE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_EDCONSTANT),FALSE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_STDESCR),FALSE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_STNAME),FALSE);
		EnableWindow(GetDlgItem(dlg,IDC_TR_AA_BTNADD),FALSE);

		
		HWND cb = GetDlgItem(dlg,IDC_TR_AA_CBVAR);
		SendMessage(cb,CB_RESETCONTENT,0,0);
		Types type = edata->expression.expression->returnType;
		for (auto& elem : data->thisPtr->m_currentTrigger->vars) {
			if(elem.type == type) {
				SendMessage(cb,CB_ADDSTRING,0,(LPARAM)elem.name.c_str());
			}
		}
		//try to use old name if possible
		Variable* oldVar = data->thisPtr->m_currentTrigger->FindVar(edata->expression.varName);
		if (oldVar) {
			int ret = SendMessage(cb,CB_SELECTSTRING,-1,(LPARAM)oldVar->name.c_str());
		}
		
	}
	else if (data->type == loc_AddData::EXPRESSION && data->GetId() == EXPR_NAMEDCONSTANT) {
		loc_AddExpressionData* edata = (loc_AddExpressionData*)data;
		//named constant
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_CBVAR),TRUE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_EDCONSTANT),FALSE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_STDESCR),FALSE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_STNAME),FALSE);
		EnableWindow(GetDlgItem(dlg,IDC_TR_AA_BTNADD),FALSE);


		HWND cb = GetDlgItem(dlg,IDC_TR_AA_CBVAR);
		SendMessage(cb,CB_RESETCONTENT,0,0);
		Types type = edata->expression.expression->returnType;
		for (auto& elem : g_NamedConstants[type]) {	
			std::wstring entryName;
			entryName += g_NamedConstantCategories[elem.category] + TEXT(" - ");
			entryName += elem.name;
			int i = SendMessage(cb,CB_ADDSTRING,0,(LPARAM)entryName.c_str());
			SendMessage(cb,CB_SETITEMDATA,i,elem.id);
		}
		//try to use old name if possible
		const NamedConstant* oldConstant = edata->expression.namedConstant;
		if (oldConstant != NULL) {
			int ret = SendMessage(cb,CB_SELECTSTRING,-1,(LPARAM)oldConstant->name.c_str());
		}
	}
	else if(data->type == loc_AddData::VARIABLE) {
		loc_SelectVariableData* edata = (loc_SelectVariableData*)data;
		//variable
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_CBVAR),FALSE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_EDCONSTANT),FALSE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_STDESCR),FALSE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_STNAME),FALSE);

		edata->valid.resize(2);
		const Variable* var = &edata->thisPtr->m_currentTrigger->vars[index];
		edata->variable.varName = var->name;
		edata->valid[0] = true;
		//invalidate expression if type doesnt fit
		if(edata->valid[1] && edata->variable.actualParameters[1].expression->returnType != var->type) {
			edata->valid[1] = false;
		}

		EnableWindow(GetDlgItem(dlg,IDC_TR_AA_BTNADD),TRUE);

	}
	else {
		//Regular entry with name and description
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_CBVAR),FALSE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_EDCONSTANT),FALSE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_STDESCR),TRUE);
		ShowWindow(GetDlgItem(dlg,IDC_TR_AA_STNAME),TRUE);
		auto& oldActualParams = data->GetActualParameters();
		auto& newParams = data->GetParameters();
		//clear parameters that can not be reused
		for (int i = 0; i < min(oldActualParams.size(),newParams.size()); i++) {
			if (!data->valid[i] || newParams[i] != oldActualParams[i].expression->returnType) {
				oldActualParams[i].actualParameters.clear();
				oldActualParams[i].expression = NULL;
			}
		}
		oldActualParams.resize(newParams.size());
		data->valid.resize(newParams.size());
		data->parts.clear();

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
					data->parts.push_back(std::move(part));
					last = i+1;
				}
			}
			std::wstring part = name->substr(last,i-last);
			data->parts.push_back(std::move(part));
			last = i+1;

			DrawName(stName,NULL,data);

			bool valid = true;
			for (bool v : data->valid) {
				if (!v) { valid = false; break; }
			}
			EnableWindow(GetDlgItem(dlg,IDC_TR_AA_BTNADD),valid);
		}
	}
	
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
			SelectFromComboBox(hwndDlg,sel,data);
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
					SelectFromComboBox(hwndDlg, index,data);

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

struct loc_AddVariableData {
	UnlimitedDialog::TRDialog* thisPtr;
	Variable var;
};

void UnlimitedDialog::TRDialog::DoAddVariable() {
	loc_AddVariableData data;
	data.thisPtr = this;
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
				if(data->var.name.size() > 0 && data->thisPtr->m_currentTrigger->FindVar(name) == NULL && data->var.defaultValue.expression != NULL) {
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

					//check if this expression allready is valid to initialize the window accordingly
					if (data->var.defaultValue.expression != NULL) {
						edata.expression = data->var.defaultValue;
						edata.valid.resize(edata.expression.actualParameters.size());
						for (int i = 0; i < edata.valid.size(); i++) edata.valid[i] = true;

					}
					int result = DialogBoxParam(General::DllInst,MAKEINTRESOURCE(IDD_TRIGGERS_ADDACTION),hwndDlg,&AddActionDialogProc,(LPARAM)&edata);
					if(result == 1) {
						data->var.defaultValue = edata.expression;
						if (data->var.name.size() > 0 && data->thisPtr->m_currentTrigger->FindVar(data->var.name) == NULL) {
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

}