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
			void SwitchCardStyle(std::vector<Value>& params);
			void AddCardLovePoints(std::vector<Value>& params);
			void AddCardLikePoints(std::vector<Value>& params);
			void AddCardDislikePoints(std::vector<Value>& params);
			void AddCardHatePoints(std::vector<Value>& params);
			void AddCardPoints(std::vector<Value>& params);
			void SetCardPoints(std::vector<Value>& params);
			void SetLover(std::vector<Value>& params);
			void SetPCResponse(std::vector<Value>& params);
			void SetStrongResponse(std::vector<Value>& params);
			void SetAbsoluteResponse(std::vector<Value>& params);
			void SetPCTarget(std::vector<Value>& params);
			void SetCardVirtue(std::vector<Value>& params);
			void SetCardTrait(std::vector<Value>& params);
			void SetCardPreference(std::vector<Value>& params);
			void SetCherryStatus(std::vector<Value>& params);
			void SetClothingState(std::vector<Value>& params);
			void SetCardPersonality(std::vector<Value>& params);
			void SetCardVoicePitch(std::vector<Value>& params);
			void SetCum(std::vector<Value>& params);
			void SetTears(std::vector<Value>& params);
			void SetHighlight(std::vector<Value>& params);
			void SetGlasses(std::vector<Value>& params);
			void SetCardClub(std::vector<Value>& params);
			void SetCardClubValue(std::vector<Value>& params);
			void SetCardClubRank(std::vector<Value>& params);
			void SetCardIntelligence(std::vector<Value>& params);
			void SetCardIntelligenceValue(std::vector<Value>& params);
			void SetCardIntelligenceRank(std::vector<Value>& params);
			void SetCardStrength(std::vector<Value>& params);
			void SetCardFightingStyle(std::vector<Value>& params);
			void SetCharacterLocked(std::vector<Value>& params);
			void SetActionAboutRoom(std::vector<Value>& params);
			void SetMasturbating(std::vector<Value>& params);
			void SetCardStrengthValue(std::vector<Value>& params);
			void SetCardStrengthRank(std::vector<Value>& params);
			void SetCardSociability(std::vector<Value>& params);
			void SetCardLoversItem(std::vector<Value>& params);
			void SetCardFriendItem(std::vector<Value>& params);
			void SetCardSexualItem(std::vector<Value>& params);
			void SetCardFirstName(std::vector<Value>& params);
			void SetCardSecondName(std::vector<Value>& params);
			void SetCardDescription(std::vector<Value>& params);
			void WriteLog(std::vector<Value>& params);
			void SetCardOrientation(std::vector<Value>& params);
			void SetCardSexExperience(std::vector<Value>& params);
			void SetCardAnalSexExperience(std::vector<Value>& params);
			void SetCardOpinion(std::vector<Value>& params);
			void SetCardSexCompatibility(std::vector<Value>& params);

			void SetCardCumStatInVagina(std::vector<Value>& params);
			void SetCardCumStatInAnal(std::vector<Value>& params);
			void SetCardCumStatInMouth(std::vector<Value>& params);
			void SetCardCumStatTotalCum(std::vector<Value>& params);
			void SetCardCumStatClimaxCount(std::vector<Value>& params);
			void SetCardCumStatSimClimaxCount(std::vector<Value>& params);
			void SetCardCumStatCondomsUsed(std::vector<Value>& params);
			void SetRoomTarget(std::vector<Value>& params);
			void SetDecals(std::vector<Value>& params);
			void SetCardCumStatRiskyCums(std::vector<Value>& params);
			void SetCardVaginalSex(std::vector<Value>& params);
			void SetCardAnalSex(std::vector<Value>& params);
			void SetCardAllSex(std::vector<Value>& params);

			void NpcMoveRoom(std::vector<Value>& params);
			void NpcActionNoTarget(std::vector<Value>& params);
			void NpcTalkTo(std::vector<Value>& params);
			void NpcTalkToAbout(std::vector<Value>& params);
			void NpcCancelAction(std::vector<Value>& params);

			void SetCardStorageInt(std::vector<Value>& params);
			void SetCardStorageFloat(std::vector<Value>& params);
			void SetCardStorageString(std::vector<Value>& params);
			void SetCardStorageBool(std::vector<Value>& params);

			void RemoveCardStorageInt(std::vector<Value>& params);
			void RemoveCardStorageFloat(std::vector<Value>& params);
			void RemoveCardStorageString(std::vector<Value>& params);
			void RemoveCardStorageBool(std::vector<Value>& params);

			void SetPC(std::vector<Value>& params);
			void SetPose(std::vector<Value>& params);
			void IsConsensualH(std::vector<Value>& params);
			void AutoPC(std::vector<Value>& params);
			void StartHScene(std::vector<Value>& params);
			void SwitchActiveInH(std::vector<Value>& params);
			void SetNpcStatus(std::vector<Value>& params);
			void ResetVoyeur(std::vector<Value>& params);

			void SetH_AI(std::vector<Value>& params);

			void Set_H_AI_LOCK(std::vector<Value>& params);
			void AddMood(std::vector<Value>& params);
			void ReplaceMood(std::vector<Value>& params);

			//event response
			void SetNpcResponseSuccess(std::vector<Value>& params);
			void SetNpcResponseAnswer(std::vector<Value>& params);
			void SetNpcResponsePercent(std::vector<Value>& params);	//int()

			void SetNpcResponseStrongPercent(std::vector<Value>& params);

			void SetNpcResponseAbsolutePercent(std::vector<Value>& params);

			void SetNpcStrongResponseSuccess(std::vector<Value>& params);

			void SetNpcAbsoluteResponseSuccess(std::vector<Value>& params);

			///////////////////////
			// Expressions

			Value GetTriggeringCard(std::vector<Value>& params);	//int ()
			Value GetDominantInH(std::vector<Value>&);
			Value GetSubmissiveInH(std::vector<Value>&);
			Value GetActiveInH(std::vector<Value>&);
			Value GetPassiveInH(std::vector<Value>&);
			Value GetHPosition(std::vector<Value>&);
			Value GetThisCard(std::vector<Value>& params); //int ()
			Value GetPC(std::vector<Value>&); //int()
			Value IsSeatFilled(std::vector<Value>& params); //bool (int)

			Value PoseExists(std::vector<Value>& params);

			Value GetCum(std::vector<Value>& params);

			Value GetTears(std::vector<Value>& params);

			Value GetGlasses(std::vector<Value>& params);

			Value GetHighlight(std::vector<Value>& params);

			//time info
			Value GetDaysPassed(std::vector<Value>& params); //int ()
			Value GetCurrentDay(std::vector<Value>& params); //int()
			Value GetCurrentPeriod(std::vector<Value>& params); //int()


			//card info
			Value GetCardVirtue(std::vector<Value>& params); //int(int)
			Value GetCardTrait(std::vector<Value>& params);
			Value GetCardPreference(std::vector<Value>& params);
			Value GetCardOpinion(std::vector<Value>& params);
			Value GetCardFigure(std::vector<Value>& params);

			Value GetCardBreastSize(std::vector<Value>& params);

			//bool(int, int)
			Value GetCardPersonality(std::vector<Value>& params); //int(int)
			Value GetCardVoicePitch(std::vector<Value>& params); //int(int)
			Value GetCardClub(std::vector<Value>& params); //int(int)
			Value GetCardClubValue(std::vector<Value>& params); //int(int)
			Value GetCardClubRank(std::vector<Value>& params); //int(int)
			Value GetCardIntelligence(std::vector<Value>& params); //int(int)
			Value GetCardIntelligenceValue(std::vector<Value>& params); //int(int)
			Value GetCardIntelligenceRank(std::vector<Value>& params); //int(int)
			Value GetCardStrength(std::vector<Value>& params); //int(int)
			Value GetCardFightingStyle(std::vector<Value>& params); //int(int)
			//int(int)
			Value GetCardLocked(std::vector<Value>& params);
			Value GetActionAboutRoom(std::vector<Value>& params);
			Value GetMasturbating(std::vector<Value>& params);
			Value GetCurrentConvo(std::vector<Value>& params);
			Value GetCherryStatus(std::vector<Value>& params);
			Value GetCardStrengthValue(std::vector<Value>& params); //int(int)
			Value GetCardStrengthRank(std::vector<Value>& params); //int(int)
			Value GetCardSociability(std::vector<Value>& params); //int(int)
			Value GetCardLastName(std::vector<Value>& params); //string(int)
			Value GetCardFirstName(std::vector<Value>& params); //string(int)
			Value GetCardFullName(std::vector<Value>& params);	//string(int)
			Value GetCardDescription(std::vector<Value>& params); //string(int)
			Value GetCardPartnerCount(std::vector<Value>& params); //int(int)
			Value GetDecals(std::vector<Value>& params);
			Value GetCardOrientation(std::vector<Value>& params); //int(int)
			Value GetCardOrientationMultiplier(std::vector<Value>& params);	//float(int, int)
			Value GetCardGender(std::vector<Value>& params); //int(int)
			Value GetCardLovePoints(std::vector<Value>& params); //int(int,int)
			Value GetCardLikePoints(std::vector<Value>& params); //int(int,int)
			Value GetCardDislikePoints(std::vector<Value>& params); //int(int,int)
			Value GetCardHatePoints(std::vector<Value>& params); //int(int,int)
			Value IsLover(std::vector<Value>& params); //bool(int,int)
			Value GetHasLovers(std::vector<Value>& params);	//bool(int)
			Value GetStrongestMood(std::vector<Value>& params); //int(int)
			Value GetMoodStrength(std::vector<Value>& params); //int(int, int)
			Value GetPregnancyRisk(std::vector<Value>& params); //int(int, int)
			Value GetSexCompatibility(std::vector<Value>& params); //int(int,int)
			Value GetCurrentSyle(std::vector<Value>& params); //int(int)
			Value GetStyle(std::vector<Value>& params); //int(string)
			Value GetSexExperience(std::vector<Value>& params); //bool(int)
			Value GetAnalSexExperience(std::vector<Value>& params); //bool(int)
			Value FindSeat(std::vector<Value>& params); //int(string)
			Value GetNpcStatus(std::vector<Value>& params); //int(int)
			Value GetHeight(std::vector<Value>& params);
			Value GetTarget(std::vector<Value>& params);
			Value PCTalkAbout(std::vector<Value>& params);
			Value GetCardLastHPartner(std::vector<Value>& params); //string(int)
			Value GetCardFirstHPartner(std::vector<Value>& params); //string(int)
			Value GetCardFirstAnalPartner(std::vector<Value>& params); //string(int)
			Value GetCardRejectCount(std::vector<Value>& params); //int(int)
			Value GetCardWinCount(std::vector<Value>& params); //int(int)
			Value GetCardVictoryCount(std::vector<Value>& params); //int(int)
			Value GetCardSkipCount(std::vector<Value>& params); //int(int)
			Value GetCardCumStatInVagina(std::vector<Value>& params); //int(int, int)
			Value GetCardCumStatInAnal(std::vector<Value>& params); //int(int, int)
			Value GetCardCumStatInMouth(std::vector<Value>& params); //int(int, int)
			Value GetCardCumStatInVaginaTotal(std::vector<Value>& params);	//int(int)
			Value GetCardCumStatInAnalTotal(std::vector<Value>& params);	//int(int)
			Value GetCardCumStatInMouthTotal(std::vector<Value>& params);	//int(int)
			Value GetCardCumStatTotalCum(std::vector<Value>& params);	//int(int, int)
			Value GetCardCumStatTotalCumTotal(std::vector<Value>& params);	//int(int)
			Value GetCardCumStatClimaxCount(std::vector<Value>& params);	//int(int, int)
			Value GetCardCumStatClimaxCountTotal(std::vector<Value>& params);	//int(int)
			Value GetCardCumStatSimClimaxCount(std::vector<Value>& params);	//int(int, int)
			Value GetCardCumStatSimClimaxCountTotal(std::vector<Value>& params);	//int(int)
			Value GetCardCumStatCondomsUsed(std::vector<Value>& params);	//int(int, int)
			Value GetCardCumStatCondomsUsedTotal(std::vector<Value>& params);	//int(int)
			Value GetCardCumStatRiskyCums(std::vector<Value>& params);
			Value GetCardCumStatRiskyCumsTotal(std::vector<Value>& params);
			Value GetCardVaginalSex(std::vector<Value>& params);	//int(int, int)	
			Value GetCardVaginalSexTotal(std::vector<Value>& params);	//int(int)
			Value GetCardAnalSex(std::vector<Value>& params);	//int(int, int)
			Value GetCardAnalSexTotal(std::vector<Value>& params);	//int(int)
			Value GetCardAllSex(std::vector<Value>& params);	//int(int, int)
			Value GetCardAllSexTotal(std::vector<Value>& params);	//int(int)
			Value GetCardLoversItem(std::vector<Value>& params);	//string(int)
			Value GetCardFriendItem(std::vector<Value>& params);	//string(int)
			Value GetCardSexualItem(std::vector<Value>& params);	//string(int)

			//basic int stuff
			Value GetRandomInt(std::vector<Value>& params); //int(int,int)
			Value AddIntegers(std::vector<Value>& params);	//int(int,int)
			Value SubstractIntegers(std::vector<Value>& params); //int(int,int)
			Value DivideIntegers(std::vector<Value>& params); //int(int,int)
			Value MultiplyIntegers(std::vector<Value>& params); //int(int,int)
			Value Float2Int(std::vector<Value>& params); //int(float)
			Value StrLength(std::vector<Value>& params); //int(string)
			Value FirstIndexOf(std::vector<Value>& params); //int(string, string)
			Value FirstIndexOfFrom(std::vector<Value>& params); //int(string, int, string)
			Value String2Int(std::vector<Value>& params); //int(string)
			Value GetNpcCurrentRoom(std::vector<Value>& params);

			
			//basic float stuff
			Value GetRandomFloat(std::vector<Value>& params); //float(float,float)
			Value AddFloats(std::vector<Value>& params);	//float(float,float)
			Value SubstractFloats(std::vector<Value>& params); //float(float,float)
			Value DivideFloats(std::vector<Value>& params); //float(float,float)
			Value MultiplyFloats(std::vector<Value>& params); //float(float,float)
			Value Int2Float(std::vector<Value>& params); //float(int)
			Value String2Float(std::vector<Value>& params); //float(string)

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
			Value GreaterThanFloats(std::vector<Value>& params); //bool(float,float)
			Value GreaterEqualsFloats(std::vector<Value>& params); //bool(float,float)
			Value EqualsFloats(std::vector<Value>& params); //bool(float,float)
			Value NotEqualsFloats(std::vector<Value>& params); //bool(float,float)
			Value LessEqualsFloats(std::vector<Value>& params); //bool(float,float)
			Value LessThanFloats(std::vector<Value>& params); //bool(float,float)
			Value IsInterruptAction(std::vector<Value>& params); //bool(int)
			Value IsMinnaAction(std::vector<Value>& params); //bool(int)
			Value IsForceAction(std::vector<Value>& params); //bool(int)
			Value IsSexAction(std::vector<Value>& params); //bool(int)
			Value IsNoPromptAction(std::vector<Value>& params); //bool(int)
			Value IsGameOverAction(std::vector<Value>& params); //bool(int)
			Value IsNoTargetAction(std::vector<Value>& params); //bool(int)
			Value RollFloat(std::vector<Value>& params); //bool(float)
			Value RollInt(std::vector<Value>& params); //bool(int)
			Value EqualsStrings(std::vector<Value>& params); //bool(string,string)

			//basic string stuff
			Value SubString(std::vector<Value>& params); //string(string, int, int)
			Value StringConcat(std::vector<Value>& params); //string(string, string)
			Value IntToString(std::vector<Value>& params); //string(int)
			Value FloatToString(std::vector<Value>& params); //string(float)
			Value BoolToString(std::vector<Value>& params); //string(bool)
			Value StringReplace(std::vector<Value>& params); //string(string, int, int)

			Value GetCardStorageInt(std::vector<Value>& params); //int(int, string, int)
			Value GetCardStorageFloat(std::vector<Value>& params); //float(int, string, float)
			Value GetCardStorageString(std::vector<Value>& params); //string(int, string, string)
			Value GetCardStorageBool(std::vector<Value>& params); //bool(int, string, bool)

			//Event Response
			//PERIOD_ENDS
			Value GetEndingPeriod(std::vector<Value>& params);
			Value GetStartingPeriod(std::vector<Value>& params);

			//NPC_RESPONSE
			Value GetNpcResponseOriginalAnswerSuccess(std::vector<Value>& params); //bool()
			Value GetNpcResponseOriginalAnswer(std::vector<Value>& params);
			Value GetNpcResponseCurrentAnswerSuccess(std::vector<Value>& params);	//bool()
			Value GetNpcResponseEffectiveAnswerSuccess(std::vector<Value>& params);
			Value GetNpcResponseCurrentAnswer(std::vector<Value>& params);
			Value GetAutoPC(std::vector<Value>& params);
			Value GetNpcResponseTarget(std::vector<Value>& params);	//int()
			Value GetNpcResponseConversation(std::vector<Value>& params);	//int()
			Value GetNpcResponseOriginalPercent(std::vector<Value>& params);	//int()
			Value GetNpcResponseCurrentPercent(std::vector<Value>& params);	//int()

			Value GetNpcResponseEffectivePercent(std::vector<Value>& params);

			//NPC_WALK_TO_ROOM
			Value GetNpcRoomTarget(std::vector<Value>& params);
			
			//NPC_WANT_ACTION_NOTARGET
			Value GetNpcActionId(std::vector<Value>& params);

			//NPC_WANT_TALK_WITH
			Value GetNpcTalkTarget(std::vector<Value>& params);

			//NPC_WANT_TALK_WITH_ABOUT
			Value GetNpcTalkAbout(std::vector<Value>& params);

			//PC_CONVERSATION_STATE_UPDATED
			Value GetConversationState(std::vector<Value>& params);
			Value GetConversationLine(std::vector<Value>& params);
			Value GetConversationNpcResponse(std::vector<Value>& params);
			Value GetConversationActor(std::vector<Value>& params);
			Value GetConversationPcResponse(std::vector<Value>& params);
			Value GetEffectivePcResponse(std::vector<Value>& params);
			Value GetConversationAction(std::vector<Value>& params);
			Value GetConversationAnswerId(std::vector<Value>& params);
			Value GetConversationCurrentlyAnswering(std::vector<Value>& params);
			Value GetEventID(std::vector<Value>& params);

			//ROOM_CHANGE
			Value GetEventPreviousRoom(std::vector<Value>& params);
			Value GetKeyPressVal(std::vector<Value>& params);
		private:
			Value EvaluateExpression(ParameterisedExpression& expr);
			bool ExecuteAction(ParameterisedAction& action);

		};




	}
}
