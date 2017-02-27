#pragma once

#include "Triggers\Triggers.h"
#include "Triggers\Event.h"

namespace Shared {

namespace Triggers {

void RegisterTrigger(Triggers::Trigger* trg);

void UnregisterTrigger(Triggers::Trigger* trg);

void ThrowEvent(Triggers::EventData* data);

}

}
