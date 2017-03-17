#include "Module.h"

#include <queue>

namespace Shared {
namespace Triggers {



Module::Module(std::wstring name,std::wstring descr,const std::vector<Trigger*>& triggers, 
			   const std::vector<GlobalVariable>& environment, const std::vector<Module>& moduleEnvironment) {
	this->name = name;
	this->description = descr;
	this->dependencies = dependencies;
	GenerateTrigGlobalDepend(triggers,environment,moduleEnvironment);
}

void Module::GenerateTrigGlobalDepend(const std::vector<Trigger*>& triggers,const std::vector<GlobalVariable>& triggerGlobals, const std::vector<Module>& modules) {
	for (Trigger* trg : triggers) {
		this->triggers.push_back(*trg);
	}

	std::queue<std::pair<const Trigger*,const Trigger::GUIAction*>> actions;
	std::queue<std::pair<const Trigger*,const ParameterisedExpression*>> exprs;
	//gather all top level expressions
	for(auto& trig : this->triggers) {
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
						//found our global; make sure we havnt added this one yet
						found = false;
						for(auto& gvar : globals) {
							if(gvar.name == expr->varName) {
								found = true;
								break;
							}
						}
						if(!found) {
							globals.push_back(global);
						}
						break;
					}
				}
			}
		}

		for(auto& e : expr->actualParameters) {
			exprs.push(std::make_pair(trg,&e));
		}
	}

	//from the referenced globals, see which globals belong to other modules currently included
	for(int i = 0; i < globals.size(); i++) {
		auto& var = globals[i];
		for (auto& mod : modules) {
			for (auto& modVar : mod.globals) {
				if (modVar.name == var.name) {
					//insert into our modules if it isnt in there allready
					bool found = false;
					for(auto& currMod : this->dependencies) {
						if(currMod == mod.name) {
							found = true;
							break;
						}
					}
					if (!found) this->dependencies.push_back(mod.name);
					//remove this var from our varlist; it belongs to this module
					globals.erase(globals.begin() + i);
					i--; //adjust i accordingly
					goto loopEnd;
				}
			}
		}
		loopEnd:;
	}
	
		

}



}
}