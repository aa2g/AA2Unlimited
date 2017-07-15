#pragma once

#include <Windows.h>
#include <string>


namespace Shared {
namespace Triggers {


/*
 * Info that is stored for every event/expression/action that helps identifying said event/expression/action.
 */

class InfoData {
public:
	DWORD id;								//a unique identifier. This id is only unique inside the class it is used in, not accross all eev's
	int category;							//category is a string that is appended in the gui for easier navigation
	std::wstring name;						//a name, visible from the dropdown menu
	std::wstring interactiveName;			//name in the gui; parameters are replaced by %ps and can be clicked to be changed
	std::wstring description;				//description
};


}
}