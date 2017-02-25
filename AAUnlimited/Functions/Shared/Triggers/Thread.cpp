#include "Thread.h"

#include "Triggers.h"
#include "Files\Logger.h"

namespace Shared {
namespace Triggers {


Thread::GlobalStorage Thread::globalStorage;


void Thread::ExecuteTrigger(Trigger* trg) {
	if (trg == NULL) return;
	if (!trg->IsInitalized()) trg->Initialize();
	if (trg->IsBroken()) return;
	executeCount = 0;
	maxExecuteCount = 1000;
	execTrigger = trg;
	execStarted = true;
	execFinished = false;

	//initialize variables
	localStorage.vars.resize(trg->vars.size());
	for(int i = 0; i < trg->vars.size(); i++) {
		localStorage.vars[i].variable = &trg->vars[i];
		localStorage.vars[i].currValue = EvaluateExpression(localStorage.vars[i].variable->defaultValue);
		if(localStorage.vars[i].currValue.type == TYPE_INVALID) {
			return;
		}
	}

	//execute actions
	for(ip = 0; ip < trg->actions.size() && !execFinished; ip++) {
		//get action
		auto& action = trg->actions[ip];
		ExecuteAction(action);
		LOGPRIO(Logger::Priority::WARN) << "Thread reached execution limit; trigger " <<  execTrigger->name << "\r\n";
		if(executeCount > maxExecuteCount) {
			return;
		}
	}
}

bool Thread::ExecuteAction(ParameterisedAction& action) {
	if(action.action->id == ACTION_SETVAR) {
		//handle these specially
		VariableInstance* var = &localStorage.vars[action.actualParameters[0].varId];
		Value val = EvaluateExpression(action.actualParameters[1]);
		if(val.type == TYPE_INVALID) {
			return false;
		}
		var->currValue = val;
	}
	else {
		//evaluate parameters
		std::vector<Value> values;
		values.resize(action.actualParameters.size());
		for (int j = 0; j < action.actualParameters.size(); j++) {
			auto& param = action.actualParameters[j];
			values[j] = EvaluateExpression(param);
			if (values[j].type == TYPE_INVALID) {
				return false;
			}
		}
		//execute function
		(this->*(action.action->func)) (values);
	}
	
	return true;
}

Value Thread::EvaluateExpression(ParameterisedExpression& expr) {
	executeCount++;
	if(expr.expression->id == EXPR_CONSTANT) {
		//constant
		return expr.constant;
	}
	else if (expr.expression->id == EXPR_VAR) {
		//variable
		VariableInstance* var = &localStorage.vars[expr.varId];
		return var->currValue;
	}
	else {
		if(expr.expression->returnType == TYPE_BOOL && (expr.expression->id == 3 || expr.expression->id == 4)) {
			//short circut evaluation required
			if(expr.expression->id == 3) {
				//and
				for (int j = 0; j < expr.actualParameters.size(); j++) {
					auto& param = expr.actualParameters[j];
					Value result = EvaluateExpression(param);
					if (!result.bVal) return Value(false);
				}
				return Value(true);
			}
			else {
				//or
				bool returnValue = false;
				for (int j = 0; j < expr.actualParameters.size(); j++) {
					auto& param = expr.actualParameters[j];
					Value result = EvaluateExpression(param);
					if (result.bVal) return Value(true);
				}
				return Value(false);
			}
			
		}
		else {
			//normal function
			
			//evaluate parameters
			std::vector<Value> values;
			values.resize(expr.actualParameters.size());
			for (int j = 0; j < expr.actualParameters.size(); j++) {
				auto& param = expr.actualParameters[j];
				values[j] = EvaluateExpression(param);
				if (values[j].type == TYPE_INVALID) {
					LOGPRIO(Logger::Priority::WARN) << "Thread Execution error; failed expression in trigger " <<  execTrigger->name << "\r\n";
					return Value();
				}
			}
			//execute function
			return (this->*(expr.expression->func)) (values);
		}
	}
}

}
}
