#include "Actions.h"

#include "Thread.h"

#include "Files\Logger.h"
#include "Functions\AAPlay\Globals.h"

namespace Shared {
namespace Triggers {

//int offset, bool condition
void Thread::ConditionalJump(std::vector<Value>& params) {
	int offset = params[0].iVal;
	bool cond = params[1].bVal;
	if (!cond) return;
	this->ip += offset;
}

//void
void Thread::EndExecution(std::vector<Value>& params) {
	this->execFinished = true;
}


//int seat, int newseat
void Thread::SwitchAAUDataSet(std::vector<Value>& params) {
	int seat = params[0].iVal;
	int newset = params[1].iVal;
	if(!AAPlay::g_characters[seat].m_char) {
		LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
		return;
	}
	auto& aau = AAPlay::g_characters[seat].m_cardData;
	aau.SwitchActiveAAUDataSet(newset);
}







	/*
	* A list of all actions available.
	*/
	std::vector<Action> g_Actions = {
		{
			1, TEXT("Set Variable %p = %p"), TEXT("Sets the value of a given variable to a new one"),{ TYPE_INVALID, TYPE_INVALID },
			NULL
		},
		{ 
			2, TEXT("Switch %p 's AAU Data Set to %p"), TEXT("TODO: add description"),{ TYPE_INT, TYPE_INT },
			&Thread::SwitchAAUDataSet
		},
		{
			3, TEXT("Jump %p Actions if %p"), 
			TEXT("Skips additional actions if the given condition is true. May skip negative amounts to go back."), 
			{ TYPE_INT, TYPE_BOOL },
			&Thread::ConditionalJump
		},
		{
			4, TEXT("End Execution"), TEXT("ends execution of this thread. think of a return statement."),{ },
			&Thread::EndExecution
		}

	};


}
}