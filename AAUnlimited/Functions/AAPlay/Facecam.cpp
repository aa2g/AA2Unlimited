#define _USE_MATH_DEFINES 
#include <cmath>

#include "Facecam.h"

#include "Files\Config.h"
#include "External\ExternalClasses\HClasses\HInfo.h"
#include "External\ExternalClasses\Bone.h"
#include "General\ModuleInfo.h"
#include "Files\Logger.h"

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


ExtClass::HInfo* loc_hinfo = NULL;
ExtClass::Bone* loc_focusBone = NULL;
int loc_state = 0;

BYTE facelessSlotMale = 90;	//if default fails us set it to something
BYTE facelessSlotFemale = 7;

BYTE m_activeFaceSlot = BYTE(255);
BYTE m_passiveFaceSlot = BYTE(255);

ExtClass::CharacterData::Hair* m_activeHair = NULL;
ExtClass::CharacterData::Hair* m_passiveHair = NULL;

void PostTick(ExtClass::HInfo* hInfo, bool notEnd) {
	if (g_Config.GetKeyValue(Config::USE_H_FACECAM).bVal == false) return;
	if (!notEnd) {
		//prevent losing your heads
		//loc_hinfo->m_activeParticipant->m_charPtr->m_charData->m_faceSlot = m_activeFaceSlot;							//restore active's face slot
		//loc_hinfo->m_activeParticipant->m_charPtr->m_charData->m_hair = ExtClass::CharacterData::Hair(*m_activeHair);	//restore active's haircut
		//loc_hinfo->m_passiveParticipant->m_charPtr->m_charData->m_faceSlot = m_passiveFaceSlot;							//restore passive's face slot
		//loc_hinfo->m_passiveParticipant->m_charPtr->m_charData->m_hair = ExtClass::CharacterData::Hair(*m_passiveHair);	//restore passive's haircut

		loc_state = 0;
		loc_focusBone = NULL;
		loc_hinfo = NULL;
		if (m_passiveHair != NULL) {
			delete m_passiveHair;
			m_passiveHair == NULL;
		}
		if (m_activeHair != NULL) {
			delete m_activeHair;
			m_activeHair == NULL;
		}
		m_activeFaceSlot = BYTE(255);
		m_passiveFaceSlot = BYTE(255);
	}
	else {		
		loc_hinfo = hInfo;
		//remember faces the first time
		if (m_passiveFaceSlot == BYTE(255)) m_passiveFaceSlot = loc_hinfo->m_passiveParticipant->m_charPtr->m_charData->m_faceSlot;
		if (m_activeFaceSlot == BYTE(255)) m_activeFaceSlot = loc_hinfo->m_activeParticipant->m_charPtr->m_charData->m_faceSlot;
		//remember hairstyles the first time
		if (m_passiveHair == NULL) m_passiveHair = new ExtClass::CharacterData::Hair(loc_hinfo->m_passiveParticipant->m_charPtr->m_charData->m_hair);
		if (m_activeHair == NULL) m_activeHair = new ExtClass::CharacterData::Hair(loc_hinfo->m_activeParticipant->m_charPtr->m_charData->m_hair);
		//remember the empty face slots
		facelessSlotMale = (BYTE)g_Config.GetKeyValue(Config::FACELESS_SLOT_MALE).iVal;
		facelessSlotFemale = (BYTE)g_Config.GetKeyValue(Config::FACELESS_SLOT_FEMALE).iVal;

		if (loc_state != 0) {

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
			mat._42 += 1;	//slight adjustment so that the partner looks vaguely in to the camera and not above
			//mat._43 += 1;	//TODO: possibly adjust position so that the pivot is right between the eyes instead of inside the head
			cam->m_matrix = mat;

			//*(BYTE*)(General::GameBase + 0x3A6C80) = 3; //whether the q button is pressed
		}
	}
	
	
}


void AdjustCamera(ExtClass::Bone* bone) {
	const BYTE facelessSlotActive = (loc_hinfo->m_activeParticipant->m_charPtr->m_charData->m_gender) ? facelessSlotFemale : facelessSlotMale;
	const BYTE facelessSlotPassive = (loc_hinfo->m_passiveParticipant->m_charPtr->m_charData->m_gender) ? facelessSlotFemale : facelessSlotMale;

	ExtClass::CharacterData::Hair baldHaircut = ExtClass::CharacterData::Hair();
		baldHaircut.frontHair = 0;
		baldHaircut.sideHair = 0;
		baldHaircut.backhair = 0;
		baldHaircut.hairExtension = 0;
		
	if (loc_hinfo == NULL) return;
	//make sure the q button was pressed, which means bone is the first bone of participant 2 (or 1 depending on position ._.)
	//	Q - toggles states
	//	W - toggles head visibility depending on the state:
	//		state 1 - passive's POV
	//		state 2 - active's POV
	//		state 3 - default camera
	if (bone != loc_hinfo->m_passiveParticipant->m_charPtr->m_bonePtrArray[0]
		&& bone != loc_hinfo->m_activeParticipant->m_charPtr->m_bonePtrArray[0]) {	//if not Q was pressed

		if (bone == loc_hinfo->m_passiveParticipant->m_charPtr->m_bonePtrArray[1]
			|| bone == loc_hinfo->m_activeParticipant->m_charPtr->m_bonePtrArray[1]) { //if W was pressed
			
			if (loc_state == 1) { //toggle passive
				if (loc_hinfo->m_passiveParticipant->m_charPtr->m_charData->m_faceSlot != facelessSlotPassive) {			//if head is visible
					loc_hinfo->m_passiveParticipant->m_charPtr->m_charData->m_faceSlot = facelessSlotPassive;				//set passive's face slot to faceless(male only for now)
					loc_hinfo->m_passiveParticipant->m_charPtr->m_charData->m_hair = baldHaircut;						//set passive's haircut to bald
				} else {
					loc_hinfo->m_passiveParticipant->m_charPtr->m_charData->m_faceSlot = m_passiveFaceSlot;	//restore passive's face slot
					loc_hinfo->m_passiveParticipant->m_charPtr->m_charData->m_hair = ExtClass::CharacterData::Hair(*m_passiveHair);			//restore passive's haircut
				}
				//possibly reload the model
			} else if (loc_state == 2) { //toggle active
				if (loc_hinfo->m_activeParticipant->m_charPtr->m_charData->m_faceSlot != facelessSlotActive) {			//if head is visible
					loc_hinfo->m_activeParticipant->m_charPtr->m_charData->m_faceSlot = facelessSlotActive;				//set active's face slot to faceless(male only for now)
					loc_hinfo->m_activeParticipant->m_charPtr->m_charData->m_hair = baldHaircut;						//set active's haircut to bald
				} else {
					loc_hinfo->m_activeParticipant->m_charPtr->m_charData->m_faceSlot = m_activeFaceSlot;				//restore active's face slot
					loc_hinfo->m_activeParticipant->m_charPtr->m_charData->m_hair = ExtClass::CharacterData::Hair(*m_activeHair);			//restore active's haircut
				}
				//possibly reload the model
			} else { //restore both
				loc_hinfo->m_activeParticipant->m_charPtr->m_charData->m_faceSlot = m_activeFaceSlot;				//restore active's face slot
				loc_hinfo->m_activeParticipant->m_charPtr->m_charData->m_hair = ExtClass::CharacterData::Hair(*m_activeHair);			//restore active's haircut

				loc_hinfo->m_passiveParticipant->m_charPtr->m_charData->m_faceSlot = m_passiveFaceSlot;				//restore passive's face slot
				loc_hinfo->m_passiveParticipant->m_charPtr->m_charData->m_hair = ExtClass::CharacterData::Hair(*m_passiveHair);			//restore passive's haircut

				//possibly reload the models
			}
			return;
		}
		else return;
	} else { //if Q was pressed
		//switch state
		loc_state++;
	}

	if (loc_state == 3) {
		//if we come back to free camera mode, restore the matrix to an identity matrix
		loc_state = 0;
		static const D3DMATRIX idMatr = {
											1.0f,0,0,0,
											0,1.0f,0,0,
											0,0,1.0f,0,
											0,0,0,1.0f
										};
		loc_hinfo->GetCamera()->m_matrix = idMatr;

	}

	//set bone depending on state
	if(loc_state == 1) {
		loc_focusBone = loc_hinfo->m_passiveParticipant->m_charPtr->m_bonePtrArray[0];
	}
	else {
		loc_focusBone = loc_hinfo->m_activeParticipant->m_charPtr->m_bonePtrArray[0];
	}
	

	
}



}