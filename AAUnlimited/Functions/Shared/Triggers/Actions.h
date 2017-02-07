#pragma once

#include "Value.h"
#include "Expressions.h"

#include <vector>

namespace Shared {
namespace Triggers {


class ParameterisedAction;
class Thread;

/*
* An action that can be taken. An action has a side effect to be archieved, such as
* setting a variable, changing a cards aau data set or doing something in the game
*/
class Action {
public:
	DWORD id;								//identifier used to store in card
	std::wstring name;						//a name, visible from the dropdown menu
	std::wstring description;				//description with parameters inserted at %p s
	std::vector<Types> parameters;			//list of parameters

	void (Thread::*func)(std::vector<Value>&);		//function that does this action

	static const Action* FromId(int id);
};

extern std::vector<Action> g_Actions;

class ParameterisedAction {
public:
	const Action* action;
	std::vector<ParameterisedExpression> actualParameters;

	inline ParameterisedAction() : action(NULL) {}
};



inline const Action* Action::FromId(int id) {
	if (id < 1) return NULL;
	return &g_Actions[id-1];
}


}
}