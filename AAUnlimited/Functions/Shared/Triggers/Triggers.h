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

	class GlobalVariable {
	public:
		DWORD id;
		Types type;
		std::wstring name;
		Value defaultValue;
		Value currentValue;
		bool initialized;

		inline GlobalVariable() : initialized(false) {}
		GlobalVariable(Variable& var);
	};

	class Trigger {
	public:
		std::wstring name;
		std::vector<ParameterisedEvent> events;
		std::vector<Variable> vars;
		struct GUIAction {
			ParameterisedAction action;
			std::vector<GUIAction*> subactions; 
			GUIAction* parent;

			//overload these to keep parent consitent
			GUIAction() = default;
			~GUIAction();
			GUIAction(const GUIAction& rhs);
			GUIAction(GUIAction&& rhs);
			GUIAction& operator=(const GUIAction& rhs);
			GUIAction& operator==(GUIAction&& rhs);
			void FixParents();
		};
		std::vector<GUIAction*> guiActions; //a hirachical version of  the actions used in the gui view

		std::vector<ParameterisedAction> actions;

		std::vector<GlobalVariable>* globalVars;
		int owningCard;

		void Initialize(std::vector<GlobalVariable>* globals, int owningCard);	//has to be called on an instance before a thread can execute it

		static const int INSERT_START = -1;
		static const int INSERT_END = -2;
		//void InsertAction(const ParameterisedAction& action, GUIAction* insertAfter); currently handled manually by the gui
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

		struct AddActionsFromGuiActions_State {
			int ifStartJump = -1;
			int ifEndJump = -1;
			std::vector<std::pair<int,int>> elseIfLabels;
			int elseJump = -1;
			inline void ResetITEState() { ifStartJump = ifEndJump = elseJump = -1; elseIfLabels.clear(); }

			std::vector<int>* loopBreaks = NULL;
			std::vector<int>* loopContinues = NULL;
		};
		void AddActionsFromGuiActions(std::vector<GUIAction*>& guiActions,AddActionsFromGuiActions_State state);
	};
	


};
};