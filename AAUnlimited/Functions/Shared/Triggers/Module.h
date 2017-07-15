#pragma once

#include <string>

#include "Triggers.h"

namespace Shared {
namespace Triggers {


/*
 * A Module is a set of triggers
 */
class Module {
public:
	std::wstring name;
	std::wstring description;
	std::vector<Trigger> triggers;
	std::vector<GlobalVariable> globals;
	std::vector<std::wstring> dependencies;
	
	Module() = default;
	Module(std::wstring name,std::wstring descr,const std::vector<Trigger*>& triggers, 
		const std::vector<GlobalVariable>& environment, const std::vector<Module>& moduleEnvironment);

	//generates the globals field from existing globals by looking into the triggers and determining which globals are being used
	void GenerateTrigGlobalDepend(const std::vector<Trigger*>& triggers,const std::vector<GlobalVariable>& triggerGlobals, const std::vector<Module>& modules);
};


}
}
