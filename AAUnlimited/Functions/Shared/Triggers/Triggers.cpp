#include "Triggers.h"

#include <queue>

#include "Files\Logger.h"
#include "Functions\Shared\TriggerEventDistributor.h"

namespace Shared {
namespace Triggers {


void Trigger::Initialize() {
	//all expressions that reference variables must be adjusted to their id
	//first, gather all expressions
	std::queue<ParameterisedExpression*> exprQueue;
	for(auto& event : events) {
		for(auto& expr : event.actualParameters) {
			exprQueue.push(&expr);
		}
	}
	for (auto& action : actions) {
		for (auto& expr : action.actualParameters) {
			exprQueue.push(&expr);
		}
	}
	//modify them
	while(!exprQueue.empty()) {
		ParameterisedExpression* expr = exprQueue.front();
		exprQueue.pop();
		if(expr->expression->id == 2) {
			int varIndex = -1;
			for (int i = 0; i < vars.size(); i++) {
				if (vars[i].name == expr->varName) {
					varIndex = i;
					break;
				}
			}
			if(varIndex != -1) {
				expr->varId = varIndex;
			}
			else {
				broken = true;
				LOGPRIO(Logger::Priority::WARN) << "Undefined variable " << expr->varName << " referenced in trigger " << name << "\r\n";
			}
		}
	}

	RegisterTrigger(this);
	
	bInitialized = true;
}

Trigger::~Trigger() {
	if(bInitialized) {
		UnregisterTrigger(this);
	}
}

void Trigger::InsertAction(const ParameterisedAction& action,int insertAfter) {
	bInitialized = false;
	if(insertAfter == INSERT_START) {
		actions.insert(actions.begin(), action);
	}
	else if(insertAfter == INSERT_END) {
		actions.push_back(action);
	}
	else {
		if (insertAfter < 0 || (unsigned int)insertAfter >= actions.size()) return;
		actions.insert(actions.begin() + (insertAfter+1),action);
	}
}

void Trigger::InsertVariable(const Variable& var,int insertAfter) {
	bInitialized = false;
	if (insertAfter == INSERT_START) {
		vars.insert(vars.begin(),var);
	}
	else if (insertAfter == INSERT_END) {
		vars.push_back(var);
	}
	else {
		if (insertAfter < 0 || (unsigned int)insertAfter >= vars.size()) return;
		vars.insert(vars.begin() + (insertAfter+1),var);
	}
}

void Trigger::InsertEvent(const ParameterisedEvent& event,int insertAfter) {
	bInitialized = false;
	if (insertAfter == INSERT_START) {
		events.insert(events.begin(),event);
	}
	else if (insertAfter == INSERT_END) {
		events.push_back(event);
	}
	else {
		if (insertAfter < 0 || (unsigned int)insertAfter >= events.size()) return;
		events.insert(events.begin() + (insertAfter+1),event);
	}
}

Variable * Trigger::FindVar(std::wstring name) {
	for(auto& elem : vars) {
		if (elem.name == name) return &elem;
	}
	return NULL;
}

};
};