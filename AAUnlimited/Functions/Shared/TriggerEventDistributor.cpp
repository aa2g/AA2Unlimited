#include "StdAfx.h"




namespace Shared {

	namespace Triggers {

		namespace STUPIDNAME {
			std::vector<Trigger*> loc_triggers[N_EVENTS];
		}

		using namespace STUPIDNAME;

		void RegisterTrigger(Triggers::Trigger* trg) {
			for (auto& elem : trg->events) {
				loc_triggers[elem.event->id - 1].push_back(trg);
			}

		}

		void UnregisterTrigger(Triggers::Trigger* trg) {
			for (auto& elem : trg->events) {
				int index = elem.event->id - 1;
				for (auto it = loc_triggers[index].begin(); it != loc_triggers[index].end(); it++) {
					if (*it == trg) {
						loc_triggers[index].erase(it);
						break;
					}
				}
			}
		}

		void ThrowEvent(EventData* data) {
			if (g_Config.bTriggers) {
				for (auto& trigger : loc_triggers[data->GetId() - 1]) {
					Thread thread;
					thread.eventData = data;
					thread.ExecuteTrigger(trigger);
				}
			}
		}


	}


}