#pragma once

#include "Value.h"
#include "Expressions.h"
#include "InfoData.h"

#include <vector>

namespace Shared {
namespace Triggers {



	/*
	* An Event. Certain actions in the game fire certain events.
	*/
	class Event/* : public InfoData*/{
	public:
		DWORD id;								//a unique identifier. This id is only unique inside the class it is used in, not accross all eev's
		int category;							//category is a string that is appended in the gui for easier navigation
		std::wstring name;						//a name, visible from the dropdown menu
		std::wstring interactiveName;			//name in the gui; parameters are replaced by %ps and can be clicked to be changed
		std::wstring description;				//description
		std::vector<Types> parameters;

		static const Event* FromId(int id);
	};

	enum EventCategories {
		EVENTCAT_CARD,
		EVENTCAT_CHARACTION,
		EVENTCAT_LOADS,
		EVENTCAT_TIME,
		EVENTCAT_CONVERSATION,
		EVENTCAT_N
	};

	enum Events {
		INVALID = 0,
		CLOTHES_CHANGED, CARD_INITIALIZED, CARD_DESTROYED,
		HI_POLY_INIT, HI_POLY_END, 
		CARD_ADDED,
		PERIOD_ENDS,
		NPC_RESPONSE,

		
		
		N_EVENTS
	};

	extern std::wstring g_EventCategories[EVENTCAT_N];
	extern std::vector<Event> g_Events;

	class ParameterisedEvent {
	public:
		const Event* event;
		std::vector<ParameterisedExpression> actualParameters;

		inline ParameterisedEvent() : event(NULL) {}

	};



	inline const Event* Event::FromId(int id) {
		if (id < 1) return NULL;
		return &g_Events[id-1];
	}

	/*
	 * Stores data provided by events. Every event has its own derived version.
	 */
	class EventData {
	protected:
		int id;
	public:
		int card;
		inline int GetId() const { return id; }
	};

#define EDC_DECLARE(name,enumname) class name : public EventData { \
											public: \
												inline name() { id = enumname; } 
#define EDC_END };

	EDC_DECLARE(ClothesChangedData,CLOTHES_CHANGED)
		
	EDC_END

	EDC_DECLARE(CardInitializeData,CARD_INITIALIZED)
		
	EDC_END

	EDC_DECLARE(CardDestroyData,CARD_DESTROYED)
		
	EDC_END

	EDC_DECLARE(CardAddedData,CARD_ADDED)
		
	EDC_END

	EDC_DECLARE(HiPolyInitData,HI_POLY_INIT)
		
	EDC_END

	EDC_DECLARE(HiPolyEndData,HI_POLY_END)
		
	EDC_END

	EDC_DECLARE(PeriodEndsData,PERIOD_ENDS)
		int currentPeriod;
	EDC_END

	EDC_DECLARE(NpcResponseData,NPC_RESPONSE)
		int answeredTowards;
		int conversationId;
		bool originalResponse;
		bool changedResponse;
	EDC_END


#undef EDC_DECLARE
#undef EDC_END

}
}