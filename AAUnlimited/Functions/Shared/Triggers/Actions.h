#pragma once

#include "Value.h"
#include "Expressions.h"
#include "InfoData.h"

#include <vector>

namespace Shared {
namespace Triggers {


class ParameterisedAction;
class Thread;

/*
* An action that can be taken. An action has a side effect to be archieved, such as
* setting a variable, changing a cards aau data set or doing something in the game
*/
class Action/* : public InfoData*/ {
public:
	DWORD id;								//a unique identifier. This id is only unique inside the class it is used in, not accross all eev's
	int category;							//category is a string that is appended in the gui for easier navigation
	std::wstring name;						//a name, visible from the dropdown menu
	std::wstring interactiveName;			//name in the gui; parameters are replaced by %ps and can be clicked to be changed
	std::wstring description;				//description
	std::vector<Types> parameters;			//list of parameters

	void (Thread::*func)(std::vector<Value>&);		//function that does this action

	static const Action* FromId(int id);
};

enum ActionCategories {
	ACTIONCAT_GENERAL,
	ACTIONCAT_MODIFY_CARD,
	ACTIONCAT_FLOW_CONTROL,
	ACTIONCAT_MODIFY_CHARACTER,
	ACTIONCAT_EVENT,
	ACTIONCAT_NPCACTION,
	ACTIONCAT_N
};

enum Actions {
	ACTION_INVALID = 0,
	ACTION_SETVAR = 1,
	ACTION_IF = 2,
	ACTION_ELSEIF = 3,
	ACTION_ELSE = 4,

	ACTION_CONDJUMP = 5,
	ACTION_LOOP = 6,
	ACTION_CONDBREAK = 7,
	ACTION_CONDCONTINUE = 8,
	ACTION_FORLOOP = 9
};

extern std::wstring g_ActionCategories[ACTIONCAT_N];
extern std::vector<Action> g_Actions;

class ParameterisedAction {
public:
	const Action* action;
	std::vector<ParameterisedExpression> actualParameters;

	inline ParameterisedAction() : action(NULL) {}
	ParameterisedAction(DWORD actionId,const std::vector<ParameterisedExpression>& params);

	
};



inline const Action* Action::FromId(int id) {
	if (id < 1) return NULL;
	if (id > g_Actions.size()) return NULL;
	return &g_Actions[id-1];
}


}
}