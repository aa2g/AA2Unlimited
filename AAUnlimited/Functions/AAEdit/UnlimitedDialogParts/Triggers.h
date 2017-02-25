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
	virtual std::wstring GetParamString(int paramN,bool* retValid);
};

struct loc_AddActionData : public loc_AddData {
	ParameterisedAction action;

	loc_AddActionData();
	virtual std::vector<ParameterisedExpression>& GetActualParameters();
	virtual const std::vector<Types>& GetParameters();
	virtual DWORD GetId();
	virtual void InitializeComboBox(HWND box);
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
	virtual std::wstring GetParamString(int paramN,bool* retValid);
};



}