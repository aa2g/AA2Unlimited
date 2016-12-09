#pragma once

#include <string>
#include <functional>

#include "Functions\Shared\Globals.h"
#include "External\ExternalClasses\CharacterStruct.h"
#include "Functions\AAUCardData.h"

namespace Shared {


/*
 * Defines a slider functionality
 */
struct Slider {
	enum Operation {
		ADD,MULTIPLY,DIVIDE
	};
	struct Functor {
		
		std::function<float(float)> func;

		//mostly for lambdas
		template<typename T>
		Functor(T& t) {
			func = t;
		}

		Functor(const std::function<float(float)>& p) {
			func = p;
		}

		Functor(float(*ptr)(float)) {
			func = ptr;
		}

		Functor(float f) {
			func = [f](float val) {return val * f; };
		}

		Functor(int f) {
			func = [f](float val) {return val * f; };
		}
	};

	ExtClass::CharacterStruct::Models target; //xx file it affects
	int index; //index of this slider in the array
	std::wstring boneName; //bone if affects
	Functor mod[9]; //used as a mask by multiplying
	Operation op;
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
		AAUCardData::BoneMod retmod;
		for (int i = 0; i < 9; i++) retmod.data[i] = mod[i].func(value);
		switch(op) {
		case MULTIPLY:
		case DIVIDE:
			for (int i = 0; i < 3; i++) {
				if (retmod.scales[i] == 0) retmod.scales[i] = 1;
			}
			for (int i = 0; i < 3; i++) {
				if (retmod.transformations[i] == 0) retmod.transformations[i] = 1;
			}
			break;
		case ADD:
		default:
			break;
		}
		return retmod;
	}

	static void ModifySRT(D3DVECTOR3* scale,D3DVECTOR3* rot,D3DVECTOR3* trans, Slider::Operation op, const AAUCardData::BoneMod& mod);
	static void ModifyKeyframe(ExtClass::Keyframe* frame,Slider::Operation op,const AAUCardData::BoneMod& mod);
};

extern const std::vector<Slider> g_sliders[ExtClass::CharacterStruct::N_MODELS];


//info from thicker x/y mod
//note that their way of scaling seems to be to add additional bones with an _ofst suffix and giving them scales instead
//a01_J_Kosi_010: unterer körper, startet beim bauchnabel, endet bei beinen leicht unter beckenhöhe
//a01_J_Spin_020: oberer körper, startet am brustkorb und endet an hals und arm ansatz
//a01_J_SiriR_010|a01_J_SiriL_010: linker/rechter hintern. note that 20 is also part of it, but not modified in the mod
//a01_J_UplegL_010|a01_J_UplegR_010: linker/rechter oberschenkel. 20 affects the entire leg, interistingly

}
