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
	AAUCardData::MeshModFlag flags;

	inline float GetNeutralValue() const { 
		switch(op) {
		case ADD:
			return 0;
		case MULTIPLY:
		case DIVIDE:
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
		{ExtClass::CharacterStruct::FACE, 0, TEXT("A00_J_kuti"), {1,0,0, 0,0,0, 0,0,0}, Slider::ADD, AAUCardData::MODIFY_FRAME }, //mouth
		{ExtClass::CharacterStruct::FACE, 1, TEXT("A00_O_mimi"),{ 0,0,0, 0,0,0, 0,1,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //ear height
		{ExtClass::CharacterStruct::FACE, 2, TEXT("A00_J_kuti"),{ 0,0,0, 0,0,0, 0,1,0.9f }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //mouth height
		{ExtClass::CharacterStruct::FACE, 3, TEXT("A00_J_mayuLrot"),{ 0,0,0, 0,0,0, 0,1,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //left eyebrow height
		{ExtClass::CharacterStruct::FACE, 4, TEXT("A00_J_mayuRrot"),{ 0,0,0, 0,0,0, 0,1,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //right eyebrow height
		{ ExtClass::CharacterStruct::FACE, 5, TEXT("A00_J_meC"),{ 0,0,0, 0,0,0, 0,0,1 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //eye depth?
	},
	{
		//SKELETON
		{ ExtClass::CharacterStruct::SKELETON, 0, TEXT("SCENE_ROOT"),{ 0,1,0, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //body height,
		{ ExtClass::CharacterStruct::SKELETON, 1, TEXT("SCENE_ROOT"),{ 1,0,0, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //body width,
		{ ExtClass::CharacterStruct::SKELETON, 2, TEXT("SCENE_ROOT"),{ 0,0,1, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //body thickness
		{ ExtClass::CharacterStruct::SKELETON, 3, TEXT("a01_J_ArmL_01"),{ 1,0,1, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //left arm thickness
		{ ExtClass::CharacterStruct::SKELETON, 4, TEXT("a01_J_ArmR_01"),{ 1,0,1, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //right arm thickness
		{ ExtClass::CharacterStruct::SKELETON, 5, TEXT("a01_J_HandL_02"),{ 1,0,0, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //left hand fingers length
		{ ExtClass::CharacterStruct::SKELETON, 6, TEXT("a01_J_HandR_02"),{ 1,0,0, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //right hand fingers length
		{ ExtClass::CharacterStruct::SKELETON, 7, TEXT("a01_J_Kosi_010"),{ 1,0,0, 0,0,0, 0,0,0 }, Slider::MULTIPLY, AAUCardData::MODIFY_FRAME }, //kosi thickness
		{ ExtClass::CharacterStruct::SKELETON, 8, TEXT("a01_J_Kosi_010"),{ 0,0,1, 0,0,0, 0,0,0 }, Slider::MULTIPLY, AAUCardData::MODIFY_FRAME },
		{ ExtClass::CharacterStruct::SKELETON, 9, TEXT("a01_J_SiriL_010"),{ 1,0,0, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME },
		{ ExtClass::CharacterStruct::SKELETON,10, TEXT("a01_J_SiriR_010"),{ 1,0,0, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME },
		{ ExtClass::CharacterStruct::SKELETON,11, TEXT("a01_J_SiriL_010"),{ 0,0,1, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME },
		{ ExtClass::CharacterStruct::SKELETON,12, TEXT("a01_J_SiriR_010"),{ 0,0,1, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME },
		{ ExtClass::CharacterStruct::SKELETON,13, TEXT("a01_J_OyaL_01"),{ 1,0,0, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //left hand thumb length
		{ ExtClass::CharacterStruct::SKELETON,14, TEXT("a01_J_OyaR_01"),{ 1,0,0, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //right hand thumb length
		{ ExtClass::CharacterStruct::SKELETON,15, TEXT("a01_J_FootL_03"),{ 0,0,1, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //left foot length
		{ ExtClass::CharacterStruct::SKELETON,16, TEXT("a01_J_FootR_03"),{ 0,0,1, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //right foot length
		{ ExtClass::CharacterStruct::SKELETON,17, TEXT("a01_J_FootL_03"),{ 1,0,0, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //left foot width
		{ ExtClass::CharacterStruct::SKELETON,18, TEXT("a01_J_FootR_03"),{ 1,0,0, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //right foot width
		{ ExtClass::CharacterStruct::SKELETON,19, TEXT("a01_J_UplegL_020"),{ 1,0,0, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //left thigh thickness
		{ ExtClass::CharacterStruct::SKELETON,20, TEXT("a01_J_UplegR_020"),{ 1,0,0, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //right thigh thickness
		{ ExtClass::CharacterStruct::SKELETON,21, TEXT("a01_J_KataL_02"),{ 0,1,0, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //left shoulder height
		{ ExtClass::CharacterStruct::SKELETON,22, TEXT("a01_J_KataR_02"),{ 0,1,0, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //right shoulder height
		{ ExtClass::CharacterStruct::SKELETON,23, TEXT("a01_J_KataL_01"),{ 0,0,0, 0,0,0, 0,1,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //left shoulder height
		{ ExtClass::CharacterStruct::SKELETON,24, TEXT("a01_J_KataR_01"),{ 0,0,0, 0,0,0, 0,1,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //right shoulder height
		{ ExtClass::CharacterStruct::SKELETON,25, TEXT("a01_J_Kosi_020"),{ 1,0,0, 0,0,0, 0,0,0 }, Slider::DIVIDE, AAUCardData::MODIFY_FRAME }, //counterpart to 7
		{ ExtClass::CharacterStruct::SKELETON,26, TEXT("a01_J_Kosi_020"),{ 0,0,1, 0,0,0, 0,0,0 }, Slider::DIVIDE, AAUCardData::MODIFY_FRAME }, //counterpart to 8
		{ ExtClass::CharacterStruct::SKELETON,27, TEXT("a01_J_Neck_01"),{ 1,0,1, 0,0,0, 0,0,0 }, Slider::MULTIPLY, AAUCardData::MODIFY_FRAME }, //neck size
		{ ExtClass::CharacterStruct::SKELETON,28, TEXT("a01_J_Neck_02"),{ 1,0,1, 0,0,0, 0,0,0 }, Slider::DIVIDE, AAUCardData::MODIFY_FRAME }, //neck size counterpart
		{ ExtClass::CharacterStruct::SKELETON,29, TEXT("a_J_dan07"),{ 1,1,1, 0,0,0, 0,0,0 }, Slider::MULTIPLY, AAUCardData::MODIFY_FRAME }, //male only
		{ ExtClass::CharacterStruct::SKELETON,30, TEXT("a_J_dan03"),{ 0,0,0, 0,0,0, 0,0,1 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //apply to females too
		{ ExtClass::CharacterStruct::SKELETON,31, TEXT("a_J_ana03"),{ 0,0,0, 0,0,0, 0,0,1 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //not sure why these exist twice
		{ ExtClass::CharacterStruct::SKELETON,32, TEXT("a_J_dan03"),{ 1,1,0, 0,0,0, 0,0,0 }, Slider::MULTIPLY, AAUCardData::MODIFY_FRAME }, //girth
		{ ExtClass::CharacterStruct::SKELETON,33, TEXT("a_J_ana03"),{ 1,1,0, 0,0,0, 0,0,0 }, Slider::MULTIPLY, AAUCardData::MODIFY_FRAME }, //
		{ ExtClass::CharacterStruct::SKELETON,34, TEXT("a_J_dan06"),{ 1,1,1, 0,0,0, 0,0,0 }, Slider::MULTIPLY, AAUCardData::MODIFY_FRAME }, //apply to females too
		{ ExtClass::CharacterStruct::SKELETON,35, TEXT("a_J_ana06"),{ 1,1,1, 0,0,0, 0,0,0 }, Slider::MULTIPLY, AAUCardData::MODIFY_FRAME }, //not sure why these exist twice
		{ ExtClass::CharacterStruct::SKELETON,36, TEXT("a01_J_KataL_02"),{ 1,0,0, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //left shoulder width
		{ ExtClass::CharacterStruct::SKELETON,37, TEXT("a01_J_KataR_02"),{ 1,0,0, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_FRAME }, //right shoulder width
	},
	{
		//BODY
		{ ExtClass::CharacterStruct::BODY, 0, TEXT("a01_J_Kosi_010"),{ 1,0,0, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_BONE }, //kosi thickness
		{ ExtClass::CharacterStruct::BODY, 1, TEXT("a01_J_Kosi_010"),{ 0,0,1, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_BONE }, //kosi width
		{ ExtClass::CharacterStruct::BODY, 2, TEXT("a01_J_Neck_01"),{ 1,0,1, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_BONE },
		{ ExtClass::CharacterStruct::BODY, 3, TEXT("a01_J_Neck_02"),{ 1,0,1, 0,0,0, 0,0,0 }, Slider::ADD, AAUCardData::MODIFY_BONE },
		{ ExtClass::CharacterStruct::BODY, 4, TEXT("a01_J_ArmL_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.264981031f }, Slider::ADD, AAUCardData::MODIFY_BONE }, //upper arm
		{ ExtClass::CharacterStruct::BODY, 5, TEXT("a01_J_UdeL_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f }, Slider::ADD, AAUCardData::MODIFY_BONE }, //lower arm
		{ ExtClass::CharacterStruct::BODY, 6, TEXT("a01_J_HijiL_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f }, Slider::ADD, AAUCardData::MODIFY_BONE }, //armgelenk
		{ ExtClass::CharacterStruct::BODY, 7, TEXT("a01_J_ArmR_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.264981031f }, Slider::ADD, AAUCardData::MODIFY_BONE },
		{ ExtClass::CharacterStruct::BODY, 8, TEXT("a01_J_UdeR_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f }, Slider::ADD, AAUCardData::MODIFY_BONE },
		{ ExtClass::CharacterStruct::BODY, 9, TEXT("a01_J_HijiR_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f }, Slider::ADD, AAUCardData::MODIFY_BONE },
		{ ExtClass::CharacterStruct::BODY,10, TEXT("a01_J_TekubiL_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f }, Slider::ADD, AAUCardData::MODIFY_BONE },
		{ ExtClass::CharacterStruct::BODY,11, TEXT("a01_J_TekubiR_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f }, Slider::ADD, AAUCardData::MODIFY_BONE },
		{ ExtClass::CharacterStruct::BODY,12, TEXT("a01_J_ArmL_02"),{ 0,1,1, 0,0,0, 0,-15.795f,0.264981031f }, Slider::ADD, AAUCardData::MODIFY_BONE },
		{ ExtClass::CharacterStruct::BODY,13, TEXT("a01_J_ArmR_02"),{ 0,1,1, 0,0,0, 0,-15.795f,0.264981031f }, Slider::ADD, AAUCardData::MODIFY_BONE },
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

//info from thicker x/y mod
//note that their way of scaling seems to be to add additional bones with an _ofst suffix and giving them scales instead
//a01_J_Kosi_010: unterer körper, startet beim bauchnabel, endet bei beinen leicht unter beckenhöhe
//a01_J_Spin_020: oberer körper, startet am brustkorb und endet an hals und arm ansatz
//a01_J_SiriR_010|a01_J_SiriL_010: linker/rechter hintern. note that 20 is also part of it, but not modified in the mod
//a01_J_UplegL_010|a01_J_UplegR_010: linker/rechter oberschenkel. 20 affects the entire leg, interistingly



}
