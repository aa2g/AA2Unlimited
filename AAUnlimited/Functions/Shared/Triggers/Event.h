#pragma once

#include "Value.h"
#include "Expressions.h"

#include <vector>

namespace Shared {
namespace Triggers {



	/*
	* An Event. Certain actions in the game fire certain events.
	*/
	class Event {
	public:
		DWORD id;
		std::wstring name;
		std::wstring description;
		std::vector<Types> parameters;

		static const Event* FromId(int id);
	};

	enum Events {
		INVALID = 0,
		CLOTHES_CHANGED, CARD_INITIALIZED, CARD_DESTROYED,
		HI_POLY_INIT, HI_POLY_END, 
		
		PERIOD_ENDS,
		
		N_EVENTS
	};

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
		inline int GetId() const { return id; }
		virtual void SetThreadStorage(Thread* thread) const = 0;
	};

#define EDC_DECLARE(name,enumname) class name : public EventData { \
											public: \
												inline name() { id = enumname; } \
												void SetThreadStorage(Thread* thread) const;
#define EDC_END };

	EDC_DECLARE(ClothesChangedData,CLOTHES_CHANGED)
		int card;
	EDC_END

	EDC_DECLARE(CardInitializeData,CARD_INITIALIZED)
		int card;
	EDC_END

	EDC_DECLARE(CardDestroyData,CARD_DESTROYED)
		int card;
	EDC_END

	EDC_DECLARE(HiPolyInitData,HI_POLY_INIT)
		int card;
	EDC_END

	EDC_DECLARE(HiPolyEndData,HI_POLY_END)
		int card;
	EDC_END

	EDC_DECLARE(PeriodEndsData,HI_POLY_END)
		int currentPeriod;
	EDC_END


#undef EDC_DECLARE
#undef EDC_END

}
}