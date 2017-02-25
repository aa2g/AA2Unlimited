#pragma once

#include "Value.h"

#include <Windows.h>
#include <string>
#include <vector>

namespace Shared {
namespace Triggers {



class NamedConstant {
public:
	DWORD id;								//a unique identifier. This id is only unique inside the class it is used in, not accross all eev's
	int category;							//category is a string that is appended in the gui for easier navigation
	std::wstring name;						//a name, visible from the dropdown menu
	std::wstring interactiveName;			//name in the gui; parameters are replaced by %ps and can be clicked to be changed
	std::wstring description;				//description

	Value val;

	static const NamedConstant* NamedConstant::FromId(Types type,int id);
};

enum NamedConstantCategories {
	NCONSTCAT_RELATIONSHIP,
	NCONSTCAT_N
};

extern std::wstring g_NamedConstantCategories[NCONSTCAT_N];
extern std::vector<NamedConstant> g_NamedConstants[N_TYPES];

inline const NamedConstant* NamedConstant::FromId(Types type,int id) {
	if (id < 1) return NULL;
	return &g_NamedConstants[type][id-1];
}

}
}