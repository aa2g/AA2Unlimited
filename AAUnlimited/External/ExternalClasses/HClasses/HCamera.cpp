#include "Files\Config.h"
#include "External\ExternalClasses\HClasses\HInfo.h"
#include "External\ExternalClasses\Frame.h"
#include "General\ModuleInfo.h"
#include "Files\Logger.h"
#include "Functions\Shared\Globals.h"
#include "HCamera.h"

namespace ExtClass {
ExtClass::Frame* loc_focusBone = NULL;
D3DXVECTOR3 loc_focusOffset{ 0,0,0 };

void HCamera::PostTick(ExtClass::HInfo* hInfo, bool running) {
	if (!(running && loc_focusBone))
		return;
	ExtClass::HCamera* cam = hInfo->GetCamera();
	cam->m_zRotRad = 0;
	cam->m_distToMid = 0;

	cam->m_xShift = 0;
	cam->m_yShift = 0;
	cam->m_zShift = 0;

	auto mat = loc_focusBone->m_matrix2;
	//adjust with the offsets BEFORE the matrix (not doing an actual multiplication here cause its just a translation)
	float x = loc_focusOffset.x, y = loc_focusOffset.y, z = loc_focusOffset.z;
	mat._41 += x*mat._11 + y*mat._21 + z*mat._31;
	mat._42 += x*mat._12 + y*mat._22 + z*mat._32;
	mat._43 += x*mat._13 + y*mat._23 + z*mat._33;
	cam->m_matrix = mat;
}

void HCamera::SetFocusBone(ExtClass::Frame* bone, double x, double y, double z) {
	loc_focusBone = bone;
	loc_focusOffset.x = x;
	loc_focusOffset.y = y;
	loc_focusOffset.z = z;
}

}