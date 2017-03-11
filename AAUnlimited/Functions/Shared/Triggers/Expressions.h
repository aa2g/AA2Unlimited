#pragma once

#include "Value.h"
#include "InfoData.h"
#include "NamedConstant.h"

#include <string>
#include <Windows.h>
#include <vector>

namespace Shared {
namespace Triggers {

class Thread;

/*
* An Expression takes zero or more parameters of a certain type and returns a value of a type.
* They can be used to fill the parameters of events or actions
*/
class Expression/* : public InfoData*/ {
public:
	DWORD id;								//a unique identifier. This id is only unique inside the class it is used in, not accross all eev's
	int category;							//category is a string that is appended in the gui for easier navigation
	std::wstring name;						//a name, visible from the dropdown menu
	std::wstring interactiveName;			//name in the gui; parameters are replaced by %ps and can be clicked to be changed
	std::wstring description;				//description
	std::vector<Types> parameters;
	Types returnType;

	Value(Thread::*func)(std::vector<Value>&);

	static const Expression* FromId(Types type, int id);
};

enum ExpressionCategories {
	EXPRCAT_GENERAL,
	EXPRCAT_EVENT,
	EXPRCAT_MATH,
	EXPRCAT_CHARPROP,
	EXPRCAT_COMPARISION_INT,
	EXPRCAT_COMPARISION_STRING,
	EXPRCAT_COMPARISION_BOOL,
	EXPRCAT_COMPARISION_FLOAT,
	EXPRCAT_N
};

enum Expressions {
	EXPR_INVALID = 0,
	EXPR_CONSTANT = 1,
	EXPR_VAR = 2,
	EXPR_NAMEDCONSTANT = 3,

	EXPR_INT_PLUS = 5,

	EXPR_BOOL_GTE_INT = 7,

	EXPR_BOOL_NOT = 11
};

static const int GLOBAL_VAR_FLAG = 0x40000000;

extern std::wstring g_ExpressionCategories[EXPRCAT_N];
extern std::vector<Expression> g_Expressions[N_TYPES];

class ParameterisedExpression {
public:
	const Expression* expression;
	std::vector<ParameterisedExpression> actualParameters;
	Value constant; //used when expression is 1 (constant)
	std::wstring varName; //used when expression is 2 (variable)
	const NamedConstant* namedConstant; //used when expression is 3 (named constant)

	int varId; //set at trigger initialisation and used by thread for variable addressing. negative means globals

	inline ParameterisedExpression() : expression(NULL), varId(0), namedConstant(NULL) {}
	ParameterisedExpression(Types type, DWORD exprId, const std::vector<ParameterisedExpression>& params);
	ParameterisedExpression(Types type, Value constant);
	ParameterisedExpression(Types type, std::wstring var);
};



inline const Expression* Expression::FromId(Types type, int id) {
	if (type < 0 || type >= N_TYPES) return NULL;
	if (id < 1 || id > g_Expressions[type].size()) return NULL;
	return &g_Expressions[type][id-1];
}


}
}