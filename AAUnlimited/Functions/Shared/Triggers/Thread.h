#pragma once

#include "Value.h"
#include "Expressions.h"
#include "Actions.h"
#include "Triggers.h"

#include <vector>

namespace Shared {
namespace Triggers {


class Trigger;
class TriggerEnvironment;

/*
 * An executing thread. It has some local things and executes a trigger.
 */ 
class Thread {
	int executeCount;		//if count is too high, execution will be terminated to prevent freezes from endless loops
	int maxExecuteCount;	
	Trigger* execTrigger;	//the trigger that will be executed
	bool execStarted;	
	bool execFinished;

	int ip;					//instruction pointer
	int thisCard;			//the card to whom this trigger belongs

public:

	//local storage

	struct LocalStorage {
		std::vector<VariableInstance> vars;		//variables for this trigger
		int triggeringCard;						//the card whose trigger got fired
	} localStorage;

	/*
	 * This storage only exists once and stores event-specific data
	 */
	static struct GlobalStorage {
		int period;

	} globalStorage;

public:
	void ExecuteTrigger(Trigger* trg);

	//////////////////////////
	// Actions

	void ConditionalJump(std::vector<Value>& params);
	void EndExecution(std::vector<Value>& params);

	void SwitchAAUDataSet(std::vector<Value>& params);

	///////////////////////
	// Expressions

	Value GetTriggeringCard(std::vector<Value>& params);	//int ()
	Value GetThisCard(std::vector<Value>& params); //int ()

	//basic int stuff
	Value GetRandomInt(std::vector<Value>& params); //int(int,int)
	Value AddIntegers(std::vector<Value>& params);	//int(int,int)
	Value SubstractIntegers(std::vector<Value>& params); //int(int,int)
	Value DivideIntegers(std::vector<Value>& params); //int(int,int)
	Value MultiplyIntegers (std::vector<Value>& params); //int(int,int)

	//basic bool stuff
	//these two are handled directly cause short circut evaluation
	//Value BoolAnd(std::vector<Value>& params);//bool(bool,bool) 
	//Value BoolOr(std::vector<Value>& params);//bool(bool,bool)
	Value GreaterThanIntegers(std::vector<Value>& params); //bool(int,int)
	Value GreaterEqualsIntegers(std::vector<Value>& params); //bool(int,int)
	Value EqualsIntegers(std::vector<Value>& params); //bool(int,int)
	Value LessEqualsIntegers(std::vector<Value>& params); //bool(int,int)
	Value LessThanIntegers(std::vector<Value>& params); //bool(int,int)



private:
	Value EvaluateExpression(ParameterisedExpression& expr);
	bool ExecuteAction(ParameterisedAction& action);

};




}
}
