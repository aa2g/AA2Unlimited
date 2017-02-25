#include "NamedConstant.h"

namespace Shared {
namespace Triggers {

std::wstring g_NamedConstantCategories[NCONSTCAT_N] {
	TEXT("Relationship")
};

std::vector<NamedConstant> g_NamedConstants[N_TYPES] = {
	{ //INVALID
		
	},
	{ //INT
		{
			1, NCONSTCAT_RELATIONSHIP, 
			TEXT("Love Points"), TEXT("LOVE"), TEXT("Love points"),
			Value(0)
		},
		{
			2, NCONSTCAT_RELATIONSHIP,
			TEXT("Like Points"), TEXT("LIKE"), TEXT("Like points"),
			Value(1)
		},
		{
			3, NCONSTCAT_RELATIONSHIP,
			TEXT("Dislike Points"), TEXT("DISLIKE"), TEXT("Dislike points"),
			Value(2)
		},
		{
			4, NCONSTCAT_RELATIONSHIP,
			TEXT("Hate Points"), TEXT("HATE"), TEXT("Hate points"),
			Value(3)
		},
	},
	{ //BOOL
		
	},
	{ //FLOAT
		
	},
	{ //STRING
		
	}

};






}
}