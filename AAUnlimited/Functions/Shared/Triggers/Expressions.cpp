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
	return this->eventData->card;
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

/*
 * Event Response
 */

//bool()
Value Thread::GetNpcResponseOriginalAnswer(std::vector<Value>& params) {
	if (this->eventData->GetId() != NPC_RESPONSE) return false;
	return ((NpcResponseData*)eventData)->originalResponse;
}

//bool()
Value Thread::GetNpcResponseCurrentAnswer(std::vector<Value>& params) {
	if (this->eventData->GetId() != NPC_RESPONSE) return false;
	return ((NpcResponseData*)eventData)->changedResponse;
}

//int()
Value Thread::GetNpcResponseTarget(std::vector<Value>& params) {
	if (this->eventData->GetId() != NPC_RESPONSE) return 0;
	return ((NpcResponseData*)eventData)->answeredTowards;
}

//int()
Value Thread::GetNpcResponseConversation(std::vector<Value>& params) {
	if (this->eventData->GetId() != NPC_RESPONSE) return 0;
	return ((NpcResponseData*)eventData)->conversationId;
}

//int()
Value Thread::GetNpcResponseOriginalPercent(std::vector<Value>& params) {
	if (this->eventData->GetId() != NPC_RESPONSE) return 0;
	return ((NpcResponseData*)eventData)->originalChance;
}

//int()
Value Thread::GetNpcResponseCurrentPercent(std::vector<Value>& params) {
	if (this->eventData->GetId() != NPC_RESPONSE) return 0;
	return ((NpcResponseData*)eventData)->changedChance;
}

//NPC_WALK_TO_ROOM
//int()
Value Thread::GetNpcRoomTarget(std::vector<Value>& params) {
	if (this->eventData->GetId() != NPC_RESPONSE) return 0;
	return ((NpcResponseData*)eventData)->changedChance;
}

//int()
Value Thread::GetNpcActionId(std::vector<Value>& params) {
	switch(this->eventData->GetId()) {
	case NPC_WANT_ACTION_NOTARGET:
		return ((NpcWantActionNoTargetData*)eventData)->action;
	case NPC_WANT_TALK_WITH:
		return ((NpcWantTalkWithData*)eventData)->action;
	case NPC_WANT_TALK_WITH_ABOUT:
		return ((NpcWantTalkWithAboutData*)eventData)->action;
		break;
	default:
		return 0;
		break;
	}
}


//int()
Value Thread::GetNpcTalkTarget(std::vector<Value>& params) {
	switch (this->eventData->GetId()) {
	case NPC_WANT_TALK_WITH:
		return ((NpcWantTalkWithData*)eventData)->conversationTarget;
	case NPC_WANT_TALK_WITH_ABOUT:
		return ((NpcWantTalkWithAboutData*)eventData)->conversationTarget;
		break;
	default:
		return 0;
		break;
	}
}


//int()
Value Thread::GetNpcTalkAbout(std::vector<Value>& params) {
	switch (this->eventData->GetId()) {
	case NPC_WANT_TALK_WITH_ABOUT:
		return ((NpcWantTalkWithAboutData*)eventData)->conversationAbout;
		break;
	default:
		return 0;
		break;
	}
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
	{ //INT
		{ 
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
			TEXT("Triggering Card"), TEXT("Get Triggering Card"), TEXT("The card that triggered the event that caused the trigger to run. "
			"Used by many events."),
			{  }, (TYPE_INT),
			&Thread::GetTriggeringCard
		},
		{
			10, EXPRCAT_EVENT,
			TEXT("This Card"), TEXT("Get This Card"), TEXT("The card to whom this trigger belongs to"),
			{}, (TYPE_INT),
			&Thread::GetThisCard
		},
		{
			11, EXPRCAT_EVENT,
			TEXT("Answered Character"), TEXT("Get Answer Target"), TEXT("In a NPC Answered event, the character the NPC answered to"),
			{}, (TYPE_INT),
			&Thread::GetNpcResponseTarget
		},
		{
			12, EXPRCAT_EVENT,
			TEXT("Answered Conversation"), TEXT("Get Answered Conversation"), TEXT("The Type of Question the NPC answered in a NPC Answered event."),
			{}, (TYPE_INT),
			&Thread::GetNpcResponseConversation
		},
		{
			13, EXPRCAT_EVENT,
			TEXT("Npc Room Target"), TEXT("Get Npc Room Target"), TEXT("Room that the Npc Walks to in a Npc Walks to Room event."),
			{}, (TYPE_INT),
			&Thread::GetNpcRoomTarget
		},
		{
			14, EXPRCAT_EVENT,
			TEXT("Npc Action Id"), TEXT("Get Action Id"), TEXT("The Type of Action an Npc Wants to Perform in a no-target-action event, or the conversation "
			"id in in targeted actions"),
			{}, (TYPE_INT),
			&Thread::GetNpcActionId
		},
		{
			15, EXPRCAT_EVENT,
			TEXT("Npc Talk Target"), TEXT("Npc Talk Target"), TEXT("In a Npc Talk With, or Npc Talk With About event, this is the character the Npc talks with."),
			{}, (TYPE_INT),
			&Thread::GetNpcTalkTarget
		},
		{
			16, EXPRCAT_EVENT,
			TEXT("Npc Talk About"), TEXT("Npc Talk About"), TEXT("In a Npc Talk With About event, this is the character the Npc talks about."),
			{}, (TYPE_INT),
			&Thread::GetNpcTalkAbout
		},
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
		},
		{
			14, EXPRCAT_EVENT,
			TEXT("Npc Original Answer"), TEXT("Get Npc Original Answer"), 
			TEXT("If executed in a trigger with the Npc Answers Event, this is the original Answer the NPC made"),
			{ }, (TYPE_BOOL),
			&Thread::GetNpcResponseOriginalAnswer
		},
		{
			15, EXPRCAT_EVENT,
			TEXT("Npc Current Answer"), TEXT("Get Npc Current Answer"),
			TEXT("If executed in a trigger with the Npc Answers Event, this is the current Answer, modified by this or previously executed Triggers. "
			"using the Set Npc Response Answer Action"),
			{ }, (TYPE_BOOL),
			&Thread::GetNpcResponseCurrentAnswer
		},
		{
			16, EXPRCAT_EVENT,
			TEXT("Npc Original Answer Chance"), TEXT("Get Npc Original Answer Percent"),
			TEXT("If executed in a trigger with the Npc Answers Event, this is success Chance that the Interaction had in Percent"),
			{}, (TYPE_BOOL),
			&Thread::GetNpcResponseOriginalPercent
		},
		{
			17, EXPRCAT_EVENT,
			TEXT("Npc Current Answer Chance"), TEXT("Get Npc Current Answer Percent"),
			TEXT("If executed in a trigger with the Npc Answers Event, this is the current Interaction Percent, modified by this or previously executed Triggers. "
			"using the Set Npc Response Percent Action"),
			{}, (TYPE_BOOL),
			&Thread::GetNpcResponseCurrentPercent
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