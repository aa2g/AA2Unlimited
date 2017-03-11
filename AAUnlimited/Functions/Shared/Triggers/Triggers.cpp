#include "Triggers.h"

#include <queue>
#include <stack>

#include "Files\Logger.h"
#include "Functions\Shared\TriggerEventDistributor.h"
#include "Thread.h"

namespace Shared {
namespace Triggers {

namespace {
	int CalculateJumpDistance(int jumpAction, int jumpTo) {
		//jumps are relative from the action after the jump, so the distance to jump
		//is (jumpAction) - (jumpTo + 1)
		return jumpTo - jumpAction - 1;
	}
};


void Trigger::AddActionsFromGuiActions(std::vector<GUIAction*>& guiActions, AddActionsFromGuiActions_State state) {
	for(auto* elem : guiActions) {
		DWORD elemId = elem->action.action->id;
		if(elemId != ACTION_IF && elemId != ACTION_ELSEIF && elemId != ACTION_ELSE) {
			//no if/then/else chain, so clear that
			state.ResetITEState();
		}
		switch(elemId) {
		case ACTION_ELSEIF:
		case ACTION_IF: {
			//add a jump, make it jump on !if condition
			ParameterisedExpression exprJumpN(TYPE_INT,Value(0));
			ParameterisedExpression exprJumpCond(TYPE_BOOL,EXPR_BOOL_NOT,elem->action.actualParameters);
			ParameterisedAction jumpAction(ACTION_CONDJUMP, { exprJumpN, exprJumpCond });
			actions.push_back(jumpAction);
			int headerJump = actions.size() - 1;
			//insert the subactions associated with this if
			AddActionsFromGuiActions(elem->subactions,AddActionsFromGuiActions_State());
			//add an additional jump at the end that will skip elseif/else parts
			exprJumpN = ParameterisedExpression(TYPE_INT,Value(0));
			exprJumpCond = ParameterisedExpression(TYPE_BOOL,Value(true));
			jumpAction = ParameterisedAction(ACTION_CONDJUMP,{ exprJumpN, exprJumpCond });
			actions.push_back(jumpAction);
			int endJump = actions.size() - 1;
			//fix header jump target for now
			int jumpN = CalculateJumpDistance(headerJump, endJump+1); //jump from header to end jump, and then 1 more (beyond it)
			actions[headerJump].actualParameters[0].constant.iVal = jumpN;
			if(elem->action.action->id == ACTION_IF || state.ifStartJump == -1) {
				//starting a new if
				//insert our if into the state
				state.ifStartJump = headerJump;
				state.ifEndJump = endJump;
			}
			else {
				//adding a else if
				//first, switch if end label to our end
				jumpN = CalculateJumpDistance(state.ifEndJump,endJump+1);
				actions[state.ifStartJump].actualParameters[0].constant.iVal = jumpN;
				//now, switch all the else if end labels as well
				for(auto& elseifLabels : state.elseIfLabels) {
					int elseIfEnd = elseifLabels.second;
					jumpN = CalculateJumpDistance(elseIfEnd,endJump+1);
					actions[elseIfEnd].actualParameters[0].constant.iVal = jumpN;
				}
				//lastly, add us to the else if labels
				state.elseIfLabels.push_back(std::make_pair(headerJump, endJump));
			}
			break; }
		case ACTION_ELSE: {
			//dump actions
			AddActionsFromGuiActions(elem->subactions,AddActionsFromGuiActions_State());
			if(state.ifStartJump != -1) {
				//as with the if/else if, change the end labels to fit into this
				int end = actions.size() - 1;
				int jumpN = end - state.ifEndJump + 1;
				actions[state.ifStartJump].actualParameters[0].constant.iVal = jumpN;
				//now, switch all the else if end labels as well
				for (auto& elseifLabels : state.elseIfLabels) {
					int elseIfEnd = elseifLabels.second;
					jumpN = end - elseIfEnd + 1;
					actions[elseIfEnd].actualParameters[0].constant.iVal = jumpN;
				}
			}
			//clear ite state, as an else always ends an ite chain
			state.ResetITEState();
			break; }
		case ACTION_LOOP: {
			//dump actions first
			AddActionsFromGuiActions_State subState;
			std::vector<int> breaks;
			std::vector<int> continues;
			subState.loopBreaks = &breaks;
			subState.loopContinues = &continues;
			int loopHead = actions.size(); //first loop that this will print
			AddActionsFromGuiActions(elem->subactions,subState); //let it fill the breaks and continues
			int loopEnd = actions.size(); //first action that will appear after this loop (the backjump)
			if(loopHead != loopEnd) {
				//insert the jump that actually does the looping
				int jumpN = CalculateJumpDistance(loopEnd,loopHead);
				ParameterisedExpression exprJumpN(TYPE_INT,Value(0));
				ParameterisedExpression exprJumpCond(TYPE_BOOL,EXPR_BOOL_NOT,elem->action.actualParameters);
				ParameterisedAction jumpAction(ACTION_CONDJUMP,{ exprJumpN, exprJumpCond });
				actions.push_back(jumpAction);
				loopEnd++; //loop ends after this jump now
				//redirect all the continues
				for(int contJump : continues) {
					jumpN = CalculateJumpDistance(contJump,loopHead);
					actions[contJump].actualParameters[0].constant.iVal = jumpN;
				}
				//redirect all the breaks
				for(int breakJump : breaks) {
					jumpN = CalculateJumpDistance(breakJump,loopEnd);
					actions[breakJump].actualParameters[0].constant.iVal = jumpN;
				}
			}
			break; }
		case ACTION_FORLOOP: {
			//first, set the variable to the given value
			ParameterisedAction setVarAction(ACTION_SETVAR,{ elem->action.actualParameters[0], elem->action.actualParameters[1] });
			actions.push_back(setVarAction);
			int loopHead = actions.size();
			//this next action will be our loop header. it does the conditional exit on var(0) >= till(2)
			ParameterisedExpression exprJumpN(TYPE_INT,Value(0));
			ParameterisedExpression exprJumpCond(TYPE_BOOL,EXPR_BOOL_GTE_INT, { elem->action.actualParameters[0],elem->action.actualParameters[2] });
			ParameterisedAction jumpAction(ACTION_CONDJUMP,{ exprJumpN, exprJumpCond });
			actions.push_back(jumpAction);
			//from here, do all the actions
			AddActionsFromGuiActions_State subState;
			std::vector<int> breaks;
			std::vector<int> continues;
			subState.loopBreaks = &breaks;
			subState.loopContinues = &continues;
			AddActionsFromGuiActions(elem->subactions,subState);
			//increase the variable by one
			int continuePoint = actions.size();
			ParameterisedExpression const1(TYPE_INT,Value(1));
			ParameterisedExpression incExpr(TYPE_INT, EXPR_INT_PLUS, {elem->action.actualParameters[0], const1});
			ParameterisedAction incAction(ACTION_SETVAR,{elem->action.actualParameters[0], incExpr});
			actions.push_back(incAction);
			//jump back up, unconditionally
			int endJump = actions.size();
			int jumpN = CalculateJumpDistance(endJump,loopHead);
			exprJumpN = ParameterisedExpression(TYPE_INT,Value(jumpN));
			exprJumpCond = ParameterisedExpression(TYPE_BOOL,Value(true));
			jumpAction = ParameterisedAction(ACTION_CONDJUMP,{ exprJumpN, exprJumpCond });
			actions.push_back(jumpAction);
			int loopEnd = actions.size();
			//lastly, fix continues and breaks
			//redirect all the continues
			for (int contJump : continues) {
				jumpN = CalculateJumpDistance(contJump,continuePoint);
				actions[contJump].actualParameters[0].constant.iVal = jumpN;
			}
			//redirect all the breaks
			for (int breakJump : breaks) {
				jumpN = CalculateJumpDistance(breakJump,loopEnd);
				actions[breakJump].actualParameters[0].constant.iVal = jumpN;
			}
			break; }
		case ACTION_CONDCONTINUE:
			if(state.loopContinues != NULL) {
				//add jump stub
				ParameterisedExpression exprJumpN(TYPE_INT,Value(0));
				ParameterisedAction jumpAction(ACTION_CONDJUMP,{ exprJumpN, elem->action.actualParameters[0] });
				actions.push_back(jumpAction);
				state.loopContinues->push_back(actions.size() - 1);
			}
			break;
		case ACTION_CONDBREAK:
			if (state.loopBreaks != NULL) {
				//add jump stub
				ParameterisedExpression exprJumpN(TYPE_INT,Value(0));
				ParameterisedAction jumpAction(ACTION_CONDJUMP,{ exprJumpN, elem->action.actualParameters[0] });
				actions.push_back(jumpAction);
				state.loopBreaks->push_back(actions.size() - 1);
			}
			break;
		default:
			actions.push_back(elem->action);
			AddActionsFromGuiActions(elem->subactions, state);
			break;
		}
	}
}

void Trigger::Initialize(std::vector<GlobalVariable>* globals, int owningCard) {
	globalVars = globals;
	this->owningCard = owningCard;

	//check for global consistency
	if(globalVars == NULL) {
		broken = true;
	}

	AddActionsFromGuiActions_State initState;
	AddActionsFromGuiActions(guiActions,initState);

	//all expressions that reference variables must be adjusted to their id
	//first, gather all expressions
	std::queue<ParameterisedExpression*> exprQueue;
	for(auto& event : events) {
		for(auto& expr : event.actualParameters) {
			exprQueue.push(&expr);
		}
	}
	for(auto& var : vars) {
		exprQueue.push(&var.defaultValue);
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

		//if they reference vars, set their id
		if(expr->expression->id == 2) {
			int varIndex = -1;
			//search in globals
			if(globalVars != NULL) {
				for(int i = 0; i < globalVars->size(); i++) {
					if((*globalVars)[i].name == expr->varName) {
						varIndex = i | GLOBAL_VAR_FLAG;
						break;
					}
				}
			}
			if(varIndex == -1) {
				//search in locals
				for (int i = 0; i < vars.size(); i++) {
					if (vars[i].name == expr->varName) {
						varIndex = i;
						break;
					}
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
		//add sub expressions to queue
		for(auto& expr : expr->actualParameters) {
			exprQueue.push(&expr);
		}
	}

	//type checks: make sure all events, variables, actions and expressions have the right types
	auto CheckExpressions = [&](ParameterisedExpression* exprToCheck, int n, const std::wstring& type, const std::wstring& name) {
		std::queue<ParameterisedExpression*> queue;
		queue.push(exprToCheck);
		while(!queue.empty()) {
			ParameterisedExpression* expr = queue.front();
			queue.pop();
			if (expr->actualParameters.size() != expr->expression->parameters.size()) {
				LOGPRIO(Logger::Priority::WARN) << "Trigger Init error; parameter missmatch in trigger " <<  name << ", " << type << " nr. " << n << 
					"(" << name << "): " << expr->expression->name << " expects " << expr->expression->parameters.size() << 
					" parameters, but got " << expr->actualParameters.size() << "\r\n";
				broken = true;
				continue;
			}
			for (int j = 0; j < expr->actualParameters.size(); j++) {
				if (expr->actualParameters[j].expression->returnType != expr->expression->parameters[j]) {
					LOGPRIO(Logger::Priority::WARN) << "Trigger Init error; parameter missmatch in trigger " <<  name << ", " << type << " nr. " << n <<
						"(" << name << "): " << expr->expression->name << ", param " << j << " expects type " << 
						g_Types[expr->expression->parameters[j]].name << ", but got type " << 
						g_Types[expr->actualParameters[j].expression->returnType].name << "\r\n";
					broken = true;
					break;
				}
			}
			//add sub expressions to queue
			for (auto& expr : expr->actualParameters) {
				queue.push(&expr);
			}
		}
	};
	//check events
	for (int i = 0; i < events.size(); i++){
		auto& event = events[i];
		if (event.actualParameters.size() != event.event->parameters.size()) {
			LOGPRIO(Logger::Priority::WARN) << "Trigger Init error; parameter missmatch in trigger " <<  name << ", event nr. " << i << ": " <<
				event.event->name << " expects " << event.event->parameters.size() << " parameters, but got " << event.actualParameters.size() << "\r\n";
			broken = true;
			continue;
		}
		for (int j = 0; j < event.actualParameters.size(); j++) {
			if (event.actualParameters[j].expression->returnType != event.event->parameters[j]) {
				LOGPRIO(Logger::Priority::WARN) << "Trigger Init error; parameter missmatch in trigger " <<  name << ": " <<
					event.event->name << ", param " << j << " expects type " << g_Types[event.event->parameters[j]].name <<
					", but got type " << g_Types[event.actualParameters[j].expression->returnType].name << "\r\n";
				broken = true;
				break;
			}
			CheckExpressions(&event.actualParameters[j],j,TEXT("Event"),event.event->name);
		}
	}


	//check variables
	for (int i = 0; i < vars.size(); i++) {
		auto& var = vars[i];
		if (var.type != var.defaultValue.expression->returnType) {
			LOGPRIO(Logger::Priority::WARN) << "Trigger Init error; parameter missmatch in trigger " <<  name << ", var nr. " << i << ": " <<
				var.name << " expects init expression of type " << g_Types[var.type].name << " , but got " << 
				g_Types[var.defaultValue.expression->returnType].name << "\r\n";
			broken = true;
			continue;
		}
		CheckExpressions(&var.defaultValue,0,TEXT("Variable"),var.name);
		
	}
	//check actions
	for (int i = 0; i < actions.size(); i++) {
		auto& action = actions[i];
		//make sure parameters and actual parameters are correct
		if (action.actualParameters.size() != action.action->parameters.size()) {
			LOGPRIO(Logger::Priority::WARN) << "Trigger Init error; parameter missmatch in trigger " <<  name << ", action nr. " << i << ": " <<
				action.action->name << " expects " << action.action->parameters.size() << " parameters, but got " << action.actualParameters.size() << "\r\n";
			broken = true;
			continue;
		}
		for (int j = 0; j < action.actualParameters.size(); j++) {
			if(action.action->id == ACTION_SETVAR) {
				//special meaning for those
				if(action.actualParameters[0].expression->id != EXPR_VAR) {
					LOGPRIO(Logger::Priority::WARN) << "Trigger Init error; parameter missmatch in trigger " <<  name << ", action nr. " << i << ": " <<
						action.action->name << " expects var expression as first parameter, but got " << action.actualParameters[0].expression->id << "\r\n";
					broken = true;
					break;
				}
				else if(action.actualParameters[1].expression->returnType != action.actualParameters[0].expression->returnType) {
					LOGPRIO(Logger::Priority::WARN) << "Trigger Init error; parameter missmatch in trigger " <<  name << ", action nr. " << i << ": " <<
						action.action->name << ", param 2 did not have matching type to variable; types (var, expr) where " <<
						g_Types[action.actualParameters[1].expression->returnType].name << ", " <<
						g_Types[action.actualParameters[0].expression->returnType].name << "\r\n";
					broken = true;
					break;
				}
			}
			else if (action.actualParameters[j].expression->returnType != action.action->parameters[j]) {
				LOGPRIO(Logger::Priority::WARN) << "Trigger Init error; parameter missmatch in trigger " <<  name << ", action nr. " << i << ": " <<
					action.action->name << ", param " << j << " expects type " << g_Types[action.action->parameters[j]].name << 
					", but got type " << g_Types[action.actualParameters[j].expression->returnType].name << "\r\n";
				broken = true;
				break;
			}
			CheckExpressions(&action.actualParameters[j],j,TEXT("Action"),action.action->name);
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

Trigger::GUIAction::~GUIAction() {
	for(auto* subAct : subactions) {
		delete subAct;
	}
}

Trigger::GUIAction::GUIAction(const GUIAction& rhs) : action(rhs.action), parent(NULL) {
	subactions.reserve(rhs.subactions.size());
	for(auto* subAct : subactions) {
		subactions.push_back(new GUIAction(*subAct));
	}
	FixParents();
}
Trigger::GUIAction::GUIAction(GUIAction&& rhs) : action(std::move(rhs.action)),subactions(std::move(rhs.subactions)), parent(NULL) {
	FixParents();
}
Trigger::GUIAction& Trigger::GUIAction::operator=(const GUIAction& rhs) {
	this->parent = NULL;
	this->action = rhs.action;
	subactions.reserve(rhs.subactions.size());
	for (auto* subAct : subactions) {
		subactions.push_back(new GUIAction(*subAct));
	}
	FixParents();
	return *this;
}
Trigger::GUIAction& Trigger::GUIAction::operator==(GUIAction&& rhs) {
	this->parent = NULL;
	this->action = std::move(rhs.action);
	this->subactions = std::move(rhs.subactions);
	return *this;
}

void Trigger::GUIAction::FixParents() {
	for(auto* elem : subactions) {
		elem->parent = this;
		elem->FixParents();
	}
}


Shared::Triggers::GlobalVariable::GlobalVariable(Variable & var) : id(0),type(var.type),name(var.name) {
	defaultValue = var.defaultValue.constant;
	currentValue = defaultValue;
	initialized = false;
}


};
};