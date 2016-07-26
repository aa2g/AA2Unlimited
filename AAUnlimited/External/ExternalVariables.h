#pragma once

#include "ExternalVariables\AAEdit\WindowData.h"

namespace ExtVars {

inline void InitializeExtVars() {
	ExtVars::AAEdit::InitializeExternals();
}

}