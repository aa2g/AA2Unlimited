#pragma once

#include "Functions\AAEdit\UnlimitedDialog.h"
#include "Functions\Shared\Triggers\Triggers.h"

#include <vector>


namespace AAEdit {

	using namespace Shared::Triggers;

struct loc_AddData {
	

	enum {
		EVENT,EXPRESSION,ACTION,VARIABLE
	} type;
	UnlimitedDialog::TRDialog* thisPtr;
	std::vector<RECT> clickRects;
	std::vector<bool> valid;
	std::vector<std::wstring> parts;
	bool allowChange;

	loc_AddData();

	virtual void InitializeComboBox(HWND box) = 0;
	virtual void SelectFromComboBox(HWND dlg,int index) = 0;
	void SelectFromComboBox(HWND dlg,int index, const std::wstring* name,const std::wstring* descr);
	virtual void AddExpression(ParameterisedExpression& exp,int paramN);
	virtual void RemoveExpression(int paramN);
	virtual std::vector<ParameterisedExpression>& GetActualParameters() = 0;
	virtual const std::vector<Types>& GetParameters() = 0; //warning: make sure actual data exists, else it crashes
	virtual DWORD GetId() = 0;

	virtual std::wstring GetParamString(int paramN,_Out_ bool* valid) = 0;

	//opens a window to add a parameter. returns true if the parameter was added successfully
	virtual bool PromptAddParameter(HWND parentDialog,int i,Types retType = TYPE_INVALID);

	std::wstring ToString();
};

struct loc_SelectVariableData : public loc_AddData {
	ParameterisedExpression variable;
	Types retType;

	loc_SelectVariableData();
	virtual std::vector<ParameterisedExpression>& GetActualParameters();
	virtual const std::vector<Types>& GetParameters();
	virtual DWORD GetId();
	virtual void InitializeComboBox(HWND box);
	virtual void SelectFromComboBox(HWND dlg,int index);
	virtual std::wstring GetParamString(int paramN,bool* retValid);
};

struct loc_AddActionData : public loc_AddData {
	ParameterisedAction action;

	loc_AddActionData();
	virtual std::vector<ParameterisedExpression>& GetActualParameters();
	virtual const std::vector<Types>& GetParameters();
	virtual DWORD GetId();
	virtual void InitializeComboBox(HWND box);
	virtual void SelectFromComboBox(HWND dlg,int index);
	virtual std::wstring GetParamString(int paramN,bool* retValid);

	//considers special actions that require variables instead
	virtual bool PromptAddParameter(HWND parentDialog,int i,Types retType = TYPE_INVALID);
};


struct loc_AddEventData : public loc_AddData {
	ParameterisedEvent event;

	loc_AddEventData();
	virtual std::vector<ParameterisedExpression>& GetActualParameters();
	virtual const std::vector<Types>& GetParameters();
	virtual DWORD GetId();
	virtual void InitializeComboBox(HWND box);
	virtual void SelectFromComboBox(HWND dlg,int index);
	virtual std::wstring GetParamString(int paramN,bool* retValid);
};


struct loc_AddExpressionData : public loc_AddData {
	ParameterisedExpression expression;
	Types retType;

	loc_AddExpressionData();
	virtual std::vector<ParameterisedExpression>& GetActualParameters();
	virtual const std::vector<Types>& GetParameters();
	virtual DWORD GetId();
	virtual void InitializeComboBox(HWND box);
	virtual void SelectFromComboBox(HWND dlg,int index);
	virtual std::wstring GetParamString(int paramN,bool* retValid);
};


struct loc_AddVariableData {
	UnlimitedDialog::TRDialog* thisPtr;
	Variable var;
	bool onlyConstantInit = false;
	bool checkLocalNameConflict = false;
	bool checkGlobalNameConflict = false;
};

struct loc_ModuleInfo {
	UnlimitedDialog::TRDialog* thisPtr;
	std::wstring name;
	std::wstring description;

	std::vector<std::wstring> saveTriggers;
	std::vector<Shared::Triggers::GlobalVariable*> saveGlobals;
	std::vector<std::wstring> dependencies;
};

/////////////////

#define TVN_MULTISELCHANGED (WM_USER + 0)
#define TVM_GETMULTISEL (WM_USER+1)

struct NMTVMULTI {
	NMHDR hdr;
	HTREEITEM oldItem;
	HTREEITEM newItem;
	bool selectBoth;
	std::vector<HTREEITEM>* currentSelection;
};

struct TreeViewMultiselectData {
	std::vector<HTREEITEM> selection;
	bool supressPaint = false;
};
LRESULT CALLBACK TreeViewMultiselect(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam,UINT_PTR uIdSubclass,DWORD_PTR dwRefData);

inline void MakeTreeViewMultiselect(HWND treeView) {
	SetWindowSubclass(treeView,TreeViewMultiselect,0,(DWORD_PTR)new TreeViewMultiselectData);
}

}