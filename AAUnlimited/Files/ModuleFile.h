#pragma once

#include <Windows.h>
#include <string>

#include "Functions\Shared\Triggers\Triggers.h"

/*
 * Describes a module, a set of triggers with information.
 * This is a binary file and not supposed to be editable in an editor (though it of course is if you know what youre doing)
 */
class ModuleFile {
public:
	ModuleFile(const std::wstring& name,const std::wstring description,const std::vector<Shared::Triggers::Trigger*>& triggers);
	std::wstring name;
	std::wstring description;
	std::vector<Shared::Triggers::Trigger> triggers;

	void FromFile(const TCHAR* path);
	void WriteToFile(const TCHAR* path);

	inline bool IsGood() { return good; }
private:
	bool good;


};