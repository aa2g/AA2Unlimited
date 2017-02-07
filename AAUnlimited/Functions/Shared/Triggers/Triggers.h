#pragma once

#include "Value.h"
#include "Actions.h"
#include "Expressions.h"
#include "Event.h"

#include <vector>
#include <string>
#include <Windows.h>

/*
 * A trigger system for cards, similar to wc3.
 * Each trigger consists of events, local variables, and actions.
 */



namespace Shared {
namespace Triggers {

	/*
	 * A Variable type.
	 */
	class Variable {
	public:
		DWORD id;
		Types type;
		std::wstring name;
		ParameterisedExpression defaultValue;
	};

	class VariableInstance {
	public:
		Variable* variable;
		Value currValue;
	};

	class Trigger {
	public:
		std::wstring name;
		std::vector<ParameterisedEvent> events;
		std::vector<Variable> vars;
		std::vector<ParameterisedAction> actions;

		void Initialize();	//has to be called on an instance before a thread can execute it

		static const int INSERT_START = -1;
		static const int INSERT_END = -2;
		void InsertAction(const ParameterisedAction& action, int insertAfter);
		void InsertVariable(const Variable& var,int insertAfter);
		void InsertEvent(const ParameterisedEvent& event,int insertAfter);

		Variable* FindVar(std::wstring name);

		inline bool IsInitalized() { return bInitialized; }
		inline bool IsBroken() { return broken; }

		Trigger() : bInitialized(false) {}
		~Trigger();
	private:
		bool broken;
		bool bInitialized;
	};
	


};
};