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
int loc_state = 0; //0 - default, 1 - passive POV, 2 - active POV

ExtClass::CharacterData* m_passiveActor = NULL;										//Passive Actor's reference obtained at the start of the scene
BYTE m_passiveFaceSlot = BYTE(255);													//Passive Actor's original face slot, obtained at the start of the scene
ExtClass::CharacterData::Hair* m_passiveHair = NULL;								//Passive Actor's hairstyle reference, obtained at the start of the scene         
BYTE facelessSlotPassive = BYTE(255);												//Passive Actor's desired faceless face, depending on sex               

ExtClass::CharacterData* m_activeActor = NULL;										//Active Actor's reference obtained at the start of the scene
BYTE m_activeFaceSlot = BYTE(255);													//Active Actor's original face slot, obtained at the start of the scene
ExtClass::CharacterData::Hair* m_activeHair = NULL;									//Active Actor's hairstyle reference, obtained at the start of the scene
BYTE facelessSlotActive = BYTE(255);												//Active Actor's desired faceless face, depending on sex

void PostTick(ExtClass::HInfo* hInfo, bool notEnd) {
	if (g_Config.GetKeyValue(Config::USE_H_FACECAM).bVal == false) return;
	if (!notEnd) {
		//prevent losing your heads
		//m_activeActor->m_faceSlot = m_activeFaceSlot;								//restore active's face slot
		//m_activeActor->m_hair = ExtClass::CharacterData::Hair(*m_activeHair);		//restore active's haircut
		//m_passiveActor->m_faceSlot = m_passiveFaceSlot;							//restore passive's face slot
		//m_passiveActor->m_hair = ExtClass::CharacterData::Hair(*m_passiveHair);	//restore passive's haircut

		LOGPRIO(Logger::Priority::INFO) << "Cleaning up...\n";

		loc_state = 0;
		loc_focusBone = NULL;
		loc_hinfo = NULL;

		if (m_passiveHair) {
			delete m_passiveHair;
			m_passiveHair = NULL;
		}
		if (m_activeHair) {
			delete m_activeHair;
			m_activeHair = NULL;
		}

		m_passiveFaceSlot = BYTE(255);
		m_activeFaceSlot = BYTE(255);
		facelessSlotPassive = BYTE(255);
		facelessSlotActive = BYTE(255);

		LOGPRIO(Logger::Priority::INFO) << "Cleaned up!\n";
	}
	else {
		loc_hinfo = hInfo;
		//remember actors the first time
		if (m_passiveActor == NULL)	m_passiveActor = loc_hinfo->m_passiveParticipant->m_charPtr->m_charData;
		if (m_activeActor == NULL)	m_activeActor = loc_hinfo->m_activeParticipant->m_charPtr->m_charData;
		//remember faces the first time
		if (m_passiveFaceSlot == BYTE(255))	m_passiveFaceSlot = m_passiveActor->m_faceSlot;
		if (m_activeFaceSlot == BYTE(255))	m_activeFaceSlot = m_activeActor->m_faceSlot;
		//remember hairstyles the first time
		if (m_passiveHair == NULL)	m_passiveHair = new ExtClass::CharacterData::Hair(m_passiveActor->m_hair);
		if (m_activeHair == NULL)	m_activeHair = new ExtClass::CharacterData::Hair(m_activeActor->m_hair);
		//remember the empty face slots the first time
		if (facelessSlotPassive == BYTE(255))	facelessSlotPassive = (m_passiveActor->m_gender) ? (BYTE)g_Config.GetKeyValue(Config::FACELESS_SLOT_FEMALE).iVal : (BYTE)g_Config.GetKeyValue(Config::FACELESS_SLOT_MALE).iVal;
		if (facelessSlotActive == BYTE(255))	facelessSlotActive = (m_activeActor->m_gender) ? (BYTE)g_Config.GetKeyValue(Config::FACELESS_SLOT_FEMALE).iVal : (BYTE)g_Config.GetKeyValue(Config::FACELESS_SLOT_MALE).iVal;

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
			mat._41 += g_Config.GetKeyValue(Config::POV_OFFSET_X).fVal;
			mat._42 += g_Config.GetKeyValue(Config::POV_OFFSET_Y).fVal;	//slight adjustment so that the partner looks vaguely in to the camera and not above
			mat._43 += g_Config.GetKeyValue(Config::POV_OFFSET_Z).fVal;	//TODO: possibly adjust position so that the pivot is right between the eyes instead of inside the head
			cam->m_matrix = mat;

			//*(BYTE*)(General::GameBase + 0x3A6C80) = 3; //whether the q button is pressed
		}
	}
}


void AdjustCamera(ExtClass::Bone* bone) {

	ExtClass::CharacterData::Hair baldHaircut = ExtClass::CharacterData::Hair();
		baldHaircut.frontHair = 0;
		baldHaircut.sideHair = 0;
		baldHaircut.backhair = 0;
		baldHaircut.backhairFlip = 1;
		baldHaircut.hairExtension = 0;
		
	if (loc_hinfo == NULL) return;
	//make sure the q button was pressed, which means bone is the first bone of participant 2 (or 1 depending on position ._.)
	//	Q - toggles states
	//	W - toggles head visibility depending on the state:
	//		state 1 - passive's POV
	//		state 2 - active's POV
	//		state 3 - default camera
	if (bone != loc_hinfo->m_passiveParticipant->m_charPtr->m_bonePtrArray[0]
		&& bone != loc_hinfo->m_activeParticipant->m_charPtr->m_bonePtrArray[0]) {		//if not Q was pressed

		if (bone == loc_hinfo->m_passiveParticipant->m_charPtr->m_bonePtrArray[1]
			|| bone == loc_hinfo->m_activeParticipant->m_charPtr->m_bonePtrArray[1]) {	//if W was pressed
			
			//Sorry for this piece of cancer but it helps with tracking the actors and stuff.
			//
			//LOGPRIO(Logger::Priority::INFO) << "W was pressed\n"

			//	<< "Active partner:\n\t["
			//	<< m_activeActor->m_surname << " " << m_activeActor->m_forename << "]" << (m_activeActor->m_gender ? "F" : "M")
			//	<< "\n\tFace Slot: " << m_activeActor->m_faceSlot
			//	<< " Hairstyle: (" << m_activeActor->m_hair.frontHair << ", " << m_activeActor->m_hair.sideHair << ", " << m_activeActor->m_hair.backhair << ", " << m_activeActor->m_hair.hairExtension << ")\n"

			//	<< "Passive partner:\n\t["
			//	<< m_passiveActor->m_surname << " " << m_passiveActor->m_forename << "]" << (m_passiveActor->m_gender ? "F" : "M")
			//	<< "\n\tFace Slot: " << m_passiveActor->m_faceSlot
			//	<< " Hairstyle: (" << m_passiveActor->m_hair.frontHair << ", " << m_passiveActor->m_hair.sideHair << ", " << m_passiveActor->m_hair.backhair << ", " << m_passiveActor->m_hair.hairExtension << ")\n"

			//	<< "Saved data:"
			//	<< "\n\tFace slots: "
			//	<< "Active=" << m_activeFaceSlot
			//	<< ", Passive=" << m_passiveFaceSlot
			//	<< "\n\tHairstyles: "
			//	<< "Active=(" << m_activeHair->frontHair << ", " << m_activeHair->sideHair << ", " << m_activeHair->backhair << ", " << m_activeHair->hairExtension << ")"
			//	<< ", Passive=(" << m_passiveHair->frontHair << ", " << m_passiveHair->sideHair << ", " << m_passiveHair->backhair << ", " << m_passiveHair->hairExtension << ")\n\n";

			if (loc_state == 1) { //toggle passive
				if (m_passiveActor->m_faceSlot != facelessSlotPassive) {							//if head is visible
					m_passiveActor->m_faceSlot = facelessSlotPassive;								//set passive's face slot to faceless(male only for now)
					m_passiveActor->m_hair = baldHaircut;											//set passive's haircut to bald
				} else {
					m_passiveActor->m_faceSlot = m_passiveFaceSlot;									//restore passive's face slot
					m_passiveActor->m_hair = *m_passiveHair;										//restore passive's haircut
				}
				//possibly reload the model
			} else if (loc_state == 2) { //toggle active
				if (m_activeActor->m_faceSlot != facelessSlotActive) {								//if head is visible
					m_activeActor->m_faceSlot = facelessSlotActive;									//set active's face slot to faceless(male only for now)
					m_activeActor->m_hair = baldHaircut;											//set active's haircut to bald
				} else {
					m_activeActor->m_faceSlot = m_activeFaceSlot;									//restore active's face slot
					m_activeActor->m_hair = *m_activeHair;											//restore active's haircut
				}
				//possibly reload the model
			} else { //restore both
				m_activeActor->m_faceSlot = m_activeFaceSlot;										//restore active's face slot
				m_activeActor->m_hair = *m_activeHair;												//restore active's haircut

				m_passiveActor->m_faceSlot = m_passiveFaceSlot;										//restore passive's face slot
				m_passiveActor->m_hair = *m_passiveHair;											//restore passive's haircut

				//possibly reload the models
			}
			return;
		}
		else {
			LOGPRIO(Logger::Priority::INFO) << "Some other key was pressed\n";
			return;
		}
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