#include "StdAfx.h"
#include "Files\PersistentStorage.h"

namespace Shared {
	namespace Triggers {
		
		void Thread::ShouldNotBeImplemented(std::vector<Value>& params) {
			MessageBox(NULL, TEXT("This message should not have been executed."), TEXT("Error"), 0);
		}


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

		//void
		void Thread::ConditionalEndExecution(std::vector<Value>& params) {
			if (params[0].bVal) {
				this->execFinished = true;
			}
		}

		//int card, int roomId
		void Thread::NpcMoveRoom(std::vector<Value>& params) {
			int cardSeat = params[0].iVal;
			int roomId = params[1].iVal;
			CharInstData* card = &AAPlay::g_characters[cardSeat];
			if (!card->IsValid()) return;

			card->m_forceAction.conversationId = 0;
			card->m_forceAction.movementType = 2;
			card->m_forceAction.roomTarget = roomId;
			card->m_forceAction.target1 = NULL;
			card->m_forceAction.target2 = NULL;
			card->m_forceAction.unknown = -1;
			card->m_forceAction.unknown2 = 1;
		}

		//int card, int actionId
		void Thread::NpcActionNoTarget(std::vector<Value>& params) {
			int cardSeat = params[0].iVal;
			int actionId = params[1].iVal;
			CharInstData* card = &AAPlay::g_characters[cardSeat];
			if (!card->IsValid()) return;

			card->m_forceAction.conversationId = actionId;
			card->m_forceAction.movementType = 3;
			card->m_forceAction.roomTarget = -1;
			card->m_forceAction.target1 = NULL;
			card->m_forceAction.target2 = NULL;
			card->m_forceAction.unknown = -1;
			card->m_forceAction.unknown2 = 1;
		}

		//int card, int conversationId, int talkTo
		void Thread::NpcTalkTo(std::vector<Value>& params) {
			int cardSeat = params[0].iVal;
			int actionId = params[1].iVal;
			int cardTarget = params[2].iVal;
			CharInstData* card = &AAPlay::g_characters[cardSeat];
			if (!card->IsValid()) return;
			CharInstData* target = &AAPlay::g_characters[cardTarget];
			if (!card->IsValid()) return;

			card->m_forceAction.conversationId = actionId;
			card->m_forceAction.movementType = 3;
			card->m_forceAction.roomTarget = -1;
			card->m_forceAction.target1 = target->m_char->GetActivity();
			card->m_forceAction.target2 = NULL;
			card->m_forceAction.unknown = -1;
			card->m_forceAction.unknown2 = 1;
		}

		//int card, int conversationId, int talkTo, int talkAbout
		void Thread::NpcTalkToAbout(std::vector<Value>& params) {
			int cardSeat = params[0].iVal;
			int actionId = params[1].iVal;
			int cardTarget = params[2].iVal;
			int cardAbout = params[3].iVal;
			CharInstData* card = &AAPlay::g_characters[cardSeat];
			if (!card->IsValid()) return;
			CharInstData* target = &AAPlay::g_characters[cardTarget];
			if (!card->IsValid()) return;
			CharInstData* about = &AAPlay::g_characters[cardAbout];
			if (!card->IsValid()) return;

			card->m_forceAction.conversationId = actionId;
			card->m_forceAction.movementType = 3;
			card->m_forceAction.roomTarget = -1;
			card->m_forceAction.target1 = target->m_char->GetActivity();
			card->m_forceAction.target2 = about->m_char->GetActivity();
			card->m_forceAction.unknown = -1;
			card->m_forceAction.unknown2 = 1;
		}

		//event response

		//bool newAnswer
		void Thread::SetNpcResponseAnswer(std::vector<Value>& params) {
			if (this->eventData->GetId() != NPC_RESPONSE) return;
			((NpcResponseData*)eventData)->changedResponse = params[0].bVal;
		}

		//int percent
		void Thread::SetNpcResponsePercent(std::vector<Value>& params) {
			if (this->eventData->GetId() != NPC_RESPONSE) return;
			((NpcResponseData*)eventData)->changedChance = params[0].iVal;
		}

		int SafeAddCardPoints(int nPoints, int pointKind, int iCardFrom, int iCardTowards) {
			if (pointKind < 0 || pointKind > 3) return 0;
			CharInstData* cardFrom = &AAPlay::g_characters[iCardFrom];
			CharInstData* cardTowards = &AAPlay::g_characters[iCardTowards];
			if (!cardFrom->IsValid()) return 0;
			if (!cardTowards->IsValid()) return 0;
			if (cardFrom == cardTowards) return 0;

			auto* ptrRel = cardFrom->m_char->GetRelations();
			auto* rel = ptrRel->m_start;
			if (ptrRel == NULL) return 0;
			for (rel; rel != ptrRel->m_end; rel++) {
				if (rel->m_targetSeat == iCardTowards) {
					break;
				}
			}
			if (rel == ptrRel->m_end) return 0;

			switch (pointKind) {
			case 0:
				rel->m_lovePoints += nPoints;
				break;
			case 1:
				rel->m_likePoints += nPoints;
				break;
			case 2:
				rel->m_dislikePoints += nPoints;
				break;
			case 3:
				rel->m_hatePoints += nPoints;
			default:
				break;
			}

			AAPlay::ApplyRelationshipPoints(cardFrom->m_char, rel);
			return 1;
		}

		//int cardFrom, int cardTowards, int nPoints
		void Thread::AddCardLovePoints(std::vector<Value>& params) {
			int nPoints = params[2].iVal;
			int iCardFrom = params[0].iVal;
			int iCardTowards = params[1].iVal;

			SafeAddCardPoints(nPoints, 0, iCardFrom, iCardTowards);
		}

		//int cardFrom, int cardTowards, int nPoints
		void Thread::AddCardLikePoints(std::vector<Value>& params) {
			int nPoints = params[2].iVal;
			int iCardFrom = params[0].iVal;
			int iCardTowards = params[1].iVal;

			SafeAddCardPoints(nPoints, 1, iCardFrom, iCardTowards);
		}

		//int cardFrom, int cardTowards, int nPoints
		void Thread::AddCardDislikePoints(std::vector<Value>& params) {
			int nPoints = params[2].iVal;
			int iCardFrom = params[0].iVal;
			int iCardTowards = params[1].iVal;

			SafeAddCardPoints(nPoints, 2, iCardFrom, iCardTowards);
		}

		//int cardFrom, int cardTowards, int nPoints
		void Thread::AddCardHatePoints(std::vector<Value>& params) {
			int nPoints = params[2].iVal;
			int iCardFrom = params[0].iVal;
			int iCardTowards = params[1].iVal;

			SafeAddCardPoints(nPoints, 3, iCardFrom, iCardTowards);
		}

		//int cardFrom, int cardTowards, int pointKind, int nPoints
		void Thread::AddCardPoints(std::vector<Value>& params) {
			int iCardFrom = params[0].iVal;
			int iCardTowards = params[1].iVal;
			int pointKind = params[2].iVal;
			int nPoints = params[3].iVal;

			if (pointKind < 0 || pointKind > 3) return;
			CharInstData* cardFrom = &AAPlay::g_characters[iCardFrom];
			CharInstData* cardTowards = &AAPlay::g_characters[iCardTowards];
			if (!cardFrom->IsValid()) return;
			if (!cardTowards->IsValid()) return;
			if (cardFrom == cardTowards) return;

			auto* ptrRel = cardFrom->m_char->GetRelations();
			auto* rel = ptrRel->m_start;
			if (ptrRel == NULL) return;
			for (rel; rel != ptrRel->m_end; rel++) {
				if (rel->m_targetSeat == iCardTowards) {
					break;
				}
			}
			if (rel == ptrRel->m_end) return;

			switch (pointKind) {
			case 0:
				rel->m_lovePoints += nPoints;
				break;
			case 1:
				rel->m_likePoints += nPoints;
				break;
			case 2:
				rel->m_dislikePoints += nPoints;
				break;
			case 3:
				rel->m_hatePoints += nPoints;
				break;
			default:
				break;
			}

			AAPlay::ApplyRelationshipPoints(cardFrom->m_char, rel);
		}
		
		//int cardFrom, int cardTowards, int ptsLove, int ptsLike, int ptsDislike, int ptsHate, int ptsSpare
		void Thread::SetCardPoints(std::vector<Value>& params) {
			int iCardFrom = params[0].iVal;
			int iCardTowards = params[1].iVal;
			float ptsLove = params[2].fVal;
			float ptsLike = params[3].fVal;
			float ptsDislike = params[4].fVal;
			float ptsHate = params[5].fVal;
			float ptsSpare = params[6].fVal;

			CharInstData* cardFrom = &AAPlay::g_characters[iCardFrom];
			CharInstData* cardTowards = &AAPlay::g_characters[iCardTowards];
			if (!cardFrom->IsValid()) return;
			if (!cardTowards->IsValid()) return;
			if (cardFrom == cardTowards) return;

			auto* ptrRel = cardFrom->m_char->GetRelations();
			auto* rel = ptrRel->m_start;
			if (ptrRel == NULL) return;
			for (rel; rel != ptrRel->m_end; rel++) {
				if (rel->m_targetSeat == iCardTowards) {
					break;
				}
			}
			if (rel == ptrRel->m_end) return;

			//normalize the points
			float ptsSum = ptsLove + ptsLike + ptsDislike + ptsHate + ptsSpare;
			float normalizer = 900.0 / ptsSum;
			ptsLove *= normalizer;
			ptsLike *= normalizer;
			ptsDislike *= normalizer;
			ptsHate *= normalizer;

			//apply the points, discard the decimals
			rel->m_lovePoints = ptsLove;
			rel->m_likePoints = ptsLike;
			rel->m_dislikePoints = ptsDislike;
			rel->m_hatePoints = ptsHate;

			AAPlay::ApplyRelationshipPoints(cardFrom->m_char, rel);
		}

		//int seat, int virtue
		void Thread::SetCardVirtue(std::vector<Value>& params) {
			int seat = params[0].iVal;
			int virtue = params[1].iVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			AAPlay::g_characters[seat].m_char->m_charData->m_character.virtue = min(max(virtue, 0), 4);
		}

		//int seat, int trait, bool enable
		void Thread::SetCardTrait(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			int trait = params[1].iVal;
			bool enable = params[2].bVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			AAPlay::g_characters[seat].m_char->m_charData->m_traitBools[trait] = enable;
		}

		//int seat, int personality
		void Thread::SetCardPersonality(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			int personality = params[1].iVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			AAPlay::g_characters[seat].m_char->m_charData->m_bPersonality = personality;
		}

		//int seat, int pitch
		void Thread::SetCardVoicePitch(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			int pitch = params[1].iVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			AAPlay::g_characters[seat].m_char->m_charData->m_voicePitch = pitch;
		}

		//int seat, int club
		void Thread::SetCardClub(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			int club = params[1].iVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			AAPlay::g_characters[seat].m_char->m_charData->m_club = club % 8;
		}
		//int seat, int value
		void Thread::SetCardClubValue(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			int value = params[1].iVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			AAPlay::g_characters[seat].m_char->m_charData->m_character.clubValue = value;
		}
		//int seat, int rank
		void Thread::SetCardClubRank(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			int rank = params[1].iVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			AAPlay::g_characters[seat].m_char->m_charData->m_character.clubClassRanking = rank;
		}

		//int seat, int intelligence
		void Thread::SetCardIntelligence(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			int intelligence = params[1].iVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			AAPlay::g_characters[seat].m_char->m_charData->m_character.intelligence = intelligence % 6;
		}
		//int seat, int value
		void Thread::SetCardIntelligenceValue(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			int value = params[1].iVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			AAPlay::g_characters[seat].m_char->m_charData->m_character.intelligenceValue = value;
		}
		//int seat, int rank
		void Thread::SetCardIntelligenceRank(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			int rank = params[1].iVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			AAPlay::g_characters[seat].m_char->m_charData->m_character.intelligenceClassRank = rank;
		}

		//int seat, int strength
		void Thread::SetCardStrength(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			int strength = params[1].iVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			AAPlay::g_characters[seat].m_char->m_charData->m_character.strength = strength % 6;
		}
		//int seat, int value
		void Thread::SetCardStrengthValue(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			int value = params[1].iVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			AAPlay::g_characters[seat].m_char->m_charData->m_character.strengthValue = value;
		}
		//int seat, int rank
		void Thread::SetCardStrengthRank(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			int rank = params[1].iVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			AAPlay::g_characters[seat].m_char->m_charData->m_character.strengthClassRank = rank;
		}

		//int seat, int sociability
		void Thread::SetCardSociability(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			int sociability = params[1].iVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			AAPlay::g_characters[seat].m_char->m_charData->m_character.sociability = sociability;
		}

		//int seat, string item
		void Thread::SetCardLoversItem(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			auto item = params[1].strVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			wcstombs_s(
				NULL,
				AAPlay::g_characters[seat].m_char->m_charData->m_item1,
				item->c_str(),
				item->size()
			);
		}

		//int seat, string item
		void Thread::SetCardFriendItem(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			auto item = params[1].strVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			wcstombs_s(
				NULL,
				AAPlay::g_characters[seat].m_char->m_charData->m_item1,
				item->c_str(),
				item->size()
			);
		}

		//int seat, string item
		void Thread::SetCardSexualItem(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			auto item = params[1].strVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			wcstombs_s(
				NULL,
				AAPlay::g_characters[seat].m_char->m_charData->m_item1,
				item->c_str(),
				item->size()
			);
		}

		//int seat, string name
		void Thread::SetCardFirstName(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			auto name = params[1].strVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			wcstombs_s(
				NULL,
				AAPlay::g_characters[seat].m_char->m_charData->m_forename,
				name->c_str(),
				name->size()
			);
		}

		//int seat, string name
		void Thread::SetCardSecondName(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			auto name = params[1].strVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			wcstombs_s(
				NULL,
				AAPlay::g_characters[seat].m_char->m_charData->m_surname,
				name->c_str(),
				name->size()
			);
		}

		//int seat, string name
		void Thread::SetCardDescription(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			auto name = params[1].strVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			wcstombs_s(
				NULL,
				AAPlay::g_characters[seat].m_char->m_charData->m_description,
				name->c_str(),
				name->size()
			);
		}

		//int seat, int orientation
		void Thread::SetCardOrientation(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			int orientation = params[1].iVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			AAPlay::g_characters[seat].m_char->m_charData->m_character.orientation = orientation;
		}

		//int seat, bool experience
		void Thread::SetCardSexExperience(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			int experience = params[1].bVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			AAPlay::g_characters[seat].m_char->m_charData->m_character.h_experience = experience;
		}

		//int seat, bool experience
		void Thread::SetCardAnalSexExperience(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			int experience = params[1].bVal;
			if (!AAPlay::g_characters[seat].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			AAPlay::g_characters[seat].m_char->m_charData->m_character.a_h_experience = experience;
		}

		//int seat, int target, int compatibility
		void Thread::SetCardSexCompatibility(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			int target = params[1].bVal;
			int compatibility = params[2].bVal;
			if (!AAPlay::g_characters[seat].m_char || !AAPlay::g_characters[target].m_char) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			AAPlay::g_characters[seat].m_char->m_charData->m_hCompatibility[target] = compatibility % 1000;
		}

		//int seat, int newset
		void Thread::SwitchCardStyle(std::vector<Value>& params) {
			int seat = params[0].iVal;
			int newset = params[1].iVal;
			if (!AAPlay::g_characters[seat].IsValid()) {
				LOGPRIO(Logger::Priority::WARN) << "[Trigger] Invalid card target; seat number " << seat << "\r\n";
				return;
			}
			auto& aau = AAPlay::g_characters[seat].m_cardData;
			aau.SwitchActiveCardStyle(newset, AAPlay::g_characters[seat].m_char->m_charData);

			auto storage = PersistentStorage::ClassStorage::getStorage(Shared::GameState::getCurrentClassSaveName());
			storage.storeCardInt(&AAPlay::g_characters[seat], L"m_currCardStyle", newset);
		}

		//int card, string keyname, int value
		void Thread::SetCardStorageInt(std::vector<Value>& params) {
			int card = params[0].iVal;
			CharInstData* inst = &AAPlay::g_characters[card];
			if (!inst->IsValid()) return;
			inst->m_cardData.GetCardStorage()[*params[1].strVal] = params[2];
			auto storage = PersistentStorage::ClassStorage::getStorage(Shared::GameState::getCurrentClassSaveName());
			storage.storeCardInt(inst, *params[1].strVal, params[2].iVal);
		}
		//int card, string keyname, float value
		void Thread::SetCardStorageFloat(std::vector<Value>& params) {
			int card = params[0].iVal;
			CharInstData* inst = &AAPlay::g_characters[card];
			if (!inst->IsValid()) return;
			inst->m_cardData.GetCardStorage()[*params[1].strVal] = params[2];
			auto storage = PersistentStorage::ClassStorage::getStorage(Shared::GameState::getCurrentClassSaveName());
			storage.storeCardFloat(inst, *params[1].strVal, params[2].fVal);
		}
		//int card, string keyname, string value
		void Thread::SetCardStorageString(std::vector<Value>& params) {
			int card = params[0].iVal;
			CharInstData* inst = &AAPlay::g_characters[card];
			if (!inst->IsValid()) return;
			inst->m_cardData.GetCardStorage()[*params[1].strVal] = params[2];
			auto storage = PersistentStorage::ClassStorage::getStorage(Shared::GameState::getCurrentClassSaveName());
			storage.storeCardString(inst, *params[1].strVal, General::CastToString(*params[2].strVal));
		}
		//int card, string keyname, bool value
		void Thread::SetCardStorageBool(std::vector<Value>& params) {
			int card = params[0].iVal;
			CharInstData* inst = &AAPlay::g_characters[card];
			if (!inst->IsValid()) return;
			inst->m_cardData.GetCardStorage()[*params[1].strVal] = params[2];
			auto storage = PersistentStorage::ClassStorage::getStorage(Shared::GameState::getCurrentClassSaveName());
			storage.storeCardBool(inst, *params[1].strVal, params[2].bVal);
		}

		//int card, string keyname
		void Thread::RemoveCardStorageInt(std::vector<Value>& params) {
			int card = params[0].iVal;
			CharInstData* inst = &AAPlay::g_characters[card];
			if (!inst->IsValid()) return;
			auto& atoms = inst->m_cardData.GetCardStorage();
			auto it = atoms.find(*params[1].strVal);
			if (it != atoms.end() && it->second.type == TYPE_INT) {
				atoms.erase(it);
			}
		}

		//int card, string keyname
		void Thread::RemoveCardStorageFloat(std::vector<Value>& params) {
			int card = params[0].iVal;
			CharInstData* inst = &AAPlay::g_characters[card];
			if (!inst->IsValid()) return;
			auto& atoms = inst->m_cardData.GetCardStorage();
			auto it = atoms.find(*params[1].strVal);
			if (it != atoms.end() && it->second.type == TYPE_FLOAT) {
				atoms.erase(it);
			}
		}

		//int card, string keyname
		void Thread::RemoveCardStorageString(std::vector<Value>& params) {
			int card = params[0].iVal;
			CharInstData* inst = &AAPlay::g_characters[card];
			if (!inst->IsValid()) return;
			auto& atoms = inst->m_cardData.GetCardStorage();
			auto it = atoms.find(*params[1].strVal);
			if (it != atoms.end() && it->second.type == TYPE_STRING) {
				atoms.erase(it);
			}
		}

		//int card, string keyname
		void Thread::RemoveCardStorageBool(std::vector<Value>& params) {
			int card = params[0].iVal;
			CharInstData* inst = &AAPlay::g_characters[card];
			if (!inst->IsValid()) return;
			auto& atoms = inst->m_cardData.GetCardStorage();
			auto it = atoms.find(*params[1].strVal);
			if (it != atoms.end() && it->second.type == TYPE_BOOL) {
				atoms.erase(it);
			}
		}

		//int seat
		void Thread::SetPC(std::vector<Value>& params) {
			int seat = params[0].iVal;
			CharInstData* inst = &AAPlay::g_characters[seat];
			if (!inst->IsValid()) return;
			else {
				Shared::GameState::setPlayerCharacter(seat);
			}
		}

		//int seatPC, int seatPartner
		void Thread::StartHScene(std::vector<Value>& params) {
			int seatPC = params[0].iVal;
			CharInstData* instPC = &AAPlay::g_characters[seatPC];
			if (!(instPC->IsValid() && instPC->m_char->m_seat == seatPC)) return;
			int seatPartner = params[1].iVal;
			CharInstData* instPartner = &AAPlay::g_characters[seatPartner];
			if (!(instPartner->IsValid() && instPartner->m_char->m_seat == seatPartner)) return;

			//save the original PC and its target
			GameState::setVoyeur(GameState::getPlayerCharacter());
			GameState::setVoyeurTarget(GameState::getVoyeur()->m_npcData->m_target);
			GameState::setIsPeeping(true);

			const DWORD offset1[]{ 0x376164, 0x88 };
			ExtClass::CharacterStruct** pc = (ExtClass::CharacterStruct**)ExtVars::ApplyRule(offset1);
			*pc = instPC->m_char;

			const DWORD offset2[]{ 0x376164, 0x28, 0x28 };
			ExtClass::NpcStatus** pcNpcStatus = (ExtClass::NpcStatus**)ExtVars::ApplyRule(offset2);
			*pcNpcStatus = instPC->m_char->m_characterStatus->m_npcStatus;

			const DWORD offset3[]{ 0x376164, 0x34, 0x14, 0x8C };
			ExtClass::NpcStatus** partnerNpcStatus = (ExtClass::NpcStatus**)ExtVars::ApplyRule(offset3);
			*partnerNpcStatus = instPartner->m_char->m_characterStatus->m_npcStatus;

			const DWORD offset4[]{ 0x3761CC, 0x28, 0x28 };
			DWORD* HSceneTrigger = (DWORD*)ExtVars::ApplyRule(offset4);
			*HSceneTrigger = 1;
		}

		//int seat, int status
		void Thread::SetNpcStatus(std::vector<Value>& params)
		{
			int seat = params[0].iVal;
			CharInstData* inst = &AAPlay::g_characters[seat];
			if (!inst->IsValid()) return;
			int status = params[1].iVal % 9;
			inst->m_char->m_characterStatus->m_npcStatus->m_status = status;
		}

		//
		void Thread::ResetVoyeur(std::vector<Value>& params)
		{
			if (Shared::GameState::getIsPeeping())	//clean up any voyerism
			{
				const DWORD offstPC[] = { 0x376164, 0x88 };
				const DWORD offstPCNPC[] = { 0x376164, 0x28, 0x28 };
				auto pc = (ExtClass::CharacterStruct**)ExtVars::ApplyRule(offstPC);
				auto pcnpc = (ExtClass::NpcData**)ExtVars::ApplyRule(offstPCNPC);
				(*pc)->m_characterStatus->m_npcStatus->m_status = 0;
				if (!Shared::GameState::getVoyeur()) return;
				*pc = Shared::GameState::getVoyeur();
				(*pc)->m_characterStatus->m_npcStatus->m_status = 0;
				*pcnpc = Shared::GameState::getVoyeurTarget();
				Shared::GameState::setVoyeur(nullptr);
				Shared::GameState::setVoyeurTarget(nullptr);
				Shared::GameState::setIsPeeping(false);
			}
		}

		//int seat, int mood, int moodStr
		void Thread::AddMood(std::vector<Value>& params) {
			int card = params[0].iVal;
			CharInstData* inst = &AAPlay::g_characters[card];
			int mood = params[1].iVal;
			int moodStr = params[2].iVal;
			//validation
			if (!inst->IsValid()) return;
			if (mood > 8 || mood < 0) return;
			if (moodStr > 9) moodStr = 9;
			if (moodStr < 0) moodStr = 0;

			auto moods1 = inst->m_char->GetMoods1();
			//shift old moods
			for (int i = 0; i < 9 - moodStr; i++) {
				moods1[i] = moods1[i + moodStr];
			}
			//add new moods
			for (int i = 9 - moodStr; i < 9; i++) {
				moods1[i] = mood;
			}
		}

		//int seat, int mood1, int mood2, int moodStr
		void Thread::ReplaceMood(std::vector<Value>& params) {
			int card = params[0].iVal;
			CharInstData* inst = &AAPlay::g_characters[card];
			int mood1 = params[1].iVal;
			int mood2 = params[2].iVal;
			int moodStr = params[3].iVal;
			//validation
			if (!inst->IsValid()) return;
			if (mood1 > 8 || mood1 < 0) return;
			if (mood2 > 8 || mood2 < 0) return;

			auto moods = inst->m_char->GetMoods1();
			int count = moodStr;
			for (int i = 8; i >= 0 && count > 0; i--) {
				if (moods[i] == mood1) {
					moods[i] = mood2;
					count--;
				}
			}
		}


		/*
		 * A list of all action categories
		 */

		std::wstring g_ActionCategories[ACTIONCAT_N] = {
			TEXT("General"),
			TEXT("Card Modification"),
			TEXT("Flow Control"),
			TEXT("Character Modification"),
			TEXT("Event Response"),
			TEXT("Npc Action")
		};


		/*
		* A list of all actions available.
		*/
		std::vector<Action> g_Actions = {
			{
				ACTION_SETVAR, ACTIONCAT_GENERAL, TEXT("Set Variable"), TEXT("%p = %p"), TEXT("Sets the value of a given variable to a new one"),
				{ TYPE_INVALID, TYPE_INVALID },
				&Thread::ShouldNotBeImplemented
			},
			{
				ACTION_IF, ACTIONCAT_FLOW_CONTROL, TEXT("If"), TEXT("If %p then"),
				TEXT("Executes Actions if Boolean Expression is true. Use with \"Else if\" and \"Else\" actions."),
				{ TYPE_BOOL },
				&Thread::ShouldNotBeImplemented
			},
			{
				ACTION_ELSEIF, ACTIONCAT_FLOW_CONTROL, TEXT("Else If"), TEXT("Else If %p then"),
				TEXT("Executes Actions if the previous If Action was not executed, and the Boolean Expression is true. Use with \"If\" and \"Else\" actions."
				" If this action does not preceed a \"If\" action, it acts as an \"If\" action instead."),
				{ TYPE_BOOL },
				&Thread::ShouldNotBeImplemented
			},
			{
				ACTION_ELSE, ACTIONCAT_FLOW_CONTROL, TEXT("Else"), TEXT("Else"),
				TEXT("Executes Actions if the preceeding \"If\" and \"Else If\" actions were all not Executed. Use with \"If\" and \"Else\" actions."
				" If this action does not preceed a \"If\" or \"Else if\" action, it is always executed."),
				{ },
				&Thread::ShouldNotBeImplemented
			},
			{
			ACTION_CONDJUMP,ACTIONCAT_FLOW_CONTROL,TEXT("Conditional Jump"),TEXT("Jump %p Actions if %p"),
				TEXT("Skips additional actions if the given condition is true. May skip negative amounts to go back."),
				{ TYPE_INT, TYPE_BOOL },
				&Thread::ConditionalJump
			},
			{
				ACTION_LOOP, ACTIONCAT_FLOW_CONTROL, TEXT("Loop"), TEXT("Loop"),
				TEXT("Loops the subactions. Can be exited using break or repeated by using continue actions"),
				{ },
				&Thread::ShouldNotBeImplemented
			},
			{
				ACTION_CONDBREAK, ACTIONCAT_FLOW_CONTROL, TEXT("Break If"), TEXT("Break If %p"),
				TEXT("Breaks out of the enclosing Loop action if the given condition is true. No effect if no loop is around."),
				{ TYPE_BOOL },
				&Thread::ShouldNotBeImplemented
			},
			{
				ACTION_CONDCONTINUE, ACTIONCAT_FLOW_CONTROL, TEXT("Continue If"), TEXT("Continue If %p"),
				TEXT("Goes back to the loop header of the enclosing Loop action if the given condition is true. No effect if no loop is around."),
				{ TYPE_BOOL },
				&Thread::ShouldNotBeImplemented
			},
			{
				ACTION_FORLOOP, ACTIONCAT_FLOW_CONTROL, TEXT("For Loop"), TEXT("For %p from %p till %p (exclusive)"),
				TEXT("Sets the given integer variable to the start value, then loops the subactions, increasing the variable by one after each pass, "
				"until the integer is greater or equal to the target value. Break and Continue may be used."),
				{ TYPE_INVALID, TYPE_INT, TYPE_INT },
				&Thread::ShouldNotBeImplemented
			},
			{
				10, ACTIONCAT_MODIFY_CARD, TEXT("Switch Style"), TEXT("%p ::Style = %p"), TEXT("Switches current character style."),
				{ TYPE_INT, TYPE_INT },
				&Thread::SwitchCardStyle
			},

			{
				11, ACTIONCAT_FLOW_CONTROL, TEXT("End Execution"), TEXT("End"), TEXT("ends execution of this thread. think of a return statement."),
				{ },
				&Thread::EndExecution
			},
			{
				12, ACTIONCAT_MODIFY_CHARACTER, TEXT("Add Love Points"), TEXT("%p ::AddLOVE( towards: %p , amount: %p )"),
				TEXT("Adds a certain amount of love points. 30 love points become one love interaction. A character can have up to 30 interactions "
				"in total; after that, earlier interactions will be replaced."),
				{ TYPE_INT, TYPE_INT, TYPE_INT },
				&Thread::AddCardLovePoints
			},
			{
				13, ACTIONCAT_MODIFY_CHARACTER, TEXT("Add Like Points"), TEXT("%p ::AddLIKE( towards: %p , points: %p )"),
				TEXT("Adds a certain amount of like points. 30 like points become one like interaction. A character can have up to 30 interactions "
				"in total; after that, earlier interactions will be replaced."),
				{ TYPE_INT, TYPE_INT, TYPE_INT },
				&Thread::AddCardLikePoints
			},
			{
				14, ACTIONCAT_MODIFY_CHARACTER, TEXT("Add Dislike Points"), TEXT("%p ::AddDISLIKE( towards: %p , points: %p )"),
				TEXT("Adds a certain amount of dislike points. 30 dislike points become one dislike interaction. A character can have up to 30 interactions "
				"in total; after that, earlier interactions will be replaced."),
				{ TYPE_INT, TYPE_INT, TYPE_INT },
				&Thread::AddCardDislikePoints
			},
			{
				15, ACTIONCAT_MODIFY_CHARACTER, TEXT("Add Hate Points"), TEXT("%p ::AddHATE( towards: %p , points: %p )"),
				TEXT("Adds a certain amount of hate points. 30 hate points become one hate interaction. A character can have up to 30 interactions "
				"in total; after that, earlier interactions will be replaced."),
				{ TYPE_INT, TYPE_INT, TYPE_INT },
				&Thread::AddCardHatePoints
			},
			{
				16, ACTIONCAT_MODIFY_CHARACTER, TEXT("Add Points"), TEXT("%p ::AddPoints( towards: %p , %p points: %p )"),
				TEXT("Adds a certain amount of points. Point type is between 0 or 3, or use one of the named constants. "
				"30 hate points become one hate interaction. A character can have up to 30 interactions "
				"in total; after that, earlier interactions will be replaced."
				"Example: "
					"{ TriggerCard ::AddPoints( towards: ThisCard , LOVE points: 30 ) }"),
				{ TYPE_INT, TYPE_INT, TYPE_INT, TYPE_INT },
				&Thread::AddCardPoints
			},
			{
				17, ACTIONCAT_FLOW_CONTROL, TEXT("Conditional End Execution"), TEXT("End Thread If %p"),
				TEXT("ends execution of this thread if the given condition evaluates to true."),
				{ TYPE_BOOL },
				&Thread::ConditionalEndExecution
			},
			{
				18, ACTIONCAT_EVENT, TEXT("Set Npc Current Response Answer"), TEXT("CurrentResponseAnswer = %p"),
				TEXT("When executed with a Npc Answers Event, this can be used to modify the answer the character will do."),
				{ TYPE_BOOL },
				&Thread::SetNpcResponseAnswer
			},
			{
				19, ACTIONCAT_EVENT, TEXT("Set Npc Current Response Percent"), TEXT("CurrentResponsePercent = %p"),
				TEXT("When executed with a Npc Answers Event, this can be used to modify the success percentage showed. Note that changing this value "
				"does not influence the Nps Answer, as it has allready been made. This Action only modifies the Percentage displayed in the UI."),
				{ TYPE_INT },
				&Thread::SetNpcResponsePercent
			},
			{
				20, ACTIONCAT_NPCACTION, TEXT("Make Npc Move to Room"), TEXT("%p ::GoTo( %p )"),
				TEXT("If the target character is controlled by the Computer, this Action makes them walk to the specified Room. "
				"If the Character is allready walking somewhere, it will do this instead. "
				"Keep in mind that executing this Action will throw an event next tick; watch out for endless loops"),
				{ TYPE_INT, TYPE_INT },
				&Thread::NpcMoveRoom
			},
			{
				21, ACTIONCAT_NPCACTION, TEXT("Make Npc do Action with no Target"), TEXT("%p ::Do(action: %p )"),
				TEXT("If the target character is controlled by the Computer, this Action makes them do an Action that does not require another character to execute. "
				"If the Character is allready walking somewhere, it will do this instead. "
				"Keep in mind that executing this Action will throw an event next tick; watch out for endless loops"),
				{ TYPE_INT, TYPE_INT },
				&Thread::NpcActionNoTarget
			},
			{
				22, ACTIONCAT_NPCACTION, TEXT("Make Npc Talk to Character"), TEXT("%p ::Do(action: %p , to: %p )"),
				TEXT("If the target character is controlled by the Computer, this Action makes them walk to and start the given conversation with the target. "
				"If the Character is allready walking somewhere, it will do this instead. "
				"Keep in mind that executing this Action will throw an event next tick; watch out for endless loops"),
				{ TYPE_INT, TYPE_INT, TYPE_INT },
				&Thread::NpcTalkTo
			},
			{
				23, ACTIONCAT_NPCACTION, TEXT("Make Npc Talk to Character about someone"), TEXT("%p ::Do(action: %p , to: %p , about: %p )"),
				TEXT("If the target character is controlled by the Computer, this Action makes them walk to and start the given conversation with the target about "
				"another character in class, such as asking for their opinion or spreading bad rumors. "
				"If the Character is allready walking somewhere, it will do this instead. "
				"Keep in mind that executing this Action will throw an event next tick; watch out for endless loops"),
				{ TYPE_INT, TYPE_INT, TYPE_INT, TYPE_INT },
				&Thread::NpcTalkToAbout
			},
			{
				24, ACTIONCAT_MODIFY_CARD, TEXT("Set Card Storage Integer"), TEXT("%p ::SetInt( %p ) = %p "),
				TEXT("Sets an entry in the cards storage. The card storage stores key-value pairs and is persistent between saves and loads. "
				"Note that the keys are shared between value types, so that for example a given key can not hold both an int and a string. "
				"When the key is allready in use, the function will silently fail."),
				{ TYPE_INT, TYPE_STRING, TYPE_INT },
				&Thread::SetCardStorageInt
			},
			{
				25, ACTIONCAT_MODIFY_CARD, TEXT("Set Card Storage Float"), TEXT("%p ::SetFloat( %p ) = %p "),
				TEXT("Sets an entry in the cards storage. The card storage stores key-value pairs and is persistent between saves and loads. "
				"Note that the keys are shared between value types, so that for example a given key can not hold both an int and a string. "
					"When the key is allready in use, the function will silently fail."),
					{ TYPE_INT, TYPE_STRING, TYPE_FLOAT },
					&Thread::SetCardStorageFloat
			},
			{
				26, ACTIONCAT_MODIFY_CARD, TEXT("Set Card Storage String"), TEXT("%p ::SetString( %p ) = %p "),
				TEXT("Sets an entry in the cards storage. The card storage stores key-value pairs and is persistent between saves and loads. "
				"Note that the keys are shared between value types, so that for example a given key can not hold both an int and a string. "
					"When the key is allready in use, the function will silently fail."),
					{ TYPE_INT, TYPE_STRING, TYPE_STRING },
					&Thread::SetCardStorageString
			},
			{
				27, ACTIONCAT_MODIFY_CARD, TEXT("Set Card Storage Bool"), TEXT("%p ::SetBool( %p ) = %p "),
				TEXT("Sets an entry in the cards storage. The card storage stores key-value pairs and is persistent between saves and loads. "
				"Note that the keys are shared between value types, so that for example a given key can not hold both an int and a string. "
					"When the key is allready in use, the function will silently fail."),
					{ TYPE_INT, TYPE_STRING, TYPE_BOOL },
				&Thread::SetCardStorageBool
			},
			{
				28, ACTIONCAT_MODIFY_CARD, TEXT("Remove Card Storage Integer"), TEXT("%p ::DropInt( %p ) "),
				TEXT("Removes an entry from the cards storage. If the given entry exists, but does not contain an int, this function will fail."),
				{ TYPE_INT, TYPE_STRING },
				&Thread::RemoveCardStorageInt
			},
			{
				29, ACTIONCAT_MODIFY_CARD, TEXT("Remove Card Storage Float"), TEXT("%p ::DropFloat( %p ) "),
				TEXT("Removes an entry from the cards storage. If the given entry exists, but does not contain a float, this function will fail."),
				{ TYPE_INT, TYPE_STRING },
				&Thread::RemoveCardStorageFloat
			},
			{
				30, ACTIONCAT_MODIFY_CARD, TEXT("Remove Card Storage String"), TEXT("%p ::DropString( %p ) "),
				TEXT("Removes an entry from the cards storage. If the given entry exists, but does not contain a string, this function will fail."),
				{ TYPE_INT, TYPE_STRING },
				&Thread::RemoveCardStorageString
			},
			{
				31, ACTIONCAT_MODIFY_CARD, TEXT("Remove Card Storage Bool"), TEXT("%p ::DropBool( %p ) "),
				TEXT("Removes an entry from the cards storage. If the given entry exists, but does not contain a bool, this function will fail."),
				{ TYPE_INT, TYPE_STRING },
				&Thread::RemoveCardStorageBool
			},
			{
				32, ACTIONCAT_MODIFY_CHARACTER, TEXT("Set Virtue"), TEXT("%p ::Virtue = %p"),
				TEXT("Set selected character's virtue. "
				"0- lowest, 1 - low, 2 - normal, 3 - high, 4 - highest."),
				{ TYPE_INT, TYPE_INT },
				&Thread::SetCardVirtue
			},
			{
				33, ACTIONCAT_MODIFY_CHARACTER, TEXT("Set Trait"), TEXT("%p ::Trait( %p ) = %p"),
				TEXT("Enable or disable selected character's trait."),
				{ TYPE_INT, TYPE_INT, TYPE_BOOL },
				&Thread::SetCardTrait
			},
			{
				34, ACTIONCAT_MODIFY_CHARACTER, TEXT("Set Personality"), TEXT("%p ::Personality = %p"),
				TEXT("Set character's personality."),
				{ TYPE_INT, TYPE_INT },
				&Thread::SetCardPersonality
			},
			{
				35, ACTIONCAT_MODIFY_CHARACTER, TEXT("Set Voice Pitch"), TEXT("%p ::Pitch = %p"),
				TEXT("Set character's voice pitch."),
				{ TYPE_INT, TYPE_INT },
				&Thread::SetCardVoicePitch
			},
			{
				36, ACTIONCAT_MODIFY_CHARACTER, TEXT("Set Club"), TEXT("%p ::Club = %p"),
				TEXT("Set character's club."),
				{ TYPE_INT, TYPE_INT },
				&Thread::SetCardClub
			},
			{
				37, ACTIONCAT_MODIFY_CHARACTER, TEXT("Set Club Value"), TEXT("%p ::ClubValue = %p"),
				TEXT("Set character's club value."),
				{ TYPE_INT, TYPE_INT },
				&Thread::SetCardClubValue
			},
			{
				38, ACTIONCAT_MODIFY_CHARACTER, TEXT("Set Club Rank"), TEXT("%p ::ClubRank = %p"),
				TEXT("Set character's club rank."),
				{ TYPE_INT, TYPE_INT },
				&Thread::SetCardClubRank
			},
			{
				39, ACTIONCAT_MODIFY_CHARACTER, TEXT("Set Intelligence"), TEXT("%p ::Intelligence = %p"),
				TEXT("Set character's intelligence."),
				{ TYPE_INT, TYPE_INT },
				&Thread::SetCardIntelligence
			},
			{
				40, ACTIONCAT_MODIFY_CHARACTER, TEXT("Set Intelligence Value"), TEXT("%p ::IntelligenceValue = %p"),
				TEXT("Set character's intelligence value."),
				{ TYPE_INT, TYPE_INT },
				&Thread::SetCardIntelligenceValue
			},
			{
				41, ACTIONCAT_MODIFY_CHARACTER, TEXT("Set Intelligence Rank"), TEXT("%p ::IntelligenceRank = %p"),
				TEXT("Set character's intelligence rank."),
				{ TYPE_INT, TYPE_INT },
				&Thread::SetCardIntelligenceRank
			},
			{
				42, ACTIONCAT_MODIFY_CHARACTER, TEXT("Set Strength"), TEXT("%p ::Strength = %p"),
				TEXT("Set character's strength."),
				{ TYPE_INT, TYPE_INT },
				&Thread::SetCardStrength
			},
			{
				43, ACTIONCAT_MODIFY_CHARACTER, TEXT("Set Strength Value"), TEXT("%p ::StrengthValue = %p"),
				TEXT("Set character's strength value."),
				{ TYPE_INT, TYPE_INT },
				&Thread::SetCardStrengthValue
			},
			{
				44, ACTIONCAT_MODIFY_CHARACTER, TEXT("Set Strength Rank"), TEXT("%p ::StrengthRank = %p"),
				TEXT("Set character's strength rank."),
				{ TYPE_INT, TYPE_INT },
				&Thread::SetCardStrengthRank
			},
			{
				45, ACTIONCAT_MODIFY_CHARACTER, TEXT("Set Sociability"), TEXT("%p ::Sociability = %p"),
				TEXT("Set character's sociability."),
				{ TYPE_INT, TYPE_INT },
				&Thread::SetCardSociability
			},
			{
				46, ACTIONCAT_MODIFY_CHARACTER, TEXT("Set First Name"), TEXT("%p ::FirstName = %p"),
				TEXT("Set character's First Name."),
				{ TYPE_INT, TYPE_STRING },
				&Thread::SetCardSecondName
			},
			{
				47, ACTIONCAT_MODIFY_CHARACTER, TEXT("Set Last Name"), TEXT("%p ::LastName = %p"),
				TEXT("Set character's Last Name."),
				{ TYPE_INT, TYPE_STRING },
				&Thread::SetCardFirstName
			},
			{
				48, ACTIONCAT_MODIFY_CHARACTER, TEXT("Set Sex Orientation"), TEXT("%p ::Orientation = %p"),
				TEXT("Set character's sexual orientation."),
				{ TYPE_INT, TYPE_INT },
				&Thread::SetCardOrientation
			},
			{
				49, ACTIONCAT_MODIFY_CHARACTER, TEXT("Set Description"), TEXT("%p ::Description = %p"),
				TEXT("Set character's description."),
				{ TYPE_INT, TYPE_STRING },
				&Thread::SetCardDescription
			},
			{
				50, ACTIONCAT_GENERAL, TEXT("Change Player Character"), TEXT("%p ::SetPC"),
				TEXT("Change current Player Character."),
				{ TYPE_INT },
				&Thread::SetPC
			},
			{
				51, ACTIONCAT_GENERAL, TEXT("Start H scene"), TEXT("Start H(pc: %p , partner: %p )"),
				TEXT("Start H scene between 2 characters"),
				{ TYPE_INT, TYPE_INT },
				&Thread::StartHScene
			},
			{
				52, ACTIONCAT_MODIFY_CHARACTER, TEXT("Set Sex Experience: Vaginal"), TEXT("%p ::SexXP = %p"),
				TEXT("Set vaginal experience for the character"),
				{ TYPE_INT, TYPE_BOOL },
				&Thread::SetCardSexExperience
			},
			{
				53, ACTIONCAT_MODIFY_CHARACTER, TEXT("Set Sex Experience: Anal"), TEXT("%p ::SexXP = %p"),
				TEXT("Set anal experience for the character"),
				{ TYPE_INT, TYPE_BOOL },
				&Thread::SetCardAnalSexExperience
			},
			{
				54, ACTIONCAT_GENERAL, TEXT("Add Mood"), TEXT("%p ::AddMood(mood: %p , strength: %p"),
				TEXT("Add mood strength."),
				{ TYPE_INT, TYPE_INT, TYPE_INT },
				&Thread::AddMood
			},
			{
				55, ACTIONCAT_GENERAL, TEXT("Replace Mood"), TEXT("%p ::ReplaceMood(mood: %p , with mood: %p , strength: %p"),
				TEXT("Replace mood 1 with mood 2 up to strength."),
				{ TYPE_INT, TYPE_INT, TYPE_INT, TYPE_INT },
				&Thread::ReplaceMood
			},
			{
				56, ACTIONCAT_MODIFY_CARD, TEXT("Set Item - Lover's"), TEXT("%p ::LoverItem = %p"),
				TEXT("Rename Lover's item."),
				{ TYPE_INT, TYPE_STRING },
				&Thread::SetCardLoversItem
			},
			{
				57, ACTIONCAT_MODIFY_CARD, TEXT("Set Item - Friend's"), TEXT("%p ::FriendItem = %p"),
				TEXT("Rename Friend's item."),
				{ TYPE_INT, TYPE_STRING },
				&Thread::SetCardFriendItem
			},
			{
				58, ACTIONCAT_MODIFY_CARD, TEXT("Set Item - Sexual"), TEXT("%p ::SexualItem = %p"),
				TEXT("Rename Sexual item."),
				{ TYPE_INT, TYPE_STRING },
				&Thread::SetCardSexualItem
			},
			{
				59, ACTIONCAT_MODIFY_CARD, TEXT("Set H Compatibility"), TEXT("%p ::Compatibility( %p ) = %p"),
				TEXT("Set card's H compatibility with the selected character. 0-999 values"),
				{ TYPE_INT, TYPE_STRING },
				&Thread::SetCardSexCompatibility
			},
			{
				60, ACTIONCAT_NPCACTION, TEXT("Set NPC status"), TEXT("%p ::NpcStatus = %p"),
				TEXT("Set NPC status. 0=still, 1=settle in location, 2=move to location, 3=walk to character, 4=follow, 7=talk, 8=minna"),
				{ TYPE_INT, TYPE_INT },
				&Thread::SetNpcStatus
			},
			{
				61, ACTIONCAT_NPCACTION, TEXT("Voyeur Clean Up"), TEXT("Voyeur Clean Up"),
				TEXT("Cleans up after voyeur sex"),
				{ },
				&Thread::ResetVoyeur
			},
			{
				62, ACTIONCAT_MODIFY_CHARACTER, TEXT("Set Points"), TEXT("%p ::SetPoints( towards: %p , LOVE: %p , LIKE: %p , DISLIKE: %p , HATE: %p , SPARE: %p )"),
				TEXT("Sets the full set of relationship points. The points are normalized, meaning you don't have to have them add up to 900 - the action will do it for you using the values as weights.\nIf you do have them add up to 900.0 they would be aplied as you provide them, minus the decimals."),
				{ TYPE_INT, TYPE_INT, TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT },
				&Thread::SetCardPoints
			},
		};




		ParameterisedAction::ParameterisedAction(DWORD actionId, const std::vector<ParameterisedExpression>& params) {
			this->action = Action::FromId(actionId);
			this->actualParameters = params;
		}


	}
}
