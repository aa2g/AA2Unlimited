#pragma once

#include "Value.h"
#include "Expressions.h"
#include "Actions.h"
#include "Triggers.h"
#include "Event.h"

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
	} localStorage;

	EventData* eventData;

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
	void ShouldNotBeImplemented(std::vector<Value>& params);

	void ConditionalJump(std::vector<Value>& params);
	void EndExecution(std::vector<Value>& params);
	void ConditionalEndExecution(std::vector<Value>& params);

	void SwitchAAUDataSet(std::vector<Value>& params);
	void AddCardLovePoints(std::vector<Value>& params);
	void AddCardLikePoints(std::vector<Value>& params);
	void AddCardDislikePoints(std::vector<Value>& params);
	void AddCardHatePoints(std::vector<Value>& params);
	void AddCardPoints(std::vector<Value>& params);

	//event response
	void SetNpcResponseAnswer(std::vector<Value>& params);

	///////////////////////
	// Expressions

	Value GetTriggeringCard(std::vector<Value>& params);	//int ()
	Value GetThisCard(std::vector<Value>& params); //int ()

	Value GetCardFirstName(std::vector<Value>& params); //string(int)
	Value GetCardSecondName(std::vector<Value>& params); //string(int)

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
	Value BoolNot(std::vector<Value>& params); //bool(bool)
	Value GreaterThanIntegers(std::vector<Value>& params); //bool(int,int)
	Value GreaterEqualsIntegers(std::vector<Value>& params); //bool(int,int)
	Value EqualsIntegers(std::vector<Value>& params); //bool(int,int)
	Value NotEqualsIntegers(std::vector<Value>& params); //bool(int,int)
	Value LessEqualsIntegers(std::vector<Value>& params); //bool(int,int)
	Value LessThanIntegers(std::vector<Value>& params); //bool(int,int)
	Value EqualsStrings(std::vector<Value>& params); //bool(string,string)

	//basic string stuff
	Value SubString(std::vector<Value>& params); //string(string, int, int)

	//Event Respone
	//NPC_RESPONSE
	Value GetNpcResponseOriginalAnswer(std::vector<Value>& params); //bool()
	Value GetNpcResponseCurrentAnswer(std::vector<Value>& params);	//bool()
	Value GetNpcResponseTarget(std::vector<Value>& params);	//int()

private:
	Value EvaluateExpression(ParameterisedExpression& expr);
	bool ExecuteAction(ParameterisedAction& action);

};




}
}
