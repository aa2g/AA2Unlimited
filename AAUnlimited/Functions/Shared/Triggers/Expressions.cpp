#include "Expressions.h"

#include "Thread.h"

namespace Shared {
namespace Triggers {

/*
 * List of possible Expressions
 */

/*
 * Int stuff
 */

//int (int min, int max)
Value Thread::GetRandomInt(std::vector<Value>& params) {
	int range = params[1].iVal - params[0].iVal + 1;
	int r = rand() % range + params[0].iVal;
	return Value(r);
}

Value Thread::AddIntegers(std::vector<Value>& params) {
	return Value(params[0].iVal + params[1].iVal);
}

Value Thread::SubstractIntegers(std::vector<Value>& params) {
	return Value(params[0].iVal - params[1].iVal);
}
//int(int,int)
Value Thread::DivideIntegers(std::vector<Value>& params) {
	return Value(params[0].iVal / params[1].iVal);
}
//int(int,int)
Value Thread::MultiplyIntegers(std::vector<Value>& params) {
	return Value(params[0].iVal * params[1].iVal);
}

/*
* Bool stuff
*/


//bool(int,int)
Value Thread::GreaterThanIntegers(std::vector<Value>& params) {
	return params[0].iVal > params[1].iVal;
}
//bool(int,int)
Value Thread::GreaterEqualsIntegers(std::vector<Value>& params) {
	return params[0].iVal >= params[1].iVal;
}
//bool(int,int)
Value Thread::EqualsIntegers(std::vector<Value>& params) {
	return params[0].iVal == params[1].iVal;
}
//bool(int,int)
Value Thread::LessEqualsIntegers(std::vector<Value>& params) {
	return params[0].iVal <= params[1].iVal;
}
//bool(int,int)
Value Thread::LessThanIntegers(std::vector<Value>& params) {
	return params[0].iVal < params[1].iVal;
}

Value Thread::GetTriggeringCard(std::vector<Value>&) {
	return this->localStorage.triggeringCard;
}

//int ()
Value Thread::GetThisCard(std::vector<Value>& params) {
	return this->thisCard;
}




/*
 * Note that the first two expressions for each Type are handled specially as they represent variables and constants.
 */
std::vector<Expression> g_Expressions[N_TYPES] = {
	{ //INVALID
		{
			1,
			TEXT("Constant"), TEXT("An arbitrary constant to input"),
			{}, (TYPE_INVALID),
			NULL
		},
		{
			2,
			TEXT("Variable"), TEXT("A Variable"),
			{}, (TYPE_INVALID),
			NULL
		},
	},
	{
		{ //INT
			1,
			TEXT("Constant"), TEXT("An arbitrary constant to input"),
			{ }, (TYPE_INT),
			NULL
		},
		{
			2,
			TEXT("Variable"), TEXT("A Variable"),
			{ }, (TYPE_INT),
			NULL
		},
		{
			3,
			TEXT("Get Random Int between %p and %p"), TEXT("Generates a random integer between the two arguments (including both)"),
			{(TYPE_INT), (TYPE_INT)}, (TYPE_INT),
			&Thread::GetRandomInt
		},
		{
			4,
			TEXT("%p + %p"), TEXT("Adds two integers"),
			{ (TYPE_INT), (TYPE_INT) }, (TYPE_INT),
			&Thread::AddIntegers
		},
		{
			5,
			TEXT("%p - %p"), TEXT("Substracts two integers"),
			{ (TYPE_INT), (TYPE_INT) }, (TYPE_INT),
			&Thread::SubstractIntegers
		},
		{
			6,
			TEXT("%p / %p"), TEXT("Divide two integers"),
			{ (TYPE_INT), (TYPE_INT) }, (TYPE_INT),
			&Thread::DivideIntegers
		},
		{
			7,
			TEXT("%p * %p"), TEXT("Multiply two integers"),
			{ (TYPE_INT), (TYPE_INT) }, (TYPE_INT),
			&Thread::MultiplyIntegers
		},
		{
			8,
			TEXT("Get Triggering Card"), TEXT("The card that triggered the event that caused the trigger to run"),
			{  }, (TYPE_INT),
			&Thread::GetTriggeringCard
		},
		{
			9,
			TEXT("Get This Card"), TEXT("The card to whom this trigger belongs to"),
			{}, (TYPE_INT),
			&Thread::GetTriggeringCard
		}
	},

	{ //BOOL
		{
			1,
			TEXT("Constant"), TEXT("An arbitrary constant to input"),
			{}, (TYPE_BOOL),
			NULL
		},
		{
			2,
			TEXT("Variable"), TEXT("A Variable"),
			{}, (TYPE_BOOL),
			NULL
		},
		{
			3,
			TEXT("%p && %p"), TEXT("Logical and, including short-circut evaluation"),
			{ TYPE_BOOL, TYPE_BOOL }, (TYPE_BOOL),
			NULL
		},
		{
			4,
			TEXT("%p || %p"), TEXT("Logical or, including short-circut evaluation"),
			{ TYPE_BOOL, TYPE_BOOL }, (TYPE_BOOL),
			NULL
		},
		{
			5,
			TEXT("(int)%p > %p"), TEXT("Greater-Than"),
			{ TYPE_INT, TYPE_INT }, (TYPE_BOOL),
			&Thread::GreaterThanIntegers
		},
		{
			6,
			TEXT("(int)%p >= %p"), TEXT("Greater-Than or equal"),
			{ TYPE_INT, TYPE_INT }, (TYPE_BOOL),
			&Thread::GreaterEqualsIntegers
		},
		{
			7,
			TEXT("(int)%p == %p"), TEXT("Equal"),
			{ TYPE_INT, TYPE_INT }, (TYPE_BOOL),
			&Thread::EqualsIntegers
		},
		{
			8,
			TEXT("(int)%p <= %p"), TEXT("less than or equal"),
			{ TYPE_INT, TYPE_INT }, (TYPE_BOOL),
			&Thread::LessEqualsIntegers
		},
		{
			9,
			TEXT("(int)%p < %p"), TEXT("less than"),
			{ TYPE_INT, TYPE_INT }, (TYPE_BOOL),
			&Thread::LessThanIntegers
		}
	},
	{ //FLOAT
		{
			1,
			TEXT("Constant"), TEXT("An arbitrary constant to input"),
			{}, (TYPE_FLOAT),
			NULL
		},
		{
			2,
			TEXT("Variable"), TEXT("A Variable"),
			{}, (TYPE_FLOAT),
			NULL
		},
	},
	{ //STRING
		{
			1,
			TEXT("Constant"), TEXT("An arbitrary constant to input"),
			{}, (TYPE_STRING),
			NULL
		},
		{
			2,
			TEXT("Variable"), TEXT("A Variable"),
			{}, (TYPE_STRING),
			NULL
		},
	}

};







}
}