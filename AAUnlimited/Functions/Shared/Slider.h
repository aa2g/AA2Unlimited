#pragma once

#include <string>

#include "Functions\Shared\Globals.h"
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
	enum Operation {
		ADD, MULTIPLY, DIVIDE
	} op;

	inline float GetNeutralValue() const { 
		switch(op) {
		case ADD:
			return 0;
		case MULTIPLY:
			return 1;
		default:
			return 0;
		}
		return 0;
	}

	inline AAUCardData::BoneMod GenerateModifier(float value) const {
		AAUCardData::BoneMod mod = this->mod;
		switch(op) {
		case MULTIPLY:
		case DIVIDE:
			for(int i = 0; i < 9; i++) {
				mod.data[i] *= value;
			}
			for (int i = 0; i < 3; i++) {
				if (mod.scales[i] == 0) mod.scales[i] = 1;
			}
			for (int i = 0; i < 3; i++) {
				if (mod.transformations[i] == 0) mod.transformations[i] = 1;
			}
			break;
		case ADD:
		default:
			for (int i = 0; i < 9; i++) mod.data[i] *= value;
			break;
		}
		return mod;
	}

	static void ModifySRT(D3DVECTOR3* scale,D3DVECTOR3* rot,D3DVECTOR3* trans, Slider::Operation op, const AAUCardData::BoneMod& mod);
	static void ModifyKeyframe(ExtClass::Keyframe* frame,Slider::Operation op,const AAUCardData::BoneMod& mod);
	static ExtClass::CharacterStruct::Models GetModelFromName(const char* name);
};

const std::vector<Slider> g_sliders[ExtClass::CharacterStruct::N_MODELS] = {
	{
		//FACE
		{ExtClass::CharacterStruct::FACE, 0, TEXT("A00_J_kuti"), {1,0,0, 0,0,0, 0,0,0}, Slider::ADD }, //mouth
		{ExtClass::CharacterStruct::FACE, 1, TEXT("A00_O_mimi"),{ 0,0,0, 0,0,0, 0,1,0 }, Slider::ADD }, //ear height
		{ExtClass::CharacterStruct::FACE, 2, TEXT("A00_J_kuti"),{ 0,0,0, 0,0,0, 0,1,0.9f }, Slider::ADD }, //mouth height
		{ExtClass::CharacterStruct::FACE, 3, TEXT("A00_J_mayuLrot"),{ 0,0,0, 0,0,0, 0,1,0 }, Slider::ADD }, //left eyebrow height
		{ExtClass::CharacterStruct::FACE, 4, TEXT("A00_J_mayuRrot"),{ 0,0,0, 0,0,0, 0,1,0 }, Slider::ADD } //right eyebrow height
	},
	{
		//SKELETON
		{ ExtClass::CharacterStruct::SKELETON, 0, TEXT("SCENE_ROOT"),{ 0,1,0, 0,0,0, 0,0,0 }, Slider::ADD }, //body height,
		{ ExtClass::CharacterStruct::SKELETON, 1, TEXT("SCENE_ROOT"),{ 1,0,0, 0,0,0, 0,0,0 }, Slider::ADD }, //body width,
		{ ExtClass::CharacterStruct::SKELETON, 2, TEXT("SCENE_ROOT"),{ 0,0,1, 0,0,0, 0,0,0 }, Slider::ADD }, //body thickness
		{ ExtClass::CharacterStruct::SKELETON, 3, TEXT("a01_J_ArmL_01"),{ 1,1,0, 0,0,0, 0,0,0 }, Slider::ADD }, //left arm thickness
		{ ExtClass::CharacterStruct::SKELETON, 4, TEXT("a01_J_ArmR_01"),{ 1,1,0, 0,0,0, 0,0,0 }, Slider::ADD }, //right arm thickness
		{ ExtClass::CharacterStruct::SKELETON, 5, TEXT("a01_J_HandL_01"),{ 0,0,1, 0,0,0, 0,0,0 }, Slider::ADD }, //left hand length
		{ ExtClass::CharacterStruct::SKELETON, 6, TEXT("a01_J_HandR_01"),{ 0,0,1, 0,0,0, 0,0,0 }, Slider::ADD }, //right hand length
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
		{ ExtClass::CharacterStruct::FACE_SLIDERS, 0, TEXT("A00_J_meC"),{ 0,0,0, 0,0,0, 0,0,1 }, Slider::ADD }, //eye depth?
	}
};

//info from thicker x/y mod
//note that their way of scaling seems to be to add additional bones with an _ofst suffix and giving them scales instead
//a01_J_Kosi_010: unterer körper, startet beim bauchnabel, endet bei beinen leicht unter beckenhöhe
//a01_J_Spin_020: oberer körper, startet am brustkorb und endet an hals und arm ansatz
//a01_J_SiriR_010|a01_J_SiriL_010: linker/rechter hintern. note that 20 is also part of it, but not modified in the mod
//a01_J_UplegL_010|a01_J_UplegR_010: linker/rechter oberschenkel. 20 affects the entire leg, interistingly



}
