#include "Expressions.h"

#include "Thread.h"
#include "Functions\AAPlay\Globals.h"
#include "General\Util.h"

namespace Shared {
namespace Triggers {

/*
 * List of possible Expressions
 */

Value Thread::GetTriggeringCard(std::vector<Value>&) {
	return this->eventData->card;
}

//int ()
Value Thread::GetThisCard(std::vector<Value>& params) {
	return this->thisCard;
}

//bool (int)
Value Thread::IsSeatFilled(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	return Value(cardInst->IsValid());
}

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

Value Thread::Float2Int(std::vector<Value>& params) {
	int i = params[0].fVal;
	return Value(i);
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
//bool(float,float)
Value Thread::GreaterThanFloats(std::vector<Value>& params) {
	return params[0].fVal > params[1].fVal;
}
//bool(float,float)
Value Thread::GreaterEqualsFloats(std::vector<Value>& params) {
	return params[0].fVal >= params[1].fVal;
} 
//bool(float,float)
Value Thread::EqualsFloats(std::vector<Value>& params) {
	return params[0].fVal == params[1].fVal;
}
//bool(float,float)
Value Thread::NotEqualsFloats(std::vector<Value>& params) {
	return params[0].fVal != params[1].fVal;
}
//bool(float,float)
Value Thread::LessEqualsFloats(std::vector<Value>& params) {
	return params[0].fVal <= params[1].fVal;
}
//bool(float,float)
Value Thread::LessThanFloats(std::vector<Value>& params) {
	return params[0].fVal < params[1].fVal;
}

/*
 * float stuff
 */

 //float (float min, float max)
Value Thread::GetRandomFloat(std::vector<Value>& params) {
	return Value(General::GetRandomFloat(params[0].fVal, params[1].fVal));
}

Value Thread::AddFloats(std::vector<Value>& params) {
	return Value(params[0].fVal + params[1].fVal);
}

Value Thread::SubstractFloats(std::vector<Value>& params) {
	return Value(params[0].fVal - params[1].fVal);
}

Value Thread::DivideFloats(std::vector<Value>& params) {
	return Value(params[0].fVal / params[1].fVal);
}

Value Thread::MultiplyFloats(std::vector<Value>& params) {
	return Value(params[0].fVal * params[1].fVal);
}

Value Thread::Int2Float(std::vector<Value>& params) {
	float v = params[0].iVal;
	return Value(v);
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

/*
 * card attributes
 */
//int(int)
Value Thread::GetCardVirtue(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	if (cardInst == NULL) return Value(TEXT(""));

	return Value(cardInst->m_char->m_charData->m_character.virtue);
}

//bool(int, int)
Value Thread::GetCardTrait(std::vector<Value>& params) {
	int card = params[0].iVal;
	int trait = params[1].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	if (cardInst == NULL) return Value(TEXT(""));

	return Value(cardInst->m_char->m_charData->m_traitBools[trait]);
}

//int(int)
Value Thread::GetCardPersonality(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	if (cardInst == NULL) return Value(TEXT(""));

	return Value(cardInst->m_char->m_charData->m_bPersonality);
}

//int(int)
Value Thread::GetCardVoicePitch(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	if (cardInst == NULL) return Value(TEXT(""));

	return Value(cardInst->m_char->m_charData->m_voicePitch);
}

//int(int)
Value Thread::GetCardClub(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	if (cardInst == NULL) return Value(TEXT(""));

	return Value(cardInst->m_char->m_charData->m_club);
}

//int(int)
Value Thread::GetCardClubValue(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	if (cardInst == NULL) return Value(TEXT(""));

	return Value((int)cardInst->m_char->m_charData->m_character.clubValue);
}

//int(int)
Value Thread::GetCardClubRank(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	if (cardInst == NULL) return Value(TEXT(""));

	return Value(cardInst->m_char->m_charData->m_character.clubClassRanking);
}

//int(int)
Value Thread::GetCardIntelligence(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	if (cardInst == NULL) return Value(TEXT(""));

	return Value(cardInst->m_char->m_charData->m_character.intelligence);
}

//int(int)
Value Thread::GetCardIntelligenceValue(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	if (cardInst == NULL) return Value(TEXT(""));

	return Value((int)cardInst->m_char->m_charData->m_character.intelligenceValue);
}

//int(int)
Value Thread::GetCardIntelligenceRank(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	if (cardInst == NULL) return Value(TEXT(""));

	return Value(cardInst->m_char->m_charData->m_character.intelligenceClassRank);
}

//int(int)
Value Thread::GetCardStrength(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	if (cardInst == NULL) return Value(TEXT(""));

	return Value(cardInst->m_char->m_charData->m_character.strength);
}

//int(int)
Value Thread::GetCardStrengthValue(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	if (cardInst == NULL) return Value(TEXT(""));

	return Value((int)cardInst->m_char->m_charData->m_character.strengthValue);
}

//int(int)
Value Thread::GetCardStrengthRank(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	if (cardInst == NULL) return Value(TEXT(""));

	return Value(cardInst->m_char->m_charData->m_character.strengthClassRank);
}
//int(int)
Value Thread::GetCardSociability(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	if (cardInst == NULL) return Value(TEXT(""));

	return Value(cardInst->m_char->m_charData->m_character.sociability);
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

//int(int)
Value Thread::GetCardPartnerCount(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	if (!cardInst->IsValid()) return Value(0);

	return Value((int)cardInst->m_char->m_hStats->m_partnerCount);

}

//int(int,int)
Value Thread::GetCardLovePoints(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	if (!cardInst->IsValid()) return Value(0);
	int cardTowards = params[1].iVal;
	CharInstData* towardsInst = &AAPlay::g_characters[cardTowards];
	if (!towardsInst->IsValid()) return Value(0);

	auto* relations = cardInst->m_char->GetRelations();
	auto* it = relations->m_start;
	for (it; it != relations->m_end; it++) {
		if (it->m_targetSeat == cardTowards) break;
	}
	if (it == relations->m_end) return Value(0);

	return Value(it->m_lovePoints + it->m_loveCount*30);

}

//int(int,int)
Value Thread::GetCardLikePoints(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	if (!cardInst->IsValid()) return Value(0);
	int cardTowards = params[1].iVal;
	CharInstData* towardsInst = &AAPlay::g_characters[cardTowards];
	if (!towardsInst->IsValid()) return Value(0);

	auto* relations = cardInst->m_char->GetRelations();
	auto* it = relations->m_start;
	for(it; it != relations->m_end; it++) {
		if (it->m_targetSeat == cardTowards) break;
	}
	if (it == relations->m_end) return Value(0);

	return Value(it->m_likePoints + it->m_likeCount*30);
}

//int(int,int)
Value Thread::GetCardDislikePoints(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	if (!cardInst->IsValid()) return Value(0);
	int cardTowards = params[1].iVal;
	CharInstData* towardsInst = &AAPlay::g_characters[cardTowards];
	if (!towardsInst->IsValid()) return Value(0);

	auto* relations = cardInst->m_char->GetRelations();
	auto* it = relations->m_start;
	for (it; it != relations->m_end; it++) {
		if (it->m_targetSeat == cardTowards) break;
	}
	if (it == relations->m_end) return Value(0);

	return Value(it->m_dislikePoints + it->m_dislikeCount*30);
}

//int(int,int)
Value Thread::GetCardHatePoints(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	if (!cardInst->IsValid()) return Value(0);
	int cardTowards = params[1].iVal;
	CharInstData* towardsInst = &AAPlay::g_characters[cardTowards];
	if (!towardsInst->IsValid()) return Value(0);

	auto* relations = cardInst->m_char->GetRelations();
	auto* it = relations->m_start;
	for (it; it != relations->m_end; it++) {
		if (it->m_targetSeat == cardTowards) break;
	}
	if (it == relations->m_end) return Value(0);

	return Value(it->m_hatePoints + it->m_hateCount*30);
}

//bool(int,int)		 
Value Thread::IsLover(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* cardInst = &AAPlay::g_characters[card];
	if (!cardInst->IsValid()) return Value(0);
	int cardTowards = params[1].iVal;
	CharInstData* towardsInst = &AAPlay::g_characters[cardTowards];
	if (!towardsInst->IsValid()) return Value(0);

	//we just have to look if this struct exists at all
	auto* relations = cardInst->m_char->GetLovers();
	auto* it = relations->m_start;
	for (it; it != relations->m_end; it++) {
		if (it->m_targetSeat == cardTowards) break;
	}
	return Value(it == relations->m_end);
}

//int(int, string, int)
Value Thread::GetCardStorageInt(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* inst = &AAPlay::g_characters[card];
	if (!inst->IsValid()) return params[2];
	auto& storage = inst->m_cardData.GetCardStorage();
	auto it = storage.find(*params[1].strVal);
	if(it != storage.end() && it->second.type == TYPE_INT) {
		return it->second;
	}
	else {
		return params[2];
	}
}
//float(int, string, float)
Value Thread::GetCardStorageFloat(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* inst = &AAPlay::g_characters[card];
	if (!inst->IsValid()) return params[2];
	auto& storage = inst->m_cardData.GetCardStorage();
	auto it = storage.find(*params[1].strVal);
	if (it != storage.end() && it->second.type == TYPE_FLOAT) {
		return it->second;
	}
	else {
		return params[2];
	}
}
//string(int, string, string)
Value Thread::GetCardStorageString(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* inst = &AAPlay::g_characters[card];
	if (!inst->IsValid()) return params[2];
	auto& storage = inst->m_cardData.GetCardStorage();
	auto it = storage.find(*params[1].strVal);
	if (it != storage.end() && it->second.type == TYPE_STRING) {
		return it->second;
	}
	else {
		return params[2];
	}
}
//bool(int, string, bool)
Value Thread::GetCardStorageBool(std::vector<Value>& params) {
	int card = params[0].iVal;
	CharInstData* inst = &AAPlay::g_characters[card];
	if (!inst->IsValid()) return params[2];
	auto& storage = inst->m_cardData.GetCardStorage();
	auto it = storage.find(*params[1].strVal);
	if (it != storage.end() && it->second.type == TYPE_BOOL) {
		return it->second;
	}
	else {
		return params[2];
	}
}

/*
 * Event Response
 */

//PERIOD_ENDS
//int()
Value Thread::GetEndingPeriod(std::vector<Value>& params) {
	if (this->eventData->GetId() != PERIOD_ENDS) return Value(0);
	return ((PeriodEndsData*)eventData)->oldPeriod;
}

//int()
Value Thread::GetStartingPeriod(std::vector<Value>& params) {
	if (this->eventData->GetId() != PERIOD_ENDS) return Value(0);
	return ((PeriodEndsData*)eventData)->newPeriod;
}

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
	TEXT("Comparision - Bool"),
	TEXT("Comparision - Float")
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
			TEXT("Enumeration"), TEXT("Enumeration"), TEXT("A known constant with a name"),
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
			TEXT("Enumeration"), TEXT("Enumeration"), TEXT("A known constant with a name"),
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
		{
			17, EXPRCAT_EVENT,
			TEXT("Staring Period"), TEXT("Staring Period"), TEXT("In a Period Ends Event, this is the new period starting."),
			{}, (TYPE_INT),
			&Thread::GetStartingPeriod
		},
		{
			18, EXPRCAT_EVENT,
			TEXT("Ending Period"), TEXT("Ending Period"), TEXT("In a Period Ends Event, this is the old period that ended."),
			{}, (TYPE_INT),
			&Thread::GetEndingPeriod
		},
		{
			19, EXPRCAT_CHARPROP,
			TEXT("Love Points"), TEXT("Get %p 's Love Points towards %p"), TEXT("The total sum of love points. This includes the love history, "
			"where each entry translates to 30 points, but which are limited to 30 history entrys (=900 points) across all 4 categories, as well as "
			"single points, that have not added up to 30 and were therefor not converted to love history yet."),
			{ TYPE_INT, TYPE_INT }, (TYPE_INT),
			&Thread::GetCardLovePoints
		},
		{
			20, EXPRCAT_CHARPROP,
			TEXT("Like Points"), TEXT("Get %p 's Like Points towards %p"), TEXT("The total sum of like points. This includes the like history, "
			"where each entry translates to 30 points, but which are limited to 30 history entrys (=900 points) across all 4 categories, as well as "
			"single points, that have not added up to 30 and were therefor not converted to like history yet."),
			{ TYPE_INT, TYPE_INT }, (TYPE_INT),
			&Thread::GetCardLikePoints
		},
		{
			21, EXPRCAT_CHARPROP,
			TEXT("Dislike Points"), TEXT("Get %p 's Dislike Points towards %p"), TEXT("The total sum of dislike points. This includes the dislike history, "
			"where each entry translates to 30 points, but which are limited to 30 history entrys (=900 points) across all 4 categories, as well as "
			"single points, that have not added up to 30 and were therefor not converted to dislike history yet."),
			{ TYPE_INT, TYPE_INT }, (TYPE_INT),
			&Thread::GetCardDislikePoints
		},
		{
			22, EXPRCAT_CHARPROP,
			TEXT("Hate Points"), TEXT("Get %p 's Hate Points towards %p"), TEXT("The total sum of hate points. This includes the hate history, "
			"where each entry translates to 30 points, but which are limited to 30 history entrys (=900 points) across all 4 categories, as well as "
			"single points, that have not added up to 30 and were therefor not converted to hate history yet."),
			{ TYPE_INT, TYPE_INT }, (TYPE_INT),
			&Thread::GetCardHatePoints
		},
		{
			23, EXPRCAT_MATH,
			TEXT("Float to Int"), TEXT("Int ( %p )"), TEXT("Converts a Float to an Int by cutting off the decimals."),
			{ TYPE_FLOAT }, (TYPE_INT),
			&Thread::Float2Int
		},
		{
			24, EXPRCAT_CHARPROP,
			TEXT("Get Card Storage Int"), TEXT("Get %p 's Card Storage entry under %p , defaulting to %p on error"), 
			TEXT("Gets the integer from the given cards storage entry. If the entry doesnt exist or holds a value of a different type, "
			"it returns the default value instead"),
			{ TYPE_INT, TYPE_STRING, TYPE_INT }, (TYPE_INT),
			&Thread::GetCardStorageInt
		},
		{
			25, EXPRCAT_CHARPROP,
			TEXT("Virtue"), TEXT("Virtue of %p"), TEXT("The virtue of this character."),
			{ TYPE_INT }, (TYPE_INT),
			&Thread::GetCardVirtue
		},
		{
			26, EXPRCAT_CHARPROP,
			TEXT("Personality"), TEXT("%p 's personality"), TEXT("The personality of this character."),
			{ TYPE_INT }, (TYPE_INT),
			&Thread::GetCardPersonality
		},
		{
			27, EXPRCAT_CHARPROP,
			TEXT("Voice Pitch"), TEXT("%p 's pitch"), TEXT("The voice pitch of this character."),
			{ TYPE_INT }, (TYPE_INT),
			&Thread::GetCardVoicePitch
		},
		{
			28, EXPRCAT_CHARPROP,
			TEXT("Club"), TEXT("%p 's club"), TEXT("The club of this character."),
			{ TYPE_INT }, (TYPE_INT),
			&Thread::GetCardClub
		},
		{
			29, EXPRCAT_CHARPROP,
			TEXT("Club Value"), TEXT("%p 's club value"), TEXT("The club value of this character."),
			{ TYPE_INT }, (TYPE_INT),
			&Thread::GetCardClubValue
		},
		{
			30, EXPRCAT_CHARPROP,
			TEXT("Club Rank"), TEXT("%p 's club rank"), TEXT("The club rank of this character."),
			{ TYPE_INT }, (TYPE_INT),
			&Thread::GetCardClubRank
		},
		{
			31, EXPRCAT_CHARPROP,
			TEXT("Intelligence"), TEXT("%p 's intelligence"), TEXT("The intelligence of this character."),
			{ TYPE_INT }, (TYPE_INT),
			&Thread::GetCardIntelligence
		},
		{
			32, EXPRCAT_CHARPROP,
			TEXT("Intelligence Value"), TEXT("%p 's intelligence value"), TEXT("The intelligence value of this character."),
			{ TYPE_INT }, (TYPE_INT),
			&Thread::GetCardIntelligenceValue
		},
		{
			33, EXPRCAT_CHARPROP,
			TEXT("Intelligence Rank"), TEXT("%p 's intelligence rank"), TEXT("The intelligence rank of this character."),
			{ TYPE_INT }, (TYPE_INT),
			&Thread::GetCardIntelligenceRank
		},
		{
			34, EXPRCAT_CHARPROP,
			TEXT("Strength"), TEXT("%p 's strength"), TEXT("The strength of this character."),
			{ TYPE_INT }, (TYPE_INT),
			&Thread::GetCardStrength
		},
		{
			35, EXPRCAT_CHARPROP,
			TEXT("Strength Value"), TEXT("%p 's strength value"), TEXT("The strength value of this character."),
			{ TYPE_INT }, (TYPE_INT),
			&Thread::GetCardStrengthValue
		},
		{
			36, EXPRCAT_CHARPROP,
			TEXT("Strength Rank"), TEXT("%p 's strength rank"), TEXT("The strength rank of this character."),
			{ TYPE_INT }, (TYPE_INT),
			&Thread::GetCardStrengthRank
		},
		{
			37, EXPRCAT_CHARPROP,
			TEXT("Sociability"), TEXT("%p 's sociability"), TEXT("The sociability of this character."),
			{ TYPE_INT }, (TYPE_INT),
			&Thread::GetCardSociability
		},
		{
			38, EXPRCAT_CHARPROP,
			TEXT("Partners count"), TEXT("%p 's partners count"), TEXT("The partners count of this character."),
			{ TYPE_INT }, (TYPE_INT),
			&Thread::GetCardPartnerCount
		},
		{
			39, EXPRCAT_EVENT,
			TEXT("Npc Original Answer Chance"), TEXT("Get Npc Original Answer Percent"),
			TEXT("If executed in a trigger with the Npc Answers Event, this is success Chance that the Interaction had in Percent"),
			{}, (TYPE_INT),
			&Thread::GetNpcResponseOriginalPercent
		},
		{
			40, EXPRCAT_EVENT,
			TEXT("Npc Current Answer Chance"), TEXT("Get Npc Current Answer Percent"),
			TEXT("If executed in a trigger with the Npc Answers Event, this is the current Interaction Percent, modified by this or previously executed Triggers. "
			"using the Set Npc Response Percent Action"),
			{}, (TYPE_INT),
			&Thread::GetNpcResponseCurrentPercent
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
			TEXT("Enumeration"), TEXT("Enumeration"), TEXT("A known constant with a name"),
			{}, (TYPE_BOOL),
			NULL
		},
		{
			4,EXPRCAT_COMPARISION_BOOL,
			TEXT("Logical And"),TEXT("%p && %p"),TEXT("Logical and, including short-circut evaluation"),
			{ TYPE_BOOL, TYPE_BOOL },(TYPE_BOOL),
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
			TEXT("Not"), TEXT("! %p"), TEXT("Logical Not"),
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
			16, EXPRCAT_CHARPROP,
			TEXT("Is Seat Filled"), TEXT("Seat %p is Filled"),
			TEXT("Whether the given Seat has a character behind it. Characters are identified using their seat number. Use this in a loop through all seats "
			"do determine if a character exists or not."),
			{ TYPE_INT }, (TYPE_BOOL),
			&Thread::IsSeatFilled
		},
		{
			17, EXPRCAT_CHARPROP,
			TEXT("Is Lover"), TEXT("%p has %p as a lover"),
			TEXT("True if the characters are currently in a relationship. (technically, it checks if the first parameter is in a relationship with the second)"),
			{ TYPE_INT, TYPE_INT }, (TYPE_BOOL),
			&Thread::IsLover
		},
		{
			18,EXPRCAT_COMPARISION_FLOAT,
			TEXT("Greater Than"),TEXT("%p > %p"),TEXT("Greater-Than"),
			{ TYPE_FLOAT, TYPE_FLOAT },(TYPE_BOOL),
			&Thread::GreaterThanFloats
		},
		{
			19, EXPRCAT_COMPARISION_FLOAT,
			TEXT("Greater Than or Equal"), TEXT("%p >= %p"), TEXT("Greater-Than or equal"),
			{ TYPE_FLOAT, TYPE_FLOAT }, (TYPE_BOOL),
			&Thread::GreaterEqualsFloats
		},
		{
			20, EXPRCAT_COMPARISION_FLOAT,
			TEXT("Equal"), TEXT("%p == %p"), TEXT("Equal"),
			{ TYPE_FLOAT, TYPE_FLOAT }, (TYPE_BOOL),
			&Thread::EqualsFloats
		},
		{
			21, EXPRCAT_COMPARISION_FLOAT,
			TEXT("Less Than or Equal"), TEXT("%p <= %p"), TEXT("less than or equal"),
			{ TYPE_FLOAT, TYPE_FLOAT }, (TYPE_BOOL),
			&Thread::LessEqualsFloats
		},
		{
			22, EXPRCAT_COMPARISION_FLOAT,
			TEXT("Less Than"), TEXT("%p < %p"), TEXT("less than"),
			{ TYPE_FLOAT, TYPE_FLOAT }, (TYPE_BOOL),
			&Thread::LessThanFloats
		},
		{
			23, EXPRCAT_COMPARISION_FLOAT,
			TEXT("Not Equal"), TEXT("%p != %p"), TEXT("not equal"),
			{ TYPE_FLOAT, TYPE_FLOAT }, (TYPE_BOOL),
			&Thread::NotEqualsFloats
		},
		{
			24, EXPRCAT_CHARPROP,
			TEXT("Get Card Storage Bool"), TEXT("Get %p 's Card Storage entry under %p , defaulting to %p on error"),
			TEXT("Gets the integer from the given cards storage entry. If the entry doesnt exist or holds a value of a different type, "
			"it returns the default value instead"),
			{ TYPE_INT, TYPE_STRING, TYPE_BOOL }, (TYPE_BOOL),
			&Thread::GetCardStorageBool
		},
		{
			25, EXPRCAT_CHARPROP,
			TEXT("Trait"), TEXT("%p 's %p trait"), TEXT(""),
			{ TYPE_INT, TYPE_INT }, (TYPE_BOOL),
			&Thread::GetCardTrait
		},
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
			TEXT("Enumeration"), TEXT("Enumeration"), TEXT("A known constant with a name"),
			{}, (TYPE_FLOAT),
			NULL
		},
		{
			4, EXPRCAT_MATH,
			TEXT("Random Float"), TEXT("Get Random Float between %p and %p"), TEXT("Generates a random float between the two arguments (including both)"),
			{ (TYPE_FLOAT), (TYPE_FLOAT) }, (TYPE_FLOAT),
			&Thread::GetRandomFloat
		},
		{
			EXPR_INT_PLUS, EXPRCAT_MATH,
			TEXT("+"), TEXT("%p + %p"), TEXT("Adds two floats"),
			{ (TYPE_FLOAT), (TYPE_FLOAT) }, (TYPE_FLOAT),
			&Thread::AddFloats
		},
		{
			6, EXPRCAT_MATH,
			TEXT("-"), TEXT("%p - %p"), TEXT("Substracts two floats"),
			{ (TYPE_FLOAT), (TYPE_FLOAT) }, (TYPE_FLOAT),
			&Thread::SubstractFloats
		},
		{
			7, EXPRCAT_MATH,
			TEXT("/"), TEXT("%p / %p"), TEXT("Divide two floats"),
			{ (TYPE_FLOAT), (TYPE_FLOAT) }, (TYPE_FLOAT),
			&Thread::DivideFloats
		},
		{
			8, EXPRCAT_MATH,
			TEXT("*"), TEXT("%p * %p"), TEXT("Multiply two floats"),
			{ (TYPE_FLOAT), (TYPE_FLOAT) }, (TYPE_FLOAT),
			&Thread::MultiplyFloats
		},
		{
			9, EXPRCAT_MATH,
			TEXT("Int to Float"), TEXT("Float ( %p )"), TEXT("Converts an Int to Float"),
			{ TYPE_INT }, (TYPE_FLOAT),
			&Thread::Int2Float
		},
		{
			10,EXPRCAT_CHARPROP,
			TEXT("Get Card Storage Float"),TEXT("Get %p 's Card Storage entry under %p , defaulting to %p on error"),
			TEXT("Gets the integer from the given cards storage entry. If the entry doesnt exist or holds a value of a different type, "
			"it returns the default value instead"),
			{ TYPE_INT, TYPE_STRING, TYPE_FLOAT },(TYPE_FLOAT),
				&Thread::GetCardStorageFloat
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
			TEXT("Enumeration"), TEXT("Enumeration"), TEXT("A known constant with a name"),
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
			TEXT("Second Name"), TEXT("Second Name of %p"), TEXT("The second name this character was given (the lower one on the default card image). "
			"Note that this may or may not be the family name depending on how the card maker ordered these."),
			{ TYPE_INT }, (TYPE_STRING),
				&Thread::SubString
		},
		{
			7, EXPRCAT_CHARPROP,
			TEXT("Get Card Storage String"), TEXT("Get %p 's Card Storage entry under %p , defaulting to %p on error"),
			TEXT("Gets the integer from the given cards storage entry. If the entry doesnt exist or holds a value of a different type, "
			"it returns the default value instead"),
			{ TYPE_INT, TYPE_STRING, TYPE_STRING }, (TYPE_STRING),
				&Thread::GetCardStorageString
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