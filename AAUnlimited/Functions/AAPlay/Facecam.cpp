#define _USE_MATH_DEFINES 
#include <cmath>

#include "Facecam.h"

#include "Files\Config.h"
#include "External\ExternalClasses\HClasses\HInfo.h"
#include "External\ExternalClasses\Frame.h"
#include "General\ModuleInfo.h"
#include "Files\Logger.h"
#include "Functions\Shared\Globals.h"

/*
 * Here is the glorious plan (please dont laugh):
 * - when you press q, the camera shifts to the neck of the passive character.
 * - the neck has the same rotational properties of the head
 * -> we need to set the camera using those properties
 * unfortunately i suck really hard and have no idea about 3d or maths. i only have 
 * the matrix of the bone, and taking out the euler angles is not enough. i cant reverse
 * engineer what i have to do or what he does to the translations either.
 *
 * thankfully, the camera has this unused matrix. so this is what we do: copy the bone matrix
 * to that matrix, and keep all camera parameters at 0.
 */
namespace Facecam {

void RestoreCamera();
void Cleanup();


ExtClass::HInfo* loc_hinfo = NULL;
ExtClass::Frame* loc_focusBone = NULL;
D3DVECTOR3 loc_focusOffset{ 0,0,0 }; //additional translation offset
int loc_state = 0; //0 - default, 1 - active POV, 2 - passive POV

ExtClass::CharacterStruct* loc_passiveChar = NULL;
ExtClass::CharacterStruct* loc_activeChar = NULL;
ExtClass::XXFile* loc_passiveFaceXX = NULL; //face xx of the characters. if changed, model was reloaded
ExtClass::XXFile* loc_activeFaceXX = NULL;
D3DVECTOR3 loc_passiveEyeOffset; //offsets from neck to eyes
D3DVECTOR3 loc_activeEyeOffset;
bool loc_passiveFaceHidden = false;
bool loc_activeFaceHidden = false;

D3DMATRIX loc_activeKaos[6]; //will hold the original kao matrizes. probably always id-matrix, but just to make sure
D3DMATRIX loc_passiveKaos[6];
//shows or hides face, hair and tongue
void ShowFace(bool active, bool visible) {
	ExtClass::CharacterStruct* character = active ? loc_activeChar
												  : loc_passiveChar;
	if (character) {
		ExtClass::XXFile* toHide[6] = { character->m_xxFace, character->m_xxTounge,
									  character->m_xxHairs[0],character->m_xxHairs[1],
									  character->m_xxHairs[2],character->m_xxHairs[3] }; //alternative, we could scale the neck
		D3DMATRIX(&kaoBackup)[6] = active ? loc_activeKaos : loc_passiveKaos;
		for (int i = 0; i < 6; i++) {
			//all_root->scene_root->kao; we scale kao to 0 or id
			//do save search for kao
			ExtClass::XXFile* it = toHide[i];
			if (it == NULL) continue; //some of them might not be there (e.g has no side hair)
			ExtClass::Frame* boneIt = it->m_root; //all_root
			if (boneIt == NULL || boneIt->m_nChildren != 1) continue;
			boneIt = &boneIt->m_children[0]; //sene_root (weird notation cause boneIt = boneIt->m_boneArray sends the wrong message)
			if (boneIt == NULL || boneIt->m_nChildren != 1) continue;
			boneIt = &boneIt->m_children[0]; //kao
			//backup the matrix if possible
			static const D3DMATRIX nullMatrix = { 0.001f,0,0,0,  0,0.001f,0,0,  0,0,0.001f,0,  0,0,0,1.0f };
			if (memcmp(&boneIt->m_matrix1, &nullMatrix, sizeof(nullMatrix)) != 0) {
				//its not a nullmatrix, back up the current one into the array
				kaoBackup[i] = boneIt->m_matrix1;
			}
			//set to 0 matrix or backup depending on visible parameter
			if (visible) {
				boneIt->m_matrix1 = kaoBackup[i];
			}
			else {
				boneIt->m_matrix1 = nullMatrix;
			}
		}
	}
}

D3DVECTOR3 FindEyeOffset(ExtClass::CharacterStruct* character) {
	D3DVECTOR3 retVal{ 0,0,0 };
	//find left and right eye
	ExtClass::Frame* rightEye,*leftEye;
	rightEye = character->m_xxFace->FindBone("A00_J_meR2",-1);
	leftEye = character->m_xxFace->FindBone("A00_J_meL2",-1);

	if (rightEye != NULL && leftEye != NULL) {
		ExtClass::Frame* it;
		D3DVECTOR3 rightEyePos {0,0,0};
		it = rightEye;
		while(it != NULL) {
			rightEyePos.x += it->m_matrix5._41;
			rightEyePos.y += it->m_matrix5._42;
			rightEyePos.z += it->m_matrix5._43;
			it = it->m_parent;
		}
		D3DVECTOR3 leftEyePos {0,0,0};
		it = leftEye;
		while (it != NULL) {
			rightEyePos.x += it->m_matrix5._41;
			rightEyePos.y += it->m_matrix5._42;
			rightEyePos.z += it->m_matrix5._43;
			it = it->m_parent;
		}
		//point in the middle between the two eyes
		D3DVECTOR3 eyeMid;
		eyeMid.x = rightEyePos.x - leftEyePos.x; eyeMid.x = leftEyePos.x + eyeMid.x/2;
		eyeMid.y = rightEyePos.y - leftEyePos.y; eyeMid.y = leftEyePos.y + eyeMid.y/2;
		eyeMid.z = rightEyePos.z - leftEyePos.z; eyeMid.z = leftEyePos.z + eyeMid.z/2;
		return eyeMid;
	}
	
	return retVal;
}

void PostTick(ExtClass::HInfo* hInfo, bool notEnd) {
	if (g_Config.GetKeyValue(Config::USE_H_FACECAM).bVal == false) return;
	if (!notEnd) {
		RestoreCamera();
		Cleanup();
	}
	else {
		//save active/passive characters at start
		if(loc_activeChar == NULL) {
			loc_activeChar = hInfo->m_activeParticipant->m_charPtr;
			loc_passiveChar = hInfo->m_passiveParticipant->m_charPtr;
			loc_hinfo = hInfo;
		}

		//if 3d model was reloaded
		if (loc_state == 0 && 
			(loc_activeChar->m_xxFace != loc_activeFaceXX || loc_passiveChar->m_xxFace != loc_passiveFaceXX))
		{
			loc_activeFaceXX = loc_activeChar->m_xxFace;
			loc_passiveFaceXX = loc_passiveChar->m_xxFace;
			loc_activeEyeOffset = FindEyeOffset(loc_activeChar);
			loc_passiveEyeOffset = FindEyeOffset(loc_passiveChar);
			RestoreCamera();
		}
		if(loc_activeChar->m_xxFace != loc_activeFaceXX &&
			loc_state == 1) {
			loc_activeFaceXX = loc_activeChar->m_xxFace;
			RestoreCamera();
			loc_activeEyeOffset = FindEyeOffset(loc_activeChar);
		}
		if (loc_passiveChar->m_xxFace != loc_passiveFaceXX &&
			loc_state == 2) {
			loc_passiveFaceXX = loc_passiveChar->m_xxFace;
			RestoreCamera();
			loc_passiveEyeOffset = FindEyeOffset(loc_passiveChar);
		}


		if (loc_focusBone != NULL) {
			//we need to keep all camera parameters at 0 (except for fov of course)
			ExtClass::HCamera* cam = hInfo->GetCamera();

			//only roll should be fixed
			//the rest of rotations can be controlled with the mouse to look around
			cam->m_zRotRad = 0;

			cam->m_distToMid = 0;

			cam->m_xShift = 0;
			cam->m_yShift = 0;
			cam->m_zShift = 0;

			//we align the camera with the bone by copying the matrix
			auto mat = loc_focusBone->m_matrix2;
			//adjust with the offsets BEFORE the matrix (not doing an actual multiplication here cause its just a translation)
			float x = loc_focusOffset.x ,y = loc_focusOffset.y,z = loc_focusOffset.z;
			mat._41 += x*mat._11 + y*mat._21 + z*mat._31;
			mat._42 += x*mat._12 + y*mat._22 + z*mat._32;
			mat._43 += x*mat._13 + y*mat._23 + z*mat._33;
			cam->m_matrix = mat;

			//*(BYTE*)(General::GameBase + 0x3A6C80) = 3; //whether the q button is pressed
		}
	}
}

void RestoreCamera()
{
	if (loc_hinfo) {
	//restore the matrix to an identity matrix
		static const D3DMATRIX idMatr = {
			1.0f,0,0,0,
			0,1.0f,0,0,
			0,0,1.0f,0,
			0,0,0,1.0f
		};
		loc_hinfo->GetCamera()->m_matrix = idMatr;

		//unbind camera from the bone
		loc_focusBone = NULL; //all 3d stuff is invalid now, reloaded
		loc_focusOffset = { 0,0,0 };

		//return both faces to normal
		loc_activeFaceHidden = false;
		ShowFace(false, !loc_activeFaceHidden);
		loc_passiveFaceHidden = false;
		ShowFace(true, !loc_passiveFaceHidden);
	}
}

void AdjustCamera(ExtClass::Frame* bone) {

	ExtClass::CharacterData::Hair baldHaircut = ExtClass::CharacterData::Hair();
		baldHaircut.frontHair = 0;
		baldHaircut.sideHair = 0;
		baldHaircut.backhair = 0;
		baldHaircut.backhairFlip = 1;
		baldHaircut.hairExtension = 0;
		
	if (loc_hinfo == NULL) return;
	//make sure the q or w button was pressed, which means bone is the first bone of participant 2 (or 1 depending on position ._.)
	//	Q - toggles states
	//	W - returns camera to normal and focuses on the chest
	if (bone == loc_hinfo->m_passiveParticipant->m_charPtr->m_bonePtrArray[0]	//if Q was pressed
			|| bone == loc_hinfo->m_activeParticipant->m_charPtr->m_bonePtrArray[0]) { 
		loc_state++;
		if (loc_state == 3) {
			//if we come back to free camera mode, restore the matrix to an identity matrix
			loc_state = 0;
			RestoreCamera();
		}

		//set bone depending on state
		ExtClass::CharacterStruct* toFocus;
		switch(loc_state) {
		case 1:
			toFocus = loc_activeChar;
			loc_focusOffset = loc_activeEyeOffset;
			//hide active
			loc_activeFaceHidden = !loc_activeFaceHidden;
			ShowFace(true, !loc_activeFaceHidden);
			break;
		case 2:
			toFocus = loc_passiveChar;
			loc_focusOffset = loc_passiveEyeOffset;
			//hide passive
			loc_passiveFaceHidden = !loc_passiveFaceHidden;
			ShowFace(false, !loc_passiveFaceHidden);
			//return active
			loc_activeFaceHidden = !loc_activeFaceHidden;
			ShowFace(true, !loc_activeFaceHidden);
			break;
		case 0:
		default:
			toFocus = NULL;
		}
		if(toFocus != NULL) {
			loc_focusBone = toFocus->m_bonePtrArray[0];
			loc_focusOffset.x += g_Config.GetKeyValue(Config::POV_OFFSET_X).fVal;
			loc_focusOffset.y += g_Config.GetKeyValue(Config::POV_OFFSET_Y).fVal;
			loc_focusOffset.z += g_Config.GetKeyValue(Config::POV_OFFSET_Z).fVal;


			loc_hinfo->GetCamera()->m_yRotRad = M_PI; //put this to pi cause its reversed for some reason
		} else {
			RestoreCamera();
		}
		return;
	}
	else if(bone == loc_hinfo->m_passiveParticipant->m_charPtr->m_bonePtrArray[1]	//if W was pressed
			|| bone == loc_hinfo->m_activeParticipant->m_charPtr->m_bonePtrArray[1]) {
		//return to default state
		loc_state = 0;
		RestoreCamera();
		
		return;
	}
	else {
		LOGPRIO(Logger::Priority::INFO) << "Some other key was pressed\n";
		return;
	}
}

void Cleanup() {
	loc_state = 0;
	loc_activeFaceXX = NULL;
	loc_passiveFaceXX = NULL;

	loc_hinfo = NULL;
	loc_activeChar = NULL;
	loc_passiveChar = NULL;
}
}