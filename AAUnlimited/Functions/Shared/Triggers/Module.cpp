#include "Module.h"

#include <queue>

namespace Shared {
namespace Triggers {



Module::Module(std::wstring name,std::wstring descr,std::vector<Trigger*> triggers,const std::vector<GlobalVariable>& environment) {
	this->name = name;
	this->description = descr;
	for(Trigger* trg : triggers) {
		this->triggers.push_back(*trg);
	}
	GenerateGlobals(environment);
}

void Module::GenerateGlobals(const std::vector<GlobalVariable>& triggerGlobals) {
	
	std::queue<std::pair<const Trigger*,const Trigger::GUIAction*>> actions;
	std::queue<std::pair<const Trigger*,const ParameterisedExpression*>> exprs;
	//gather all top level expressions
	for(auto& trig : triggers) {
		for(auto* action : trig.guiActions) {
			actions.push(std::make_pair(&trig,action));
		}
	}
	while(!actions.empty()) {
		const Trigger* trg = actions.front().first;
		const Trigger::GUIAction* action = actions.front().second;
		actions.pop();
		for(auto& expr : action->action.actualParameters) {
			exprs.push(std::make_pair(trg,&expr));
		}
		for(auto* a : action->subactions) {
			actions.push(std::make_pair(trg,a));
		}
	}

	//search for variable expressions
	while(!exprs.empty()) {
		const Trigger* trg = exprs.front().first;
		const ParameterisedExpression* expr = exprs.front().second;
		exprs.pop();
		
		if(expr->expression->id == EXPR_VAR) {
			//if the name is not a local variable, its a global
			bool found = false;
			for(auto& var : trg->vars) {
				if(var.name == expr->varName) {
					found = true;
					break;
				}
			}
			if(!found) {
				//must be a global: find inside known globals
				for(auto& global : triggerGlobals) {
					if(global.name == expr->varName) {
						//found our global
						globals.push_back(global);
						break;
					}
				}
			}
		}

		for(auto& e : expr->actualParameters) {
			exprs.push(std::make_pair(trg,&e));
		}
	}

}



}
}