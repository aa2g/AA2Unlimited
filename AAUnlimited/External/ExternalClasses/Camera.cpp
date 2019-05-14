#include "StdAfx.h"

namespace ExtClass {
ExtClass::Frame* loc_focusBone = NULL;
D3DXVECTOR3 loc_focusOffset{ 0,0,0 };
static bool loc_zunlock;
static bool needRotParams;
int needRotParamsDelay = 10;
float stabilizeCoefficient = 0;
//int debug_i = 1;

void Camera::PostTick(ExtClass::HInfo* hInfo, bool running) {
	if (!(running && loc_focusBone))
		return;
	ExtClass::Camera* cam = hInfo->GetCamera();
	cam->m_distToMid = 0;
	if (!loc_zunlock)
		cam->m_zRotRad = 0;

	cam->m_xShift = 0;
	cam->m_yShift = 0;
	cam->m_zShift = 0;

	auto mat = loc_focusBone->m_matrix2;
	//adjust with the offsets BEFORE the matrix (not doing an actual multiplication here cause its just a translation)
	float x = loc_focusOffset.x, y = loc_focusOffset.y, z = loc_focusOffset.z;

	if (needRotParams) { // In the first 10 frames we also need reset the camera (Otherwise, it is overwritten somewhere else ;=( )
		if (needRotParamsDelay < 1) {
			needRotParams = false;
			needRotParamsDelay = 10;
			//LOGPRIONC(Logger::Priority::INFO) "-2--needRotParams--" << debug_i << "___" << cam->m_fov << "\r\n"; debug_i++;
		}
		needRotParamsDelay--;

		mat._41 += x * mat._11 + y * mat._21 + z * mat._31;
		mat._42 += x * mat._12 + y * mat._22 + z * mat._32;
		mat._43 += x * mat._13 + y * mat._23 + z * mat._33;
		cam->m_matrix = mat;
	}
	else {
		// Bone coords with offsets (without Rotating)
		auto mat_cam = cam->m_matrix;
		mat_cam._41 += (mat._41 + x * mat._11 + y * mat._21 + z * mat._31 - mat_cam._41) * stabilizeCoefficient;
		mat_cam._42 += (mat._42 + x * mat._12 + y * mat._22 + z * mat._32 - mat_cam._42) * stabilizeCoefficient;
		mat_cam._43 += (mat._43 + x * mat._13 + y * mat._23 + z * mat._33 - mat_cam._43) * stabilizeCoefficient;

		cam->m_matrix = mat_cam;
	}
}

int Camera::SetFocusBone(ExtClass::Frame* bone, double x, double y, double z, bool zunlock) {
	//LOGPRIONC(Logger::Priority::INFO) "-2--SetFocusBone--" << debug_i << "\r\n"; debug_i++;
	needRotParams = true;
	loc_zunlock = zunlock;
	loc_focusBone = bone;
	loc_focusOffset.x = x;
	loc_focusOffset.y = y;
	loc_focusOffset.z = z;
	return 1;
}

void Camera::InitPovParams(int stabilize_percents) {
	stabilizeCoefficient = (float)stabilize_percents / 100;
}

void Camera::ResetPovToNormal() {
	Controls::keyPress(0x11); // W key
	LUA_EVENT_NORET("facecam_deactivate"); // Verify reset status (if key W not reset)
}

}
