#pragma once

#include "Value.h"

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
class Expression {
public:
	DWORD id;
	std::wstring name;
	std::wstring description;
	std::vector<Types> parameters;
	Types returnType;

	Value(Thread::*func)(std::vector<Value>&);

	static const Expression* FromId(Types type, int id);
};

extern std::vector<Expression> g_Expressions[N_TYPES];

class ParameterisedExpression {
public:
	const Expression* expression;
	std::vector<ParameterisedExpression> actualParameters;
	Value constant; //used when expression is 1 (constant)
	std::wstring varName; //used when expression is 2 (variable)

	int varId; //set at trigger initialisation and used by thread for variable addressing

	inline ParameterisedExpression() : expression(NULL), varId(0) {}
};



inline const Expression* Expression::FromId(Types type, int id) {
	if (id < 1) return NULL;
	return &g_Expressions[type][id-1];
}


}
}