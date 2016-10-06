#define _USE_MATH_DEFINES 
#include <cmath>

#include "Facecam.h"

#include "Files\Config.h"
#include "External\ExternalClasses\HClasses\HInfo.h"
#include "External\ExternalClasses\Bone.h"
#include "General\ModuleInfo.h"

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

void PostTick(ExtClass::HInfo* hInfo, bool notEnd) {
	if (g_Config.GetKeyValue(Config::USE_H_FACECAM).bVal == false) return;
	if (!notEnd) {
		loc_state = 0;
		loc_focusBone = NULL;
		loc_hinfo = NULL;
	}
	else {
		loc_hinfo = hInfo;
		if (loc_state != 0) {

			//we need to keep all camera parameters at 0 (except for fov of course)
			ExtClass::HCamera* cam = hInfo->GetCamera();

			cam->m_xRotRad = 0;
			cam->m_yRotRad = M_PI;
			cam->m_zRotRad = 0;

			cam->m_distToMid = 0;

			cam->m_xShift = 0;
			cam->m_yShift = 0;
			cam->m_zShift = 0;

			//we align the camera with the bone by copying the matrix
			cam->m_matrix = loc_focusBone->m_matrix2;

			//*(BYTE*)(General::GameBase + 0x3A6C80) = 3; //whether the q button is pressed
		}
	}
	
	
}


void AdjustCamera(ExtClass::Bone* bone) {
	if (loc_hinfo == NULL) return;
	//make sure the q button was pressed, which means bone is the first bone of participant 2 (or 1 depending on position ._.)
	if (bone != loc_hinfo->m_passiveParticipant->m_charPtr->m_bonePtrArray[0] 
		&& bone != loc_hinfo->m_activeParticipant->m_charPtr->m_bonePtrArray[0]) return;

	//switch state
	loc_state++;
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