#include "StdAfx.h"



namespace Shared {

	void Slider::ModifySRT(D3DXVECTOR3* scale,D3DXVECTOR3* rot,D3DXVECTOR3* trans,Slider::Operation op,const AAUCardData::BoneMod& mod) {
		switch (op) {
		case ADD:
			scale->x += mod.scales[0];
			scale->y += mod.scales[1];
			scale->z += mod.scales[2];
			trans->x += mod.transformations[0];
			trans->y += mod.transformations[1];
			trans->z += mod.transformations[2];
			rot->x += mod.rotations[0];
			rot->y += mod.rotations[1];
			rot->z += mod.rotations[2];
			break;
		case MULTIPLY:
			scale->x *= mod.scales[0];
			scale->y *= mod.scales[1];
			scale->z *= mod.scales[2];
			trans->x *= mod.transformations[0];
			trans->y *= mod.transformations[1];
			trans->z *= mod.transformations[2];
			rot->x += mod.rotations[0];
			rot->y += mod.rotations[1];
			rot->z += mod.rotations[2];
			break;
		case DIVIDE:
			if (mod.scales[0] != 0) scale->x /= mod.scales[0];
			if (mod.scales[1] != 0) scale->y /= mod.scales[1];
			if (mod.scales[2] != 0) scale->z /= mod.scales[2];
			if (mod.transformations[0] != 0) trans->x /= mod.transformations[0];
			if (mod.transformations[1] != 0)trans->y /= mod.transformations[1];
			if (mod.transformations[2] != 0)trans->z /= mod.transformations[2];
			rot->x += mod.rotations[0];
			rot->y += mod.rotations[1];
			rot->z += mod.rotations[2];
			break;
		default:
			break;
		}
	}

	void Slider::ModifyKeyframe(ExtClass::Keyframe* frame,Slider::Operation op,const AAUCardData::BoneMod& elem) {
		switch (op) {
		case ADD: {
			frame->m_scaleX += elem.scales[0];
			frame->m_scaleY += elem.scales[1];
			frame->m_scaleZ += elem.scales[2];
			D3DXQUATERNION rotQuat;
			(*Shared::D3DXQuaternionRotationYawPitchRoll)(&rotQuat,elem.rotations[1],elem.rotations[0],elem.rotations[2]);
			D3DXQUATERNION* origQuat = (D3DXQUATERNION*)&frame->m_quatX;
			(*Shared::D3DXQuaternionMultiply)(origQuat,origQuat,&rotQuat);
			frame->m_transX += elem.transformations[0];
			frame->m_transY += elem.transformations[1];
			frame->m_transZ += elem.transformations[2];
			break; }
		case MULTIPLY: {
			frame->m_scaleX *= elem.scales[0];
			frame->m_scaleY *= elem.scales[1];
			frame->m_scaleZ *= elem.scales[2];
			D3DXQUATERNION rotQuat;
			(*Shared::D3DXQuaternionRotationYawPitchRoll)(&rotQuat,elem.rotations[1],elem.rotations[0],elem.rotations[2]);
			D3DXQUATERNION* origQuat = (D3DXQUATERNION*)&frame->m_quatX;
			(*Shared::D3DXQuaternionMultiply)(origQuat,origQuat,&rotQuat);
			frame->m_transX *= elem.transformations[0];
			frame->m_transY *= elem.transformations[1];
			frame->m_transZ *= elem.transformations[2];
			break; }
		case DIVIDE: {
			if(elem.scales[0] != 0) frame->m_scaleX /= elem.scales[0];
			if (elem.scales[1] != 0) frame->m_scaleY /= elem.scales[1];
			if (elem.scales[2] != 0) frame->m_scaleZ /= elem.scales[2];
			D3DXQUATERNION rotQuat;
			(*Shared::D3DXQuaternionRotationYawPitchRoll)(&rotQuat,elem.rotations[1],elem.rotations[0],elem.rotations[2]);
			D3DXQUATERNION* origQuat = (D3DXQUATERNION*)&frame->m_quatX;
			(*Shared::D3DXQuaternionMultiply)(origQuat,origQuat,&rotQuat);
			if (elem.transformations[0] != 0) frame->m_transX /= elem.transformations[0];
			if (elem.transformations[1] != 0) frame->m_transY /= elem.transformations[1];
			if (elem.transformations[2] != 0) frame->m_transZ /= elem.transformations[2];
			break; }
		default:
			break;
		}
	}

#define EXPR(ex) static_cast<float(*)(float)>([](float x)->float { return (ex); })

	const std::vector<Slider> g_sliders[ExtClass::CharacterStruct::N_MODELS] = {
		{
			//FACE
			{ ExtClass::CharacterStruct::FACE, 0,	TEXT("A00_J_kuti"),{ 1,0,0, 0,0,0, 0,0,0 },					Slider::ADD, AAUCardData::MODIFY_FRAME }, //mouth
			{ ExtClass::CharacterStruct::FACE, 1,	TEXT("A00_O_mimi"),{ 0,0,0, 0,0,0, 0,1,0 },					Slider::ADD, AAUCardData::MODIFY_FRAME }, //ear height
			{ ExtClass::CharacterStruct::FACE, 2,	TEXT("A00_J_kuti"),{ 0,0,0, 0,0,0, 0,1,0.9f },				Slider::ADD, AAUCardData::MODIFY_FRAME }, //mouth height
			{ ExtClass::CharacterStruct::FACE, 3,	TEXT("A00_J_mayuLrot"),{ 0,0,0, 0,0,0, 0,1,0 },				Slider::ADD, AAUCardData::MODIFY_FRAME }, //left eyebrow height
			{ ExtClass::CharacterStruct::FACE, 4,	TEXT("A00_J_mayuRrot"),{ 0,0,0, 0,0,0, 0,1,0 },				Slider::ADD, AAUCardData::MODIFY_FRAME }, //right eyebrow height
			{ ExtClass::CharacterStruct::FACE, 5,	TEXT("A00_J_meC"),{ 0,0,0, 0,0,0, 0,0,1 },					Slider::ADD, AAUCardData::MODIFY_FRAME }, //eye depth?
			/* mimi
			x 0.64346
			y 0.38282
			z 0.4461*/
			{ ExtClass::CharacterStruct::FACE, 6,	TEXT("A00_O_mimiR"),{ 1,0,0, 0,0,0, 0.64346f,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //Split Ear Scale X Left
			{ ExtClass::CharacterStruct::FACE, 7,	TEXT("A00_O_mimiL"),{ 1,0,0, 0,0,0, -0.64346f,0,0 },	Slider::ADD, AAUCardData::MODIFY_FRAME }, //Split Ear Scale X Right
			{ ExtClass::CharacterStruct::FACE, 8,	TEXT("A00_O_mimiR"),{ 0,1,0, 0,0,0, 0,-0.64346f,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //Split Ear Scale Y Left
			{ ExtClass::CharacterStruct::FACE, 9,	TEXT("A00_O_mimiL"),{ 0,1,0, 0,0,0, 0,-0.64346f,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //Split Ear Scale Y Right
			{ ExtClass::CharacterStruct::FACE, 10,	TEXT("AS00_N_megane"),	{ 0,0,0, 0,0,0, 0,1,0 },			Slider::ADD, AAUCardData::MODIFY_FRAME }, //glasses Y
			{ ExtClass::CharacterStruct::FACE, 11,	TEXT("AS00_N_megane"),	{ 0,0,0, 0,0,0, 0,0,1 },			Slider::ADD, AAUCardData::MODIFY_FRAME }, //glasses Z
			{ ExtClass::CharacterStruct::FACE, 12,	TEXT("AS00_N_megane"),	{ 0,0,0, 1,0,0, 0,0,0 },			Slider::ADD, AAUCardData::MODIFY_FRAME }, //glasses Z
			{ ExtClass::CharacterStruct::FACE, 13,	TEXT("A00_O_mimiL"),{ 0,0,1, 0,0,0, 0,0,0.23f },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, //Split Ear Scale Z Left
			{ ExtClass::CharacterStruct::FACE, 14,	TEXT("A00_O_mimiR"),{ 0,0,1, 0,0,0, 0,0,0.23f },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, //Split Ear Scale Z Right
			{ ExtClass::CharacterStruct::FACE, 15,	TEXT("A00_O_mimiL"),{ 0,0,0, 0,0,0, 1,0,0 },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, //Split Ear Position X Left
			{ ExtClass::CharacterStruct::FACE, 16,	TEXT("A00_O_mimiR"),{ 0,0,0, 0,0,0, -1,0,0 },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, //Split Ear Position X Right
			{ ExtClass::CharacterStruct::FACE, 17,	TEXT("A00_O_mimiL"),{ 0,0,0, 0,0,0, 0,0,1 },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, //Split Ear Position Z Left
			{ ExtClass::CharacterStruct::FACE, 18,	TEXT("A00_O_mimiR"),{ 0,0,0, 0,0,0, 0,0,1 },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, //Split Ear Position Z Right

			//Tot sliders

			{ ExtClass::CharacterStruct::FACE, 19,	TEXT("A00_J_nose"),{ 0,0,0, 1,0,0, 0,0,0 },		    Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::FACE, 20,	TEXT("A00_J_nose"),{ 1,0,0, 0,0,0, 0,0,0 },		    Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::FACE, 21,	TEXT("A00_J_nose"),{ 0,0,0, 0,0,0, 0,1,0 },		    Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::FACE, 22,	TEXT("A00_J_nose"),{ 0,0,1, 0,0,0, 0,0,0 },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, 
			{ ExtClass::CharacterStruct::FACE, 23,	TEXT("A00_J_nosebridge"),{ 0,0,1, 0,0,0, 0,0,0 },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, 
			{ ExtClass::CharacterStruct::FACE, 24,	TEXT("A00_J_chin"),{ 0,1,0, 0,0,0, 0,0,0 },		    Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::FACE, 25,	TEXT("A00_J_chin"),{ 0,0,1, 0,0,0, 0,0,0 },		    Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::FACE, 26,	TEXT("A00_J_jaw"),{ 1,0,0, 0,0,0, 0,0,0 },		    Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::FACE, 27,	TEXT("A00_J_jaw"),{ 0,1,0, 0,0,0, 0,0,0 },		    Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::FACE, 28,	TEXT("A00_J_jaw"),{ 0,0,0, 0,0,0, 0,1,0 },		    Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::FACE, 29,	TEXT("A00_J_cheek"),{ 0,0,0, 0,0,0, 0,1,0 },		    Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::FACE, 30,	TEXT("A00_J_cheek"),{ 0,0,1, 0,0,0, 0,0,0 },		    Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::FACE, 31,	TEXT("A00_J_cheek"),{ 1,0,0, 0,0,0, 0,0,0 },		    Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::FACE, 32,	TEXT("A00_J_jaw_back"),{ 0,0,0, 0,0,0, 0,0,1 },		    Slider::ADD, AAUCardData::MODIFY_FRAME },

			//Tot Teeth Sliders
			{ ExtClass::CharacterStruct::FACE, 33,	TEXT("A00_J_sharkU0"),{ 0,0,0, 0,0,0, 0,1,0.4f },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, 
			{ ExtClass::CharacterStruct::FACE, 34,	TEXT("A00_J_sharkU1"),{ 0,0,0, 0,0,0, 0,-1,-0.4f },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, 
			{ ExtClass::CharacterStruct::FACE, 35,	TEXT("A00_J_sharkB0"),{ 0,0,0, 0,0,0, 0,1,0.4f },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, 
			{ ExtClass::CharacterStruct::FACE, 36,	TEXT("A00_J_sharkB1"),{ 0,0,0, 0,0,0, 0,-1,-0.4f },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, 
			{ ExtClass::CharacterStruct::FACE, 37,	TEXT("A00_J_incisorUR"),{ 0,0,0, 0,0,0, 0,-1,-0.2f },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, 
			{ ExtClass::CharacterStruct::FACE, 38,	TEXT("A00_J_incisorUL"),{ 0,0,0, 0,0,0, 0,-1,-0.2f },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, 
			{ ExtClass::CharacterStruct::FACE, 39,	TEXT("A00_J_incisorBR"),{ 0,0,0, 0,0,0, 0,1,0.5f },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, 
			{ ExtClass::CharacterStruct::FACE, 40,	TEXT("A00_J_incisorBL"),{ 0,0,0, 0,0,0, 0,1,0.5f },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, 
			{ ExtClass::CharacterStruct::FACE, 41,	TEXT("A00_J_canineUR"),{ 0,0,0, 0,0,0, 0,-1,-0.2f },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, 
			{ ExtClass::CharacterStruct::FACE, 42,	TEXT("A00_J_canineUL"),{ 0,0,0, 0,0,0, 0,-1,-0.2f },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, 
			{ ExtClass::CharacterStruct::FACE, 43,	TEXT("A00_J_canineBR"),{ 0,0,0, 0,0,0, 0,1,0.5f },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, 
			{ ExtClass::CharacterStruct::FACE, 44,	TEXT("A00_J_canineBL"),{ 0,0,0, 0,0,0, 0,1,0.5f },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, 
			{ ExtClass::CharacterStruct::FACE, 45,	TEXT("A00_J_premolarUR"),{ 0,0,0, 0,0,0, 0,-1,-0.2f },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, 
			{ ExtClass::CharacterStruct::FACE, 46,	TEXT("A00_J_premolarUL"),{ 0,0,0, 0,0,0, 0,-1,-0.2f },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, 
			{ ExtClass::CharacterStruct::FACE, 47,	TEXT("A00_J_premolarBR"),{ 0,0,0, 0,0,0, 0,1,0.5f },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, 
			{ ExtClass::CharacterStruct::FACE, 48,	TEXT("A00_J_premolarBL"),{ 0,0,0, 0,0,0, 0,1,0.5f },		    Slider::ADD, AAUCardData::MODIFY_FRAME }, 


			//onemoreslider.
			{ ExtClass::CharacterStruct::FACE, 49,	TEXT("A00_J_chin"),{ 1,1,0.4f, 0,0,0, 0,0.4f,-0.16f },		    Slider::ADD, AAUCardData::MODIFY_FRAME },


		},
		{
			//SKELETON
			{ ExtClass::CharacterStruct::SKELETON, 0, TEXT("a01_J_Hip_010"),	{ 0,1,0, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //body height,
			{ ExtClass::CharacterStruct::SKELETON, 1, TEXT("a01_J_Hip_010"),	{ 1,0,0, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //body width,
			{ ExtClass::CharacterStruct::SKELETON, 2, TEXT("a01_J_Hip_010"),	{ 0,0,1, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //body thickness
			{ ExtClass::CharacterStruct::SKELETON, 3, TEXT("a01_J_ArmL_01"),	{ 1,0,1, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //left arm thickness
			{ ExtClass::CharacterStruct::SKELETON, 4, TEXT("a01_J_ArmR_01"),	{ 1,0,1, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //right arm thickness
			{ ExtClass::CharacterStruct::SKELETON, 5, TEXT("a01_J_HandL_02"),	{ 1,0,0, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //left hand fingers length
			{ ExtClass::CharacterStruct::SKELETON, 6, TEXT("a01_J_HandR_02"),	{ 1,0,0, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //right hand fingers length
			{ ExtClass::CharacterStruct::SKELETON, 7, TEXT("a01_J_Kosi_010"),	{ 1,0,0, 0,0,0, 0,0,0 },		Slider::MULTIPLY, AAUCardData::MODIFY_FRAME }, //kosi thickness
			{ ExtClass::CharacterStruct::SKELETON, 8, TEXT("a01_J_Kosi_010"),	{ 0,0,1, 0,0,0, 0,0,0 },		Slider::MULTIPLY, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::SKELETON, 9, TEXT("a01_J_SiriL_010"),	{ 1,0,0, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::SKELETON,10, TEXT("a01_J_SiriR_010"),	{ 1,0,0, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::SKELETON,11, TEXT("a01_J_SiriL_010"),	{ 0,0,1, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::SKELETON,12, TEXT("a01_J_SiriR_010"),	{ 0,0,1, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::SKELETON,13, TEXT("a01_J_OyaL_01"),	{ 1,0,0, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //left hand thumb length
			{ ExtClass::CharacterStruct::SKELETON,14, TEXT("a01_J_OyaR_01"),	{ 1,0,0, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //right hand thumb length
			{ ExtClass::CharacterStruct::SKELETON,15, TEXT("a01_J_FootL_03"),	{ 0,0,1, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //left foot length
			{ ExtClass::CharacterStruct::SKELETON,16, TEXT("a01_J_FootR_03"),	{ 0,0,1, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //right foot length
			{ ExtClass::CharacterStruct::SKELETON,17, TEXT("a01_J_FootL_03"),	{ 1,0,0, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //left foot width
			{ ExtClass::CharacterStruct::SKELETON,18, TEXT("a01_J_FootR_03"),	{ 1,0,0, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //right foot width
			{ ExtClass::CharacterStruct::SKELETON,19, TEXT("a01_J_UplegL_020"),	{ 1,0,0, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //left thigh width
			{ ExtClass::CharacterStruct::SKELETON,20, TEXT("a01_J_UplegR_020"),	{ 1,0,0, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //right thigh width
			{ ExtClass::CharacterStruct::SKELETON,21, TEXT("a01_J_KataL_02"),	{ 0,1,0, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //left shoulder height
			{ ExtClass::CharacterStruct::SKELETON,22, TEXT("a01_J_KataR_02"),	{ 0,1,0, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //right shoulder height
			{ ExtClass::CharacterStruct::SKELETON,23, TEXT("a01_J_KataL_01"),	{ 0,0,0, 0,0,0, 0,1,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //left shoulder height
			{ ExtClass::CharacterStruct::SKELETON,24, TEXT("a01_J_KataR_01"),	{ 0,0,0, 0,0,0, 0,1,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //right shoulder height
			{ ExtClass::CharacterStruct::SKELETON,25, TEXT("a01_J_Kosi_020"),	{ 1,0,0, 0,0,0, 0,0,0 },		Slider::DIVIDE, AAUCardData::MODIFY_FRAME }, //counterpart to 7
			{ ExtClass::CharacterStruct::SKELETON,26, TEXT("a01_J_Kosi_020"),	{ 0,0,1, 0,0,0, 0,0,0 },		Slider::DIVIDE, AAUCardData::MODIFY_FRAME }, //counterpart to 8
			{ ExtClass::CharacterStruct::SKELETON,27, TEXT("a01_J_Neck_01"),	{ 1,0,1, 0,0,0, 0,0,0 },		Slider::MULTIPLY, AAUCardData::MODIFY_FRAME }, //neck size
			{ ExtClass::CharacterStruct::SKELETON,28, TEXT("a01_J_Neck_02"),	{ 1,0,1, 0,0,0, 0,0,0 },		Slider::DIVIDE, AAUCardData::MODIFY_FRAME }, //neck size counterpart
			{ ExtClass::CharacterStruct::SKELETON,29, TEXT("a_J_dan07"),		{ 1,1,1, 0,0,0, 0,0,0 },		Slider::MULTIPLY, AAUCardData::MODIFY_FRAME }, //male only
			{ ExtClass::CharacterStruct::SKELETON,30, TEXT("a_J_dan03"),		{ 0,0,0, 0,0,0, 0,0,1 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //apply to females too
			{ ExtClass::CharacterStruct::SKELETON,31, TEXT("a_J_ana03"),		{ 0,0,0, 0,0,0, 0,0,1 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //not sure why these exist twice
			{ ExtClass::CharacterStruct::SKELETON,32, TEXT("a_J_dan02"),		{ 1,1,0, 0,0,0, 0,0,0 },		Slider::MULTIPLY, AAUCardData::MODIFY_FRAME }, //girth
			{ ExtClass::CharacterStruct::SKELETON,33, TEXT("a_J_ana02"),		{ 1,1,0, 0,0,0, 0,0,0 },		Slider::MULTIPLY, AAUCardData::MODIFY_FRAME }, //
			{ ExtClass::CharacterStruct::SKELETON,34, TEXT("a_J_dan06"),		{ 1,1,1, 0,0,0, 0,0,0 },		Slider::MULTIPLY, AAUCardData::MODIFY_FRAME }, //apply to females too
			{ ExtClass::CharacterStruct::SKELETON,35, TEXT("a_J_ana06"),		{ 1,1,1, 0,0,0, 0,0,0 },		Slider::MULTIPLY, AAUCardData::MODIFY_FRAME }, //not sure why these exist twice
			{ ExtClass::CharacterStruct::SKELETON,36, TEXT("a01_J_KataL_02"),	{ 1,0,0, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //left shoulder width
			{ ExtClass::CharacterStruct::SKELETON,37, TEXT("a01_J_KataR_02"),	{ 1,0,0, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //right shoulder width
			// Sliders above are flawed and obsolete


			{ ExtClass::CharacterStruct::SKELETON,38, TEXT("a01_J_ArmL_00"),	{ 0,0,0, 0,0,0, 0.15f,0,0 },				Slider::ADD, AAUCardData::MODIFY_FRAME }, //arm distance
			{ ExtClass::CharacterStruct::SKELETON,39, TEXT("a01_J_ArmR_00"),	{ 0,0,0, 0,0,0, -0.15f,0,0 },				Slider::ADD, AAUCardData::MODIFY_FRAME }, //arm distance
			{ ExtClass::CharacterStruct::SKELETON,40, TEXT("a01_J_ArmL_00"),	{ 0.15f,0.15f,0.15f, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //arm scale
			{ ExtClass::CharacterStruct::SKELETON,41, TEXT("a01_J_ArmR_00"),	{ 0.15f,0.15f,0.15f, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME }, //arm scale

			{ ExtClass::CharacterStruct::SKELETON,42, TEXT("a01_N_ribon_00"),	{ 0,0,0, 0,0,0, 0,0,0.1f },					Slider::ADD, AAUCardData::MODIFY_FRAME }, //chest size
			{ ExtClass::CharacterStruct::SKELETON,43, TEXT("a01_J_MuneL_01"),	{ 0.1f,0.1f,0.1f, 0,0,0, 0.1f,-0.15f,0.035f },Slider::ADD, AAUCardData::MODIFY_FRAME }, //chest size
			{ ExtClass::CharacterStruct::SKELETON,44, TEXT("a01_J_MuneR_01"),	{ 0.1f,0.1f,0.1f, 0,0,0,-0.1f,-0.15f,0.035f },Slider::ADD, AAUCardData::MODIFY_FRAME }, //chest size

			{ ExtClass::CharacterStruct::SKELETON,45, TEXT("a01_J_SiriL_010"),	{ 0.15f,0.15f,0.15f, 0,0,0, 0.15f,0,0 },	Slider::ADD, AAUCardData::MODIFY_FRAME }, //hip thickness
			{ ExtClass::CharacterStruct::SKELETON,46, TEXT("a01_J_SiriR_010"),	{ 0.15f,0.15f,0.15f, 0,0,0, -0.15f,0,0 },	Slider::ADD, AAUCardData::MODIFY_FRAME }, //hip thickness
			{ ExtClass::CharacterStruct::SKELETON,47, TEXT("a01_J_Kokan_010"),{ 0.1f,0,0, 0,0,0, 0,0,0 },					Slider::ADD, AAUCardData::MODIFY_FRAME }, //hip thickness

			{ ExtClass::CharacterStruct::SKELETON,48, TEXT("a01_J_UplegL_010"),	{ 0,0,0, 0,0,0, 0.1f,0,0 },					Slider::ADD, AAUCardData::MODIFY_FRAME }, //Leg Distance
			{ ExtClass::CharacterStruct::SKELETON,49, TEXT("a01_J_UplegR_010"),	{ 0,0,0, 0,0,0, -0.1f,0,0 },				Slider::ADD, AAUCardData::MODIFY_FRAME }, //Leg Distance
			{ ExtClass::CharacterStruct::SKELETON,50, TEXT("a01_J_UplegDDL_010"),{ 0,0,0, 0,0,0, 0.1f,0,0 },				Slider::ADD, AAUCardData::MODIFY_FRAME }, //Leg Distance
			{ ExtClass::CharacterStruct::SKELETON,51, TEXT("a01_J_UplegDDR_010"),{ 0,0,0, 0,0,0, -0.1f,0,0 },				Slider::ADD, AAUCardData::MODIFY_FRAME }, //Leg Distance
			{ ExtClass::CharacterStruct::SKELETON,52, TEXT("a01_J_UplegDDL_020"),{ 0,0,0, 0,0,0, 0.1f,0,0 },				Slider::ADD, AAUCardData::MODIFY_FRAME }, //Leg Distance
			{ ExtClass::CharacterStruct::SKELETON,53, TEXT("a01_J_UplegDDR_020"),{ 0,0,0, 0,0,0, -0.1f,0,0 },				Slider::ADD, AAUCardData::MODIFY_FRAME }, //Leg Distance

			{ ExtClass::CharacterStruct::SKELETON,54, TEXT("a01_J_Neck_03"),{ 1,1,1, 0,0,0, 0,-0.4f,0 },				Slider::ADD, AAUCardData::MODIFY_FRAME }, //extra head size

			{ ExtClass::CharacterStruct::SKELETON,55, TEXT("a01_N_Zentai_010"),{ 1,1,1, 0,0,0, 0,-0.4f,0 },				Slider::ADD, AAUCardData::MODIFY_FRAME }, //body size

			{ ExtClass::CharacterStruct::SKELETON,56, TEXT("a01_J_MuneL_02"),{ 0.3f,0.4f,0.2f, 0,0,0, -0.18f,0,0 },	Slider::ADD, AAUCardData::MODIFY_FRAME },		//breasts size
			{ ExtClass::CharacterStruct::SKELETON,57, TEXT("a01_J_MuneR_02"),{ 0.3f,0.4f,0.2f, 0,0,0, 0.18f,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME },	//breasts size

			{ ExtClass::CharacterStruct::SKELETON,58, TEXT("a01_J_UplegL_010"),{ 0,0,0, 0,0,0, 0,0,-0.75f },					Slider::ADD, AAUCardData::MODIFY_FRAME }, //Leg Depth
			{ ExtClass::CharacterStruct::SKELETON,59, TEXT("a01_J_UplegR_010"),{ 0,0,0, 0,0,0, 0,0,-0.75f },				Slider::ADD, AAUCardData::MODIFY_FRAME }, //Leg Depth
			{ ExtClass::CharacterStruct::SKELETON,60, TEXT("a01_J_UplegDDL_010"),{ 0,0,0, 0,0,0, 0,0,-0.75f },				Slider::ADD, AAUCardData::MODIFY_FRAME }, //Leg Depth
			{ ExtClass::CharacterStruct::SKELETON,61, TEXT("a01_J_UplegDDR_010"),{ 0,0,0, 0,0,0, 0,0,-0.75f },				Slider::ADD, AAUCardData::MODIFY_FRAME }, //Leg Depth
			{ ExtClass::CharacterStruct::SKELETON,62, TEXT("a01_J_UplegDDL_020"),{ 0,0,0, 0,0,0, 0,0,-0.75f },				Slider::ADD, AAUCardData::MODIFY_FRAME }, //Leg Depth
			{ ExtClass::CharacterStruct::SKELETON,63, TEXT("a01_J_UplegDDR_020"),{ 0,0,0, 0,0,0, 0,0,-0.75f },				Slider::ADD, AAUCardData::MODIFY_FRAME }, //Leg Depth

			{ ExtClass::CharacterStruct::SKELETON,64, TEXT("a01_J_MuneL_02"),{ 0,0,0, 0,0,0, 0,0.1f,0 },				Slider::ADD, AAUCardData::MODIFY_FRAME }, //Breast Pushup L
			{ ExtClass::CharacterStruct::SKELETON,65, TEXT("a01_J_MuneR_02"),{ 0,0,0, 0,0,0, 0,0.1f,0 },				Slider::ADD, AAUCardData::MODIFY_FRAME }, //Breast Pushup R

			{ ExtClass::CharacterStruct::SKELETON,66, TEXT("a01_J_Neck_01"),{ 0,0,0, 0,0,0, 0,0.2f,0 },				Slider::ADD, AAUCardData::MODIFY_FRAME }, //Neck Height
			{ ExtClass::CharacterStruct::SKELETON,67, TEXT("a01_J_Neck_02"),{ 0,0,0, 0,0,0, 0,-0.2f,0.0255f },				Slider::ADD, AAUCardData::MODIFY_FRAME }, //Neck Height

			{ ExtClass::CharacterStruct::SKELETON,68, TEXT("a01_J_MuneL_01"),{ 0.1f,0.1f,0, 0,0,0, 0.1f,-0.15f,0 },Slider::ADD, AAUCardData::MODIFY_FRAME }, //chest width
			{ ExtClass::CharacterStruct::SKELETON,69, TEXT("a01_J_MuneR_01"),{ 0.1f,0.1f,0, 0,0,0, -0.1f,-0.15f,0 },Slider::ADD, AAUCardData::MODIFY_FRAME }, //chest width

			{ ExtClass::CharacterStruct::SKELETON,70, TEXT("a01_N_ribon_00"),{ 0,0,0, 0,0,0, 0,0,0.1f },					Slider::ADD, AAUCardData::MODIFY_FRAME }, //chest Thickness
			{ ExtClass::CharacterStruct::SKELETON,71, TEXT("a01_J_MuneL_01"),{ 0,0,0.1f, 0,0,0, 0,0,0.035f },Slider::ADD, AAUCardData::MODIFY_FRAME }, //chest Thickness
			{ ExtClass::CharacterStruct::SKELETON,72, TEXT("a01_J_MuneR_01"),{ 0,0,0.1f, 0,0,0, 0,0,0.035f },Slider::ADD, AAUCardData::MODIFY_FRAME }, //chest Thickness

			{ ExtClass::CharacterStruct::SKELETON,73, TEXT("a01_J_SiriL_010"),{ 0,0.15f,0.15f, 0,0,0, 0.15f,0,0 },	Slider::ADD, AAUCardData::MODIFY_FRAME }, //Butt Size
			{ ExtClass::CharacterStruct::SKELETON,74, TEXT("a01_J_SiriR_010"),{ 0,0.15f,0.15f, 0,0,0, -0.15f,0,0 },	Slider::ADD, AAUCardData::MODIFY_FRAME }, //Butt Size

			{ ExtClass::CharacterStruct::SKELETON,75, TEXT("a01_J_MuneR_01"),{ 0.1f,0.1f,0.1f, 0,0,0, 0.1f,-0.1f,0 },	    Slider::ADD, AAUCardData::MODIFY_FRAME }, //BalloonTits
			{ ExtClass::CharacterStruct::SKELETON,76, TEXT("a01_J_MuneL_01"),{ 0.1f,0.1f,0.1f, 0,0,0, -0.1f,-0.1f,0 },    Slider::ADD, AAUCardData::MODIFY_FRAME }, //BalloonTits
			{ ExtClass::CharacterStruct::SKELETON,77, TEXT("a01_J_MuneR_02"),{ 0.1f,0.1f,0.1f, 0,0,0, 0,0,0 },	        Slider::ADD, AAUCardData::MODIFY_FRAME }, //BalloonTits
			{ ExtClass::CharacterStruct::SKELETON,78, TEXT("a01_J_MuneL_02"),{ 0.1f,0.1f,0.1f, 0,0,0, 0,0,0 },	        Slider::ADD, AAUCardData::MODIFY_FRAME }, //BalloonTits
			{ ExtClass::CharacterStruct::SKELETON,79, TEXT("a01_J_ChikuR_01"),{ 0,0,0.25f, 0,0,0, 0,0,0 },	            Slider::ADD, AAUCardData::MODIFY_FRAME }, //Breast Perkiness
			{ ExtClass::CharacterStruct::SKELETON,80, TEXT("a01_J_ChikuL_01"),{ 0,0,0.25f, 0,0,0, 0,0,0 },	            Slider::ADD, AAUCardData::MODIFY_FRAME }, //Breast Perkiness
			{ ExtClass::CharacterStruct::SKELETON,81, TEXT("a01_J_MuneR_01"),{ 0,0,0, 0,0,0, -0.1f,0,0 },	            Slider::ADD, AAUCardData::MODIFY_FRAME }, //Breast Distance
			{ ExtClass::CharacterStruct::SKELETON,82, TEXT("a01_J_MuneL_01"),{ 0,0,0, 0,0,0, 0.1f,0,0 },	            Slider::ADD, AAUCardData::MODIFY_FRAME }, //Breast Distance



		},
		{
			//BODY
			{ ExtClass::CharacterStruct::BODY, 0, TEXT("a01_J_Kosi_010"),	{ 1,0,0, 0,0,0, 0,0,0 },					Slider::ADD, AAUCardData::MODIFY_BONE }, //kosi thickness
			{ ExtClass::CharacterStruct::BODY, 1, TEXT("a01_J_Kosi_010"),	{ 0,0,1, 0,0,0, 0,0,0 },					Slider::ADD, AAUCardData::MODIFY_BONE }, //kosi width
			{ ExtClass::CharacterStruct::BODY, 2, TEXT("a01_J_Neck_01"),	{ 1,0,1, 0,0,0, 0,0,0 },					Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::BODY, 3, TEXT("a01_J_Neck_02"),	{ 1,0,1, 0,0,0, 0,0,0 },					Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::BODY, 4, TEXT("a01_J_ArmL_01"),	{ 0,1,1, 0,0,0, 0,-15.795f,0.264981031f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //upper arm
			{ ExtClass::CharacterStruct::BODY, 5, TEXT("a01_J_UdeL_01"),	{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //lower arm
			{ ExtClass::CharacterStruct::BODY, 6, TEXT("a01_J_HijiL_01"),	{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //armgelenk
			{ ExtClass::CharacterStruct::BODY, 7, TEXT("a01_J_ArmR_01"),	{ 0,1,1, 0,0,0, 0,-15.795f,0.264981031f },	Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::BODY, 8, TEXT("a01_J_UdeR_01"),	{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::BODY, 9, TEXT("a01_J_HijiR_01"),	{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::BODY,10, TEXT("a01_J_TekubiL_01"),	{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::BODY,11, TEXT("a01_J_TekubiR_01"),	{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::BODY,12, TEXT("a01_J_ArmL_02"),	{ 0,1,1, 0,0,0, 0,-15.795f,0.264981031f },	Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::BODY,13, TEXT("a01_J_ArmR_02"),	{ 0,1,1, 0,0,0, 0,-15.795f,0.264981031f },	Slider::ADD, AAUCardData::MODIFY_BONE },

			{ ExtClass::CharacterStruct::BODY,14, TEXT("a01_J_Spin_020"),{ 0.15f,0,0.1f, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //chest size
			{ ExtClass::CharacterStruct::BODY,15, TEXT("a01_J_Kosi_010"),{ 0.15f,0,0.06f, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //hip size

			{ ExtClass::CharacterStruct::BODY,16, TEXT("a01_J_UplegL_020"),{ 0.15f,0,0.15f, 0,0,0, -0.2f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness
			{ ExtClass::CharacterStruct::BODY,17, TEXT("a01_J_UplegL_010"),{ 0.15f,0,0.15f, 0,0,0, -0.1f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness
			{ ExtClass::CharacterStruct::BODY,18, TEXT("a01_J_UplegR_020"),{ 0.15f,0,0.15f, 0,0,0, 0.2f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness
			{ ExtClass::CharacterStruct::BODY,19, TEXT("a01_J_UplegR_010"),{ 0.15f,0,0.15f, 0,0,0, 0.1f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness

			{ ExtClass::CharacterStruct::BODY,20, TEXT("a01_J_Spin_020"),{ 0,0.2f,0, 0,0,0, 0,-3.2f,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //chest height
			{ ExtClass::CharacterStruct::BODY,21, TEXT("a01_J_Kosi_010"),{ 0,0.2f,0, 0,0,0, 0,-2.0f,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //hip height

			{ ExtClass::CharacterStruct::BODY,22, TEXT("a01_J_Spin_010"),{ 0,0.1f,0, 0,0,0, 0,-1.2f,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //waist height
			{ ExtClass::CharacterStruct::BODY,23, TEXT("a01_J_Spin_010"),{ 0,0,0, 0,0,0, 0,1.0f,0 },		Slider::ADD, AAUCardData::MODIFY_BONE }, //waist position

			{ ExtClass::CharacterStruct::BODY,24, TEXT("a01_J_UplegR_020"),{ 0,0.15f,0, 0,0,0, 0,-1.5f,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //upperthigh height
			{ ExtClass::CharacterStruct::BODY,25, TEXT("a01_J_UplegL_020"),{ 0,0.15f,0, 0,0,0, 0,-1.5f,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //upperthigh height

			{ ExtClass::CharacterStruct::BODY,26, TEXT("a01_J_Spin_010"),{ 0,0,0.15f, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //waist height
			{ ExtClass::CharacterStruct::BODY,27, TEXT("a01_J_Kosi_010"),{ 0,0,0.10f, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //waist height

			{ ExtClass::CharacterStruct::BODY,28, TEXT("a01_J_Kosi_010"),{ 1,0,0, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::BODY,29, TEXT("a01_J_Kosi_010"),{ 0,0,1, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE },

			{ ExtClass::CharacterStruct::BODY,30, TEXT("a01_J_MuneL_01"),{ 0.25f,0,0, 0,0,0, -0.22f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::BODY,31, TEXT("a01_J_MuneR_01"),{ 0.25f,0,0, 0,0,0, 0.22f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::BODY,32, TEXT("a01_J_MuneL_02"),{ 0.1f,0,0, 0,0,0, -0.1f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::BODY,33, TEXT("a01_J_MuneR_02"),{ 0.1f,0,0, 0,0,0, 0.1f,0,0 },		Slider::ADD, AAUCardData::MODIFY_BONE },

			{ ExtClass::CharacterStruct::BODY,34, TEXT("a01_J_MuneL_01"),{ 0.2f,0.29f,0, 0,0,0, -0.4f,-4.4f,0 },	Slider::ADD, AAUCardData::MODIFY_BONE },	//breasts size
			{ ExtClass::CharacterStruct::BODY,35, TEXT("a01_J_MuneR_01"),{ 0.2f,0.29f,0, 0,0,0, 0.4f,-4.4f,0 },	Slider::ADD, AAUCardData::MODIFY_BONE },		//breasts size

			{ ExtClass::CharacterStruct::BODY, 36, TEXT("a01_J_Kosi_010"),{ 0,0,0, 0,0,0, 0,0,-0.75f },				Slider::ADD, AAUCardData::MODIFY_BONE }, //hip depth correction

			{ ExtClass::CharacterStruct::BODY, 37, TEXT("a01_J_ChikuL_02"),{ 1,0.7f,0, 0,0,0, -0.965f,-10.265f,0.1f },				Slider::ADD, AAUCardData::MODIFY_BONE }, //Nipple Size
			{ ExtClass::CharacterStruct::BODY, 38, TEXT("a01_J_ChikuR_02"),{ 1,0.7f,0, 0,0,0, 0.965f,-10.265f,0.1f },				Slider::ADD, AAUCardData::MODIFY_BONE }, //Nipple Size

			{ ExtClass::CharacterStruct::BODY,39, TEXT("a01_J_Spin_020"),{ 0.15f,0,0, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //chest Width

			{ ExtClass::CharacterStruct::BODY,40, TEXT("a01_J_Spin_020"),{ 0,0,0.1f, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //chest Thickness

			{ ExtClass::CharacterStruct::BODY,41, TEXT("a01_J_ArmL_02"),{ 0,1,1, 0,0,0, 0,-15.795f,0.264981031f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //upper arm
			{ ExtClass::CharacterStruct::BODY,42, TEXT("a01_J_ArmL_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.264981031f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //upper arm
			{ ExtClass::CharacterStruct::BODY,43, TEXT("a01_J_ArmR_02"),{ 0,1,1, 0,0,0, 0,-15.795f,0.264981031f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //upper arm
			{ ExtClass::CharacterStruct::BODY,44, TEXT("a01_J_ArmR_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.264981031f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //upper arm
			{ ExtClass::CharacterStruct::BODY,45, TEXT("a01_J_HijiL_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //upper arm
			{ ExtClass::CharacterStruct::BODY,46, TEXT("a01_J_HijiR_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //upper arm

			{ ExtClass::CharacterStruct::BODY,47, TEXT("a01_J_UdeL_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //lower arm
			{ ExtClass::CharacterStruct::BODY,48, TEXT("a01_J_TekubiL_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //lower arm
			{ ExtClass::CharacterStruct::BODY,49, TEXT("a01_J_UdeR_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //lower arm
			{ ExtClass::CharacterStruct::BODY,50, TEXT("a01_J_TekubiR_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //lower arm

			{ ExtClass::CharacterStruct::BODY,51, TEXT("a01_J_Kosi_010"),{ 0,0,0.06f, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //Butt Size

			{ ExtClass::CharacterStruct::BODY,52, TEXT("a01_J_UplegL_020"),{ 0.15f,0,0, 0,0,0, -0.2f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //Upper Thigh Width
			{ ExtClass::CharacterStruct::BODY,53, TEXT("a01_J_UplegR_020"),{ 0.15f,0,0, 0,0,0, 0.2f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //Upper Thigh Width

			{ ExtClass::CharacterStruct::BODY,54, TEXT("a01_J_UplegL_020"),{ 0,0,0.15f, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //Upper Thigh Thickness
			{ ExtClass::CharacterStruct::BODY,55, TEXT("a01_J_UplegR_020"),{ 0,0,0.15f, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //Upper Thigh Thickness

			{ ExtClass::CharacterStruct::BODY,56, TEXT("a01_J_UplegL_010"),{ 0.15f,0,0, 0,0,0, -0.1f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness
			{ ExtClass::CharacterStruct::BODY,57, TEXT("a01_J_UplegR_010"),{ 0.15f,0,0, 0,0,0, 0.1f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness

			{ ExtClass::CharacterStruct::BODY,58, TEXT("a01_J_UplegL_010"),{ 0,0,0.15f, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness
			{ ExtClass::CharacterStruct::BODY,59, TEXT("a01_J_UplegR_010"),{ 0,0,0.15f, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness


		},
		{
			//LEGS
			{ ExtClass::CharacterStruct::LEGS,	0, TEXT("a01_J_UplegR_010"),{ 0.15f,0,0.15f, 0,0,0, 0.1f,0,0 },		Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness
			{ ExtClass::CharacterStruct::LEGS,	1, TEXT("a01_J_UplegL_010"),{ 0.15f,0,0.15f, 0,0,0, -0.1f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness
			{ ExtClass::CharacterStruct::LEGS,	2, TEXT("a01_J_UplegR_020"),{ 0.15f,0,0.15f, 0,0,0, 0.2f,0,0 },		Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness
			{ ExtClass::CharacterStruct::LEGS,	3, TEXT("a01_J_UplegL_020"),{ 0.15f,0,0.15f, 0,0,0, -0.2f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness

			{ ExtClass::CharacterStruct::LEGS,	4, TEXT("a01_J_LegL_01"),{ 0.15f,0,0.15f, 0,0,0, -0.1f,0,0 },		Slider::ADD, AAUCardData::MODIFY_BONE }, //calves thickness
			{ ExtClass::CharacterStruct::LEGS,	5, TEXT("a01_J_LegR_01"),{ 0.15f,0,0.15f, 0,0,0, 0.1f,0,0 },		Slider::ADD, AAUCardData::MODIFY_BONE }, //calves thickness

			{ ExtClass::CharacterStruct::LEGS,	6, TEXT("a01_J_UplegR_020"),{ 0,0.15f,0, 0,0,0, 0,-1.5f,0 },		Slider::ADD, AAUCardData::MODIFY_BONE }, //upperthigh height
			{ ExtClass::CharacterStruct::LEGS,	7, TEXT("a01_J_UplegL_020"),{ 0,0.15f,0, 0,0,0, 0,-1.5f,0 },		Slider::ADD, AAUCardData::MODIFY_BONE }, //upperthigh height

			{ ExtClass::CharacterStruct::LEGS,	8, TEXT("a01_J_Kosi_010"),{ 1,0,0, 0,0,0, 0,0,0 },					Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::LEGS,	9, TEXT("a01_J_Kosi_010"),{ 0,0,1, 0,0,0, 0,0,0 },					Slider::ADD, AAUCardData::MODIFY_BONE },

			{ ExtClass::CharacterStruct::LEGS,	10, TEXT("a01_J_Kosi_010"),{ 0.15f,0,0.06f, 0,0,0, 0,0,0 },			Slider::ADD, AAUCardData::MODIFY_BONE }, //hip size

			{ ExtClass::CharacterStruct::LEGS,	11, TEXT("a01_J_Spin_010"),{ 0,0,0.15f, 0,0,0, 0,0,0 },				Slider::ADD, AAUCardData::MODIFY_BONE }, //waist thickness
			{ ExtClass::CharacterStruct::LEGS,	12, TEXT("a01_J_Kosi_010"),{ 0,0,0.10f, 0,0,0, 0,0,0 },				Slider::ADD, AAUCardData::MODIFY_BONE }, //waist thickness

			{ ExtClass::CharacterStruct::LEGS,	13, TEXT("a01_J_Spin_010"),{ 0,0.1f,0, 0,0,0, 0,-1.2f,0 },			Slider::ADD, AAUCardData::MODIFY_BONE }, //waist height
			{ ExtClass::CharacterStruct::LEGS,	14, TEXT("a01_J_Spin_010"),{ 0,0,0, 0,0,0, 0,1.0f,0 },				Slider::ADD, AAUCardData::MODIFY_BONE }, //waist position

			{ ExtClass::CharacterStruct::LEGS,	15, TEXT("a01_J_Kosi_010"),{ 0,0,0, 0,0,0, 0,0,-0.75f },			Slider::ADD, AAUCardData::MODIFY_BONE }, //hip depth correction
			{ ExtClass::CharacterStruct::LEGS,	16, TEXT("a01_J_Kosi_010"),{ 0,0.2f,0, 0,0,0, 0,-2.0f,0 },			Slider::ADD, AAUCardData::MODIFY_BONE }, //hip height

			{ ExtClass::CharacterStruct::LEGS,	17, TEXT("a01_J_Kosi_010"),{ 0,0,0.06f, 0,0,0, 0,0,0 },			Slider::ADD, AAUCardData::MODIFY_BONE }, //Butt size

			{ ExtClass::CharacterStruct::LEGS, 18, TEXT("a01_J_UplegR_020"),{ 0.15f,0,0, 0,0,0, 0.2f,0,0 },		Slider::ADD, AAUCardData::MODIFY_BONE }, //Upper Thigh Width
			{ ExtClass::CharacterStruct::LEGS, 19, TEXT("a01_J_UplegL_020"),{ 0.15f,0,0, 0,0,0, -0.2f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //Upper Thigh Width

			{ ExtClass::CharacterStruct::LEGS, 20, TEXT("a01_J_UplegR_020"),{ 0,0,0.15f, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_BONE }, //Upper Thigh Thickness
			{ ExtClass::CharacterStruct::LEGS, 21, TEXT("a01_J_UplegL_020"),{ 0,0,0.15f, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //Upper Thigh Thickness

			{ ExtClass::CharacterStruct::LEGS, 22, TEXT("a01_J_UplegR_010"),{ 0.15f,0,0, 0,0,0, 0.1f,0,0 },		Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness
			{ ExtClass::CharacterStruct::LEGS, 23, TEXT("a01_J_UplegL_010"),{ 0.15f,0,0, 0,0,0, -0.1f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness

			{ ExtClass::CharacterStruct::LEGS, 24, TEXT("a01_J_UplegR_010"),{ 0,0,0.15f, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness
			{ ExtClass::CharacterStruct::LEGS, 25, TEXT("a01_J_UplegL_010"),{ 0,0,0.15f, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness

			{ ExtClass::CharacterStruct::LEGS,	26, TEXT("a01_J_LegL_01"),{ 0.15f,0,0, 0,0,0, -0.1f,0,0 },		Slider::ADD, AAUCardData::MODIFY_BONE }, //calves thickness
			{ ExtClass::CharacterStruct::LEGS,	27, TEXT("a01_J_LegR_01"),{ 0.15f,0,0, 0,0,0, 0.1f,0,0 },		Slider::ADD, AAUCardData::MODIFY_BONE }, //calves thickness

			{ ExtClass::CharacterStruct::LEGS,	28, TEXT("a01_J_LegL_01"),{ 0,0,0.15f, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_BONE }, //calves thickness
			{ ExtClass::CharacterStruct::LEGS,	29, TEXT("a01_J_LegR_01"),{ 0,0,0.15f, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_BONE }, //calves thickness


		},
		{
			//HAIR_FRONT
			{ ExtClass::CharacterStruct::HAIR_FRONT,0, TEXT("A00_N_kamiF"),{ 1,1,1, 0,0,0, 0,0,0 },			Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_FRONT,1, TEXT("A00_N_kamiFtop"),{ 1,1,1, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_FRONT,2, TEXT("A00_N_kamiF"),{ 0,0,0, 0,0,0, 0,0.09f,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_FRONT,3, TEXT("A00_N_kamiFtop"),{ 0,0,0, 0,0,0, 0,0.09f,0 },	Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_FRONT,4, TEXT("A00_N_kamiF"),{ 1,0,0, 0,0,0, 0,0,0 },			Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_FRONT,5, TEXT("A00_N_kamiFtop"),{ 1,0,0, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_FRONT,6, TEXT("A00_N_kamiFtop00"),{ 1,1,1, 0,0,0, 0,0,0 },			Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_FRONT,7, TEXT("A00_N_kamiFtop00"),{ 0,0,0, 0,0,0, 0,0.09f,0 },	Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_FRONT,8, TEXT("A00_N_kamiFtop00"),{ 1,0,0, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME },
		},
		{
			//HAIR_SIDE
			{ ExtClass::CharacterStruct::HAIR_SIDE,0, TEXT("A00_N_kamiS"),{ 1,1,1, 0,0,0, 0,0,0 },			Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_SIDE,1, TEXT("A00_N_kamiStop"),{ 1,1,1, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_SIDE,2, TEXT("A00_N_kamiS"),{ 0,0,0, 0,0,0, 0,0.09f,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_SIDE,3, TEXT("A00_N_kamiStop"),{ 0,0,0, 0,0,0, 0,0.09f,0 },	Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_SIDE,4, TEXT("A00_N_kamiS"),{ 1,0,0, 0,0,0, 0,0,0 },			Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_SIDE,5, TEXT("A00_N_kamiStop"),{ 1,0,0, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME },
		},
		{
			//HAIR_BACK
			{ ExtClass::CharacterStruct::HAIR_BACK,0, TEXT("A00_N_kamiB"),{ 1,1,1, 0,0,0, 0,0,0 },			Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_BACK,1, TEXT("A00_N_kamiBtop"),{ 1,1,1, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_BACK,2, TEXT("A00_N_kamiBtop00"),{ 1,1,1, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_BACK,3, TEXT("A00_N_kamiB"),{ 0,0,0, 0,0,0, 0,0.09f,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_BACK,4, TEXT("A00_N_kamiBtop"),{ 0,0,0, 0,0,0, 0,0.09f,0 },	Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_BACK,5, TEXT("A00_N_kamiBtop00"),{ 0,0,0, 0,0,0, 0,0.09f,0 },	Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_BACK,6, TEXT("A00_N_kamiB"),{ 1,0,0, 0,0,0, 0,0,0 },			Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_BACK,7, TEXT("A00_N_kamiBtop"),{ 1,0,0, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_BACK,8, TEXT("A00_N_kamiBtop00"),{ 1,0,0, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME },
		},
		{
			//HAIR_EXT
			{ ExtClass::CharacterStruct::HAIR_EXT,0, TEXT("A00_N_kamiO"),{ 1,1,1, 0,0,0, 0,0,0 },			Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_EXT,1, TEXT("A00_N_kamiOtop"),{ 1,1,1, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_EXT,2, TEXT("A00_N_kamiO"),{ 0,0,0, 0,0,0, 0,0.09f,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_EXT,3, TEXT("A00_N_kamiOtop"),{ 0,0,0, 0,0,0, 0,0.09f,0 },	Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_EXT,4, TEXT("A00_N_kamiO"),{ 1,0,0, 0,0,0, 0,0,0 },			Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::HAIR_EXT,5, TEXT("A00_N_kamiOtop"),{ 1,0,0, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME },
		},
		{
			//FACE_SLIDERS
		},
		{
			//SKIRT
			{ ExtClass::CharacterStruct::SKIRT,0, TEXT("a01_J_Kosi_010"),{ 0.15f,0,0.06f, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //hip size
			{ ExtClass::CharacterStruct::SKIRT,1, TEXT("a01_J_Spin_020"),{ 0.15f,0,0.1f, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //chest size

			{ ExtClass::CharacterStruct::SKIRT,2, TEXT("a01_J_UplegL_020"),{ 0.15f,0,0.15f, 0,0,0, -0.2f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness
			{ ExtClass::CharacterStruct::SKIRT,3, TEXT("a01_J_UplegL_010"),{ 0.15f,0,0.15f, 0,0,0, -0.1f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness
			{ ExtClass::CharacterStruct::SKIRT,4, TEXT("a01_J_UplegR_020"),{ 0.15f,0,0.15f, 0,0,0, 0.2f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness
			{ ExtClass::CharacterStruct::SKIRT,5, TEXT("a01_J_UplegR_010"),{ 0.15f,0,0.15f, 0,0,0, 0.1f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness

			{ ExtClass::CharacterStruct::SKIRT,6, TEXT("a01_J_Spin_020"),{ 0,0.2f,0, 0,0,0, 0,-3.2f,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //chest height
			{ ExtClass::CharacterStruct::SKIRT,7, TEXT("a01_J_Kosi_010"),{ 0,0.2f,0, 0,0,0, 0,-2.0f,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //hip height

			{ ExtClass::CharacterStruct::SKIRT,8, TEXT("a01_J_Spin_010"),{ 0,0.1f,0, 0,0,0, 0,-1.2f,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //waist height
			{ ExtClass::CharacterStruct::SKIRT,9, TEXT("a01_J_Spin_010"),{ 0,0,0, 0,0,0, 0,1.0f,0 },		Slider::ADD, AAUCardData::MODIFY_BONE }, //waist position

			{ ExtClass::CharacterStruct::SKIRT,10, TEXT("a01_J_UplegR_020"),{ 0,0.15f,0, 0,0,0, 0,-1.5f,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //upperthigh height
			{ ExtClass::CharacterStruct::SKIRT,11, TEXT("a01_J_UplegL_020"),{ 0,0.15f,0, 0,0,0, 0,-1.5f,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //upperthigh height

			{ ExtClass::CharacterStruct::SKIRT,12, TEXT("a01_J_Spin_010"),{ 0,0,0.15f, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //waist thickness
			{ ExtClass::CharacterStruct::SKIRT,13, TEXT("a01_J_Kosi_010"),{ 0,0,0.10f, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //waist thickness

			{ ExtClass::CharacterStruct::SKIRT,14, TEXT("a01_J_ArmL_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.264981031f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //upper arm
			{ ExtClass::CharacterStruct::SKIRT,15, TEXT("a01_J_UdeL_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //lower arm
			{ ExtClass::CharacterStruct::SKIRT,16, TEXT("a01_J_HijiL_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //armgelenk
			{ ExtClass::CharacterStruct::SKIRT,17, TEXT("a01_J_ArmR_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.264981031f },	Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::SKIRT,18, TEXT("a01_J_UdeR_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::SKIRT,19, TEXT("a01_J_HijiR_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::SKIRT,20, TEXT("a01_J_TekubiL_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::SKIRT,21, TEXT("a01_J_TekubiR_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE },

			{ ExtClass::CharacterStruct::SKIRT,22, TEXT("a01_J_LegL_01"),{ 0.15f,0,0.15f, 0,0,0, -0.1f,0,0 },		Slider::ADD, AAUCardData::MODIFY_BONE }, //calves thickness
			{ ExtClass::CharacterStruct::SKIRT,23, TEXT("a01_J_LegR_01"),{ 0.15f,0,0.15f, 0,0,0, 0.1f,0,0 },		Slider::ADD, AAUCardData::MODIFY_BONE }, //calves thickness

			{ ExtClass::CharacterStruct::SKIRT,24, TEXT("a01_J_Neck_01"),{ 1,0,1, 0,0,0, 0,0,0 },					Slider::ADD, AAUCardData::MODIFY_BONE }, //Neck thickness
			{ ExtClass::CharacterStruct::SKIRT,25, TEXT("a01_J_Neck_02"),{ 1,0,1, 0,0,0, 0,0,0 },					Slider::ADD, AAUCardData::MODIFY_BONE }, //Neck thickness

			{ ExtClass::CharacterStruct::SKIRT,26, TEXT("a01_J_Kosi_010"),{ 1,0,0, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::SKIRT,27, TEXT("a01_J_Kosi_010"),{ 0,0,1, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE },

			{ ExtClass::CharacterStruct::SKIRT,28, TEXT("a01_J_MuneL_01"),{ 0.25f,0,0, 0,0,0, -0.22f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::SKIRT,29, TEXT("a01_J_MuneR_01"),{ 0.25f,0,0, 0,0,0, 0.22f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::SKIRT,30, TEXT("a01_J_MuneL_02"),{ 0.1f,0,0, 0,0,0, -0.1f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::SKIRT,31, TEXT("a01_J_MuneR_02"),{ 0.1f,0,0, 0,0,0, 0.1f,0,0 },		Slider::ADD, AAUCardData::MODIFY_BONE },

			{ ExtClass::CharacterStruct::SKIRT,32, TEXT("a01_J_ArmL_02"),{ 0,1,1, 0,0,0, 0,-15.795f,0.264981031f },	Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::SKIRT,33, TEXT("a01_J_ArmR_02"),{ 0,1,1, 0,0,0, 0,-15.795f,0.264981031f },	Slider::ADD, AAUCardData::MODIFY_BONE },

			{ ExtClass::CharacterStruct::SKIRT,34, TEXT("a01_J_MuneL_01"),{ 0.2f,0.29f,0, 0,0,0, -0.4f,-4.4f,0 },	Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::SKIRT,35, TEXT("a01_J_MuneR_01"),{ 0.2f,0.29f,0, 0,0,0, 0.4f,-4.4f,0 },	Slider::ADD, AAUCardData::MODIFY_BONE },
			{ ExtClass::CharacterStruct::SKIRT,36, TEXT("a01_J_MuneL_02"),{ 0.3f,0.4f,0.2f, 0,0,0, -0.18f,0,0 },	Slider::ADD, AAUCardData::MODIFY_FRAME },
			{ ExtClass::CharacterStruct::SKIRT,37, TEXT("a01_J_MuneR_02"),{ 0.3f,0.4f,0.2f, 0,0,0, 0.18f,0,0 },		Slider::ADD, AAUCardData::MODIFY_FRAME },

			{ ExtClass::CharacterStruct::SKIRT, 38, TEXT("a01_J_Kosi_010"),{ 0,0,0, 0,0,0, 0,0,-0.75f },			Slider::ADD, AAUCardData::MODIFY_BONE }, //hip depth correction

			{ ExtClass::CharacterStruct::SKIRT,39, TEXT("a01_J_Spin_020"),{ 0.15f,0,0, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //chest width

			{ ExtClass::CharacterStruct::SKIRT,40, TEXT("a01_J_Spin_020"),{ 0,0,0.1f, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //chest Thickness

			{ ExtClass::CharacterStruct::SKIRT,41, TEXT("a01_J_ArmL_02"),{ 0,1,1, 0,0,0, 0,-15.795f,0.264981031f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //upper arm
			{ ExtClass::CharacterStruct::SKIRT,42, TEXT("a01_J_ArmL_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.264981031f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //upper arm
			{ ExtClass::CharacterStruct::SKIRT,43, TEXT("a01_J_ArmR_02"),{ 0,1,1, 0,0,0, 0,-15.795f,0.264981031f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //upper arm
			{ ExtClass::CharacterStruct::SKIRT,44, TEXT("a01_J_ArmR_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.264981031f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //upper arm
			{ ExtClass::CharacterStruct::SKIRT,45, TEXT("a01_J_HijiL_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //upper arm
			{ ExtClass::CharacterStruct::SKIRT,46, TEXT("a01_J_HijiR_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //upper arm

			{ ExtClass::CharacterStruct::SKIRT,47, TEXT("a01_J_UdeL_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //lower arm
			{ ExtClass::CharacterStruct::SKIRT,48, TEXT("a01_J_TekubiL_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //lower arm
			{ ExtClass::CharacterStruct::SKIRT,49, TEXT("a01_J_UdeR_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //lower arm
			{ ExtClass::CharacterStruct::SKIRT,50, TEXT("a01_J_TekubiR_01"),{ 0,1,1, 0,0,0, 0,-15.795f,0.346239984f },	Slider::ADD, AAUCardData::MODIFY_BONE }, //lower arm

			{ ExtClass::CharacterStruct::SKIRT,51, TEXT("a01_J_Kosi_010"),{ 0,0,0.06f, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //Butt size

			{ ExtClass::CharacterStruct::SKIRT,52, TEXT("a01_J_UplegL_020"),{ 0.15f,0,0, 0,0,0, -0.2f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //Upper Thigh Width
			{ ExtClass::CharacterStruct::SKIRT,53, TEXT("a01_J_UplegR_020"),{ 0.15f,0,0, 0,0,0, 0.2f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //Upper Thigh Width

			{ ExtClass::CharacterStruct::SKIRT,54, TEXT("a01_J_UplegL_020"),{ 0,0,0.15f, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //Upper Thigh Thickness
			{ ExtClass::CharacterStruct::SKIRT,55, TEXT("a01_J_UplegR_020"),{ 0,0,0.15f, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //Upper Thigh Thickness

			{ ExtClass::CharacterStruct::SKIRT,56, TEXT("a01_J_UplegL_010"),{ 0.15f,0,0, 0,0,0, -0.1f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness
			{ ExtClass::CharacterStruct::SKIRT,57, TEXT("a01_J_UplegR_010"),{ 0.15f,0,0, 0,0,0, 0.1f,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness

			{ ExtClass::CharacterStruct::SKIRT,58, TEXT("a01_J_UplegL_010"),{ 0,0,0.15f, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness
			{ ExtClass::CharacterStruct::SKIRT,59, TEXT("a01_J_UplegR_010"),{ 0,0,0.15f, 0,0,0, 0,0,0 },	Slider::ADD, AAUCardData::MODIFY_BONE }, //thighs thickness

			{ ExtClass::CharacterStruct::SKIRT,60, TEXT("a01_J_LegL_01"),{ 0.15f,0,0, 0,0,0, -0.1f,0,0 },		Slider::ADD, AAUCardData::MODIFY_BONE }, //calves Width
			{ ExtClass::CharacterStruct::SKIRT,61, TEXT("a01_J_LegR_01"),{ 0.15f,0,0, 0,0,0, 0.1f,0,0 },		Slider::ADD, AAUCardData::MODIFY_BONE }, //calves Width

			{ ExtClass::CharacterStruct::SKIRT,62, TEXT("a01_J_LegL_01"),{ 0,0,0.15f, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_BONE }, //calves thickness
			{ ExtClass::CharacterStruct::SKIRT,63, TEXT("a01_J_LegR_01"),{ 0,0,0.15f, 0,0,0, 0,0,0 },		Slider::ADD, AAUCardData::MODIFY_BONE }, //calves thickness

			{ ExtClass::CharacterStruct::SKIRT, 64, TEXT("a01_J_ChikuL_02"),{ 1,0.7f,0, 0,0,0, -0.965f,-10.265f,0.1f },				Slider::ADD, AAUCardData::MODIFY_BONE }, //Nipple Size
			{ ExtClass::CharacterStruct::SKIRT, 65, TEXT("a01_J_ChikuR_02"),{ 1,0.7f,0, 0,0,0, 0.965f,-10.265f,0.1f },				Slider::ADD, AAUCardData::MODIFY_BONE }, //Nipple Size
		}
	};

#undef EXPR

}
