#include "Expressions.h"

#include "Thread.h"
#include "Functions\AAPlay\Globals.h"

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

Value Thread::GetTriggeringCard(std::vector<Value>&) {
	return this->localStorage.triggeringCard;
}

//int ()
Value Thread::GetThisCard(std::vector<Value>& params) {
	return this->thisCard;
}

/*
* Bool stuff
*/

//bool(bool)
Value Thread::BoolNot(std::vector<Value>& params) {
	return !params[0].bVal;
}

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
Value Thread::NotEqualsIntegers(std::vector<Value>& params) {
	return params[0].iVal != params[1].iVal;
}
//bool(int,int)
Value Thread::LessEqualsIntegers(std::vector<Value>& params) {
	return params[0].iVal <= params[1].iVal;
}
//bool(int,int)
Value Thread::LessThanIntegers(std::vector<Value>& params) {
	return params[0].iVal < params[1].iVal;
}

//bool(string,string)
Value Thread::EqualsStrings(std::vector<Value>& params) {
	return *(params[0].strVal) == *(params[1].strVal);
}

/*
 * string stuff
 */

 //string(string, int, int)
Value Thread::SubString(std::vector<Value>& params) {
	std::wstring& str = *params[0].strVal;
	int from = params[1].iVal;
	if (from < 0) from = 0;
	int length = params[2].iVal;
	if (length < 0) length = 0;

	return Value(str.substr(from,length));
}

//string(int)
Value Thread::GetCardFirstName(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	if (cardInst == NULL) return Value(TEXT(""));

	return Value(cardInst->m_char->m_charData->m_forename);
}

//string(int)
Value Thread::GetCardSecondName(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	if (cardInst == NULL) return Value(TEXT(""));

	return Value(cardInst->m_char->m_charData->m_surname);
}

std::wstring g_ExpressionCategories[EXPRCAT_N] = {
	TEXT("General"),
	TEXT("Event Response"),
	TEXT("Math"),
	TEXT("Character Property"),
	TEXT("Comparision - Int"),
	TEXT("Comparision - String"),
	TEXT("Comparision - Bool")
};


/*
 * Note that the first two expressions for each Type are handled specially as they represent variables and constants.
 */
std::vector<Expression> g_Expressions[N_TYPES] = {
	{ //INVALID
		{
			EXPR_CONSTANT, EXPRCAT_GENERAL,
			TEXT("Constant"), TEXT("Constant"), TEXT("An arbitrary constant to input"),
			{}, (TYPE_INVALID),
			NULL
		},
		{
			EXPR_VAR, EXPRCAT_GENERAL,
			TEXT("Variable"), TEXT("Variable"), TEXT("A Variable"),
			{}, (TYPE_INVALID),
			NULL
		},
		{
			EXPR_NAMEDCONSTANT, EXPRCAT_GENERAL,
			TEXT("Named Constant"), TEXT("Named Constant"), TEXT("A known constant with a name"),
			{}, (TYPE_INVALID),
			NULL
		},
	},
	{
		{ //INT
			EXPR_CONSTANT, EXPRCAT_GENERAL,
			TEXT("Constant"), TEXT("Constant"), TEXT("An arbitrary constant to input"),
			{ }, (TYPE_INT),
			NULL
		},
		{
			EXPR_VAR, EXPRCAT_GENERAL,
			TEXT("Variable"), TEXT("Variable"), TEXT("A Variable"),
			{ }, (TYPE_INT),
			NULL
		},
		{
			EXPR_NAMEDCONSTANT, EXPRCAT_GENERAL,
			TEXT("Named Constant"), TEXT("Named Constant"), TEXT("A known constant with a name"),
			{}, (TYPE_INT),
			NULL
		},
		{
			4, EXPRCAT_MATH,
			TEXT("Random Int"), TEXT("Get Random Int between %p and %p"), TEXT("Generates a random integer between the two arguments (including both)"),
			{(TYPE_INT), (TYPE_INT)}, (TYPE_INT),
			&Thread::GetRandomInt
		},
		{
			EXPR_INT_PLUS, EXPRCAT_MATH,
			TEXT("+"), TEXT("%p + %p"), TEXT("Adds two integers"),
			{ (TYPE_INT), (TYPE_INT) }, (TYPE_INT),
			&Thread::AddIntegers
		},
		{
			6, EXPRCAT_MATH,
			TEXT("-"), TEXT("%p - %p"), TEXT("Substracts two integers"),
			{ (TYPE_INT), (TYPE_INT) }, (TYPE_INT),
			&Thread::SubstractIntegers
		},
		{
			7, EXPRCAT_MATH,
			TEXT("/"), TEXT("%p / %p"), TEXT("Divide two integers"),
			{ (TYPE_INT), (TYPE_INT) }, (TYPE_INT),
			&Thread::DivideIntegers
		},
		{
			8, EXPRCAT_MATH,
			TEXT("*"), TEXT("%p * %p"), TEXT("Multiply two integers"),
			{ (TYPE_INT), (TYPE_INT) }, (TYPE_INT),
			&Thread::MultiplyIntegers
		},
		{
			9, EXPRCAT_EVENT,
			TEXT("Triggering Card"), TEXT("Get Triggering Card"), TEXT("The card that triggered the event that caused the trigger to run"),
			{  }, (TYPE_INT),
			&Thread::GetTriggeringCard
		},
		{
			10, EXPRCAT_EVENT,
			TEXT("This Card"), TEXT("Get This Card"), TEXT("The card to whom this trigger belongs to"),
			{}, (TYPE_INT),
			&Thread::GetTriggeringCard
		}
	},

	{ //BOOL
		{
			EXPR_CONSTANT, EXPRCAT_GENERAL,
			TEXT("Constant"), TEXT("Constant"), TEXT("An arbitrary constant to input"),
			{}, (TYPE_BOOL),
			NULL
		},
		{
			EXPR_VAR, EXPRCAT_GENERAL,
			TEXT("Variable"), TEXT("Variable"), TEXT("A Variable"),
			{}, (TYPE_BOOL),
			NULL
		},
		{
			EXPR_NAMEDCONSTANT, EXPRCAT_GENERAL,
			TEXT("Named Constant"), TEXT("Named Constant"), TEXT("A known constant with a name"),
			{}, (TYPE_BOOL),
			NULL
		},
		{
			4, EXPRCAT_COMPARISION_BOOL,
			TEXT("Logical And"), TEXT("%p && %p"), TEXT("Logical and, including short-circut evaluation"),
			{ TYPE_BOOL, TYPE_BOOL }, (TYPE_BOOL),
			NULL
		},
		{
			5, EXPRCAT_COMPARISION_BOOL,
			TEXT("Logical Or"),TEXT("%p || %p"), TEXT("Logical or, including short-circut evaluation"),
			{ TYPE_BOOL, TYPE_BOOL }, (TYPE_BOOL),
			NULL
		},
		{
			6, EXPRCAT_COMPARISION_INT,
			TEXT("Greater Than"), TEXT("%p > %p"), TEXT("Greater-Than"),
			{ TYPE_INT, TYPE_INT }, (TYPE_BOOL),
			&Thread::GreaterThanIntegers
		},
		{
			EXPR_BOOL_GTE_INT, EXPRCAT_COMPARISION_INT,
			TEXT("Greater Than or Equal"), TEXT("%p >= %p"), TEXT("Greater-Than or equal"),
			{ TYPE_INT, TYPE_INT }, (TYPE_BOOL),
			&Thread::GreaterEqualsIntegers
		},
		{
			8, EXPRCAT_COMPARISION_INT,
			TEXT("Equal"), TEXT("%p == %p"), TEXT("Equal"),
			{ TYPE_INT, TYPE_INT }, (TYPE_BOOL),
			&Thread::EqualsIntegers
		},
		{
			9, EXPRCAT_COMPARISION_INT,
			TEXT("Less Than or Equal"), TEXT("%p <= %p"), TEXT("less than or equal"),
			{ TYPE_INT, TYPE_INT }, (TYPE_BOOL),
			&Thread::LessEqualsIntegers
		},
		{
			10, EXPRCAT_COMPARISION_INT,
			TEXT("Less Than"), TEXT("%p < %p"), TEXT("less than"),
			{ TYPE_INT, TYPE_INT }, (TYPE_BOOL),
			&Thread::LessThanIntegers
		},
		{
			EXPR_BOOL_NOT, EXPRCAT_COMPARISION_BOOL,
			TEXT("Not"), TEXT("!"), TEXT("Logical Not"),
			{ TYPE_BOOL }, (TYPE_BOOL),
			&Thread::BoolNot
		},
		{
			12, EXPRCAT_COMPARISION_STRING,
			TEXT("String - Equal"), TEXT("%p == %p"), TEXT("Compares two strings"),
			{ TYPE_STRING, TYPE_STRING }, (TYPE_BOOL),
			&Thread::EqualsStrings
		},
		{
			13, EXPRCAT_COMPARISION_INT,
			TEXT("Not Equal"), TEXT("%p != %p"), TEXT("Not Equal"),
			{ TYPE_INT, TYPE_INT }, (TYPE_BOOL),
			&Thread::NotEqualsIntegers
		}
				
	},
	{ //FLOAT
		{
			EXPR_CONSTANT, EXPRCAT_GENERAL,
			TEXT("Constant"), TEXT("Constant"), TEXT("An arbitrary constant to input"),
			{}, (TYPE_FLOAT),
			NULL
		},
		{
			EXPR_VAR, EXPRCAT_GENERAL,
			TEXT("Variable"), TEXT("Variable"), TEXT("A Variable"),
			{}, (TYPE_FLOAT),
			NULL
		},
		{
			EXPR_NAMEDCONSTANT, EXPRCAT_GENERAL,
			TEXT("Named Constant"), TEXT("Named Constant"), TEXT("A known constant with a name"),
			{}, (TYPE_FLOAT),
			NULL
		},
	},
	{ //STRING
		{
			EXPR_CONSTANT, EXPRCAT_GENERAL,
			TEXT("Constant"), TEXT("Constant"), TEXT("An arbitrary constant to input"),
			{}, (TYPE_STRING),
			NULL
		},
		{
			EXPR_VAR, EXPRCAT_GENERAL,
			TEXT("Variable"), TEXT("Variable"), TEXT("A Variable"),
			{}, (TYPE_STRING),
			NULL
		},
		{
			EXPR_NAMEDCONSTANT, EXPRCAT_GENERAL,
			TEXT("Named Constant"), TEXT("Named Constant"), TEXT("A known constant with a name"),
			{}, (TYPE_STRING),
			NULL
		},
		{
			4, EXPRCAT_GENERAL,
			TEXT("Substring"), TEXT("Substring of %p from %p with length %p"), TEXT("A substring that starts at the first parameter (inclusive) and has "
			"a specific length"),
			{TYPE_STRING, TYPE_INT, TYPE_INT}, (TYPE_STRING),
			&Thread::SubString
		},
		{
			5, EXPRCAT_CHARPROP,
			TEXT("First Name"), TEXT("First Name of %p"), TEXT("The first name this character was given (the upper one on the default card image). "
			"Note that this may or may not be the family name depending on how the card maker ordered these."),
			{  TYPE_INT }, (TYPE_STRING),
				&Thread::SubString
		},
		{
			6, EXPRCAT_CHARPROP,
			TEXT("Second Name"), TEXT("Second Name of %p"), TEXT("The first name this character was given (the upper one on the default card image). "
			"Note that this may or may not be the family name depending on how the card maker ordered these."),
			{ TYPE_INT }, (TYPE_STRING),
				&Thread::SubString
		},
	}

};



ParameterisedExpression::ParameterisedExpression(Types type, DWORD exprId,const std::vector<ParameterisedExpression>& params) : ParameterisedExpression() {
	this->expression = Expression::FromId(type,exprId);
	this->actualParameters = params;
}
ParameterisedExpression::ParameterisedExpression(Types type, Value constant) : ParameterisedExpression() {
	this->expression = Expression::FromId(type,EXPR_CONSTANT);
	this->constant = constant;
}
ParameterisedExpression::ParameterisedExpression(Types type, std::wstring var) : ParameterisedExpression() {
	this->expression = Expression::FromId(type,EXPR_VAR);
	this->varName = var;
}



}
}