#pragma once

#include <Windows.h>
#include <string>

#include "Functions\Shared\Triggers\Triggers.h"
#include "Functions\Shared\Triggers\Module.h"

/*
 * Describes a module, a set of triggers with information.
 * This is a binary file and not supposed to be editable in an editor (though it of course is if you know what youre doing)
 */
class ModuleFile {
public:
	inline ModuleFile(const TCHAR* path) { FromFile(path); }
	ModuleFile(const Shared::Triggers::Module& mod);
	Shared::Triggers::Module mod;


	void FromFile(const TCHAR* path);
	void WriteToFile(const TCHAR* path);

	char* ToBuffer(int& size, int& at);

	inline bool IsGood() { return good; }
private:
	bool good;


};