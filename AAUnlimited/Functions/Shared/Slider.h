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
		{ExtClass::CharacterStruct::FACE, 0, TEXT("A00_J_kuti"), {1,0,0, 0,0,0, 0,0,0}}, //mouth
		{ExtClass::CharacterStruct::FACE, 1, TEXT("A00_O_mimi"),{ 0,0,0, 0,0,0, 0,1,0 }}, //ear height
		{ExtClass::CharacterStruct::FACE, 2, TEXT("A00_J_kuti"),{ 0,0,0, 0,0,0, 0,1,0.9f }} //mouth height
	},
	{
		//SKELETON
		{ ExtClass::CharacterStruct::SKELETON, 0, TEXT("SCENE_ROOT"),{ 0,1,0, 0,0,0, 0,0,0 } }, //body height,
		{ ExtClass::CharacterStruct::SKELETON, 1, TEXT("SCENE_ROOT"),{ 1,0,0, 0,0,0, 0,0,0 } }, //body width,
		{ ExtClass::CharacterStruct::SKELETON, 2, TEXT("SCENE_ROOT"),{ 0,0,1, 0,0,0, 0,0,0 } } //body thickness
	},
	{
		//BODY
	},
	{
		//HAIR_FRONT
	},
	{
		//HAIR_SIDE
	},
	{
		//HAIR_BACK
	},
	{
		//HAIR_EXT
	},
	{
		//FACE_SLIDERS
		
	}
};





}
