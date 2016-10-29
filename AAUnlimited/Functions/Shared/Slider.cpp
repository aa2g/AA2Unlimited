#include "Slider.h"


namespace Shared {


	void Slider::ModifySRT(D3DVECTOR3* scale,D3DVECTOR3* rot,D3DVECTOR3* trans,Slider::Operation op,const AAUCardData::BoneMod& mod) {
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
			D3DQUATERNION rotQuat;
			(*Shared::D3DXQuaternionRotationYawPitchRoll)(&rotQuat,elem.rotations[1],elem.rotations[0],elem.rotations[2]);
			D3DQUATERNION* origQuat = (D3DQUATERNION*)&frame->m_quatX;
			(*Shared::D3DXQuaternionMultiply)(origQuat,origQuat,&rotQuat);
			frame->m_transX += elem.transformations[0];
			frame->m_transY += elem.transformations[1];
			frame->m_transZ += elem.transformations[2];
			break; }
		case MULTIPLY: {
			frame->m_scaleX *= elem.scales[0];
			frame->m_scaleY *= elem.scales[1];
			frame->m_scaleZ *= elem.scales[2];
			D3DQUATERNION rotQuat;
			(*Shared::D3DXQuaternionRotationYawPitchRoll)(&rotQuat,elem.rotations[1],elem.rotations[0],elem.rotations[2]);
			D3DQUATERNION* origQuat = (D3DQUATERNION*)&frame->m_quatX;
			(*Shared::D3DXQuaternionMultiply)(origQuat,origQuat,&rotQuat);
			frame->m_transX *= elem.transformations[0];
			frame->m_transY *= elem.transformations[1];
			frame->m_transZ *= elem.transformations[2];
			break; }
		case DIVIDE: {
			if(elem.scales[0] != 0) frame->m_scaleX /= elem.scales[0];
			if (elem.scales[1] != 0) frame->m_scaleY /= elem.scales[1];
			if (elem.scales[2] != 0) frame->m_scaleZ /= elem.scales[2];
			D3DQUATERNION rotQuat;
			(*Shared::D3DXQuaternionRotationYawPitchRoll)(&rotQuat,elem.rotations[1],elem.rotations[0],elem.rotations[2]);
			D3DQUATERNION* origQuat = (D3DQUATERNION*)&frame->m_quatX;
			(*Shared::D3DXQuaternionMultiply)(origQuat,origQuat,&rotQuat);
			if (elem.transformations[0] != 0) frame->m_transX /= elem.transformations[0];
			if (elem.transformations[1] != 0) frame->m_transY /= elem.transformations[1];
			if (elem.transformations[2] != 0) frame->m_transZ /= elem.transformations[2];
			break; }
		default:
			break;
		}
	}


}