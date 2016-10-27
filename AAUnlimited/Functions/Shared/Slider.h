#pragma once

#include <string>

#include "External\ExternalClasses\CharacterStruct.h"
#include "Functions\AAUCardData.h"

namespace Shared {


/*
 * Defines a slider functionality
 */
struct Slider {

	ExtClass::CharacterStruct::Models target; //xx file it affects
	int index; //index of this slider in the array
	std::wstring boneName; //bone if affects
	AAUCardData::BoneMod mod; //used as a mask by multiplying
	/*enum Operation {
		ADDITIVE, MULTIPLICATIVE
	} op;*/

	inline float GetNeutralValue() const { 
		/*switch(op) {
		case ADDITIVE:
			return 0;
		case MULTIPLICATIVE:
			return 1;
		default:
			return 0;
		}*/
		return 0;
	}
};

const std::vector<Slider> g_sliders[ExtClass::CharacterStruct::N_MODELS] = {
	{
		//FACE
		{ExtClass::CharacterStruct::FACE, 0, TEXT("A00_J_kuti"), {1,0,0, 0,0,0, 0,0,0}/*, Slider::ADDITIVE*/} //mouth
	}
};





}
