#include "Event.h"

#include "Thread.h"

namespace Shared {
namespace Triggers {




	std::vector<Event> g_Events = {
		{ CLOTHES_CHANGED, TEXT("Clothes Changed"), 
			TEXT("Whenever a character changes clothes, either in the changing room or by entering the next period."), 
			{}
		},
		{ CARD_INITIALIZED, TEXT("Card Initialized"), 
			TEXT("After the AAU Card data was successfully loaded, either because a class containing this card was loaded, or because "
				 "this card was added to the class roster"),
			{} 
		},
		{ CARD_DESTROYED, TEXT("Card Destroyed"), 
			TEXT("Before the AAU Card data will be unloaded"),
			{} 
		},
		{ HI_POLY_INIT, TEXT("Hi Poly Model Initialized"),
			TEXT("After the Characters High Poly Model was loaded"),
			{}
		},
		{ HI_POLY_END, TEXT("Hi Poly Model Destroyed"),
			TEXT("Before the Characters High Poly Model will be unloaded"),
			{}
		}
	
	
	};

	void ClothesChangedData::SetThreadStorage(Thread* thread) const {
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
}
}