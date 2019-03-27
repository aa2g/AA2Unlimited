#include "StdAfx.h"

namespace Shared {
	namespace Triggers {


		std::wstring g_EventCategories[EVENTCAT_N] = {
			TEXT("Card"),
			TEXT("Character Action"),
			TEXT("Load"),
			TEXT("Time"),
			TEXT("Conversation")
		};

		std::vector<Event> g_Events = {
			{ CLOTHES_CHANGED, EVENTCAT_CHARACTION, TEXT("Clothes Changed"), TEXT("Clothes Changed"),
				TEXT("Whenever a character changes clothes, either in the changing room or by entering the next period. (Not yet implemented)"),
				{}
			},
			{ CARD_INITIALIZED, EVENTCAT_CARD, TEXT("Card Initialized"), TEXT("Card Initialized"),
				TEXT("After the AAU Card data was successfully loaded, either because a class containing this card was loaded, or because "
					 "this card was added to the class roster"),
				{}
			},
			{ CARD_DESTROYED, EVENTCAT_CARD, TEXT("Card Destroyed"), TEXT("Card Destroyed"),
				TEXT("Before the AAU Card data will be unloaded"),
				{}
			},
			{ HI_POLY_INIT, EVENTCAT_LOADS, TEXT("Hi Poly Model Initialized"), TEXT("Hi Poly Model Initialized"),
				TEXT("Right after the Characters High Poly Model started loading"),
				{}
			},
			{ HI_POLY_END, EVENTCAT_LOADS, TEXT("Hi Poly Model Loaded"), TEXT("Hi Poly Model Loaded"),
				TEXT("Right after the Character High Poly Model finished loading"),
				{}
			},
			{ CARD_ADDED, EVENTCAT_CARD, TEXT("Card Added to Class"), TEXT("Card Added to Class"),
				TEXT("When a card (including this card) is added to a class, after the CARD_INTIALIZED event is executed"),
				{}
			},
			{ PERIOD_ENDS, EVENTCAT_TIME, TEXT("A Period Ends"), TEXT("A Period Ends"),
				TEXT("After a period ends, including lessions themselves"),
				{}
			},
			{ NPC_RESPONSE, EVENTCAT_CONVERSATION, TEXT("Npc Answers in a Conversation"), TEXT("Npc Answers in a Conversation"),
				TEXT("Whenever a NPC made a yes/no decision, no matter if towards the PC or another NPC. Triggering Card is the NPC that gives the Answer."),
				{}
			},
			{ NPC_WALK_TO_ROOM, EVENTCAT_CONVERSATION, TEXT("Npc Starts Walking to a Room"), TEXT("Npc Starts Walking to a Room"),
				TEXT("Whenever a NPC decides to walk towards a room."),
				{}
			},
			{ NPC_WANT_ACTION_NOTARGET, EVENTCAT_CONVERSATION, TEXT("Npc Wants to do something with no Target"), TEXT("Npc Wants to do something with no Target"),
				TEXT("Whenever a NPC decides to do an action that does not require another Npc."),
				{}
			},
			{ NPC_WANT_TALK_WITH, EVENTCAT_CONVERSATION, TEXT("Npc Wants to Talk With Someone"), TEXT("Npc Wants to Talk With Someone"),
				TEXT("Whenever a NPC decides to talk to someone."),
				{}
			},
			{ NPC_WANT_TALK_WITH_ABOUT, EVENTCAT_CONVERSATION, TEXT("Npc Wants to Talk With Someone About Someone"), TEXT("Npc Wants to Talk With Someone About Someone"),
				TEXT("Whenever a NPC decides to talk to someone about someone else, such as spreading rumors or asking for opinion about someone."),
				{}
			},
			{ PC_CONVERSATION_STATE_UPDATED, EVENTCAT_CONVERSATION, TEXT("PC conversation state updated"), TEXT("PC Conversation state updated"),
				TEXT("Whenever PC conversation has moved on."),
				{}
			},
			{ PC_RESPONSE, EVENTCAT_CONVERSATION, TEXT("Pc Answers in a Conversation"), TEXT("Pc Answers in a Conversation"),
				TEXT("Right before the PC answer is processed. Triggering card is the PC"),
				{}
			},
			{ PC_CONVERSATION_LINE_UPDATED, EVENTCAT_CONVERSATION, TEXT("PC conversation line updated"), TEXT("PC Conversation line updated"),
				TEXT("Whenever NPC speaks a line of dialogue in PC conversation."),
				{}
			},
			{ ROOM_CHANGE, EVENTCAT_CONVERSATION, TEXT("Card Changes Room"), TEXT("Card Changes Room"),
			TEXT("Card enters a new room. Works for all cards."),
				{}
			},
			{ KEY_PRESS, EVENTCAT_CONVERSATION, TEXT("Key Press"), TEXT("Key was pressed"),
			TEXT("Key was pressed"),
				{}
			},
			{ HPOSITION_CHANGE, EVENTCAT_CONVERSATION, TEXT("H Position Change"), TEXT("H Position Change"),
			TEXT("The H position changed."),
				{}
			},
			{ PC_AFTER_RESPONSE, EVENTCAT_CONVERSATION, TEXT("After PC Response"), TEXT("After PC Response"),
			TEXT("The event is fired after all the triggers have finished editing PC Response."),
				{}
			},
			{ NPC_AFTER_RESPONSE, EVENTCAT_CONVERSATION, TEXT("After NPC Response"), TEXT("After NPC Response"),
			TEXT("The event is fired after all the triggers have finished editing NPC Response."),
				{}
			},
			{ HI_POLY_DESPAWN, EVENTCAT_LOADS, TEXT("HI Poly Despawn"), TEXT("HI Poly Despawn"),
			TEXT("The event is fired as the character's hi poly is despawning."),
				{}
			},
			{ H_END, EVENTCAT_LOADS, TEXT("H End"), TEXT("H Ends"),
			TEXT("The event is fired as the H scene ends."),
				{}
			},
			{ H_START, EVENTCAT_LOADS, TEXT("H Start"), TEXT("H Starts"),
			TEXT("The event is fired as the H scene starts."),
			{}
			}
		};

		/*void ClothesChangedData::SetThreadStorage(Thread* thread) const {
			thread->localStorage.triggeringCard = card;
		}

		void CardInitializeData::SetThreadStorage(Thread* thread) const {
			thread->localStorage.triggeringCard = card;
		}

		void CardDestroyData::SetThreadStorage(Thread* thread) const {
			thread->localStorage.triggeringCard = card;
		}

		void HiPolyInitData::SetThreadStorage(Thread* thread) const {
			thread->localStorage.triggeringCard = card;
		}

		void HiPolyEndData::SetThreadStorage(Thread* thread) const {
			thread->localStorage.triggeringCard = card;
		}

		void CardAddedData::SetThreadStorage(Thread* thread) const {
			thread->localStorage.triggeringCard = card;
		}

		void PeriodEndsData::SetThreadStorage(Thread* thread) const {
			thread->globalStorage.period = currentPeriod;
		}

		void NpcResponseData::SetThreadStorage(Thread* thread) const {

		}*/
	}
}