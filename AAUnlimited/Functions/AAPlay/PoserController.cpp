#include "StdAfx.h"
#include "Functions/AAPlay/PoserController.h"


#include <cmath>
#include <string.h>

namespace Poser {

#define X 0
#define Y 1
#define Z 2
#define W 3

	void PoserController::SliderInfo::setValue(int axis, float value) {
		if (currentOperation == Rotate) {
			float angle[3];
			rotation.getEulerAngles(angle);
			angle[axis] = value;
			(*Shared::D3DXQuaternionRotationYawPitchRoll)(&getRotation(), angle[1], angle[0], angle[2]);
		}
		else {
			getCurrentOperationData()->value[axis] = value;
		}
	}

	void PoserController::SliderInfo::setValue(float newValueX, float newValueY, float newValueZ) {
		if (currentOperation == Rotate) {
			rotation.setRotationYawPitchRoll(newValueX, newValueY, newValueZ);
		}
		else {
			float* data = getCurrentOperationData()->value;
			data[0] = newValueX;
			data[1] = newValueY;
			data[2] = newValueZ;
		}
	}

	void PoserController::SliderInfo::increment(float order, int axis) {
		if (currentOperation == Rotate) {
			if (order) {
				rotation.rotateAxis(axis, order, sliding, slidingRotData);
			}
			else {
				rotation.reset();
			}
		}
		else {
			if (order) {
				if (!sliding)
					getCurrentOperationData()->value[axis] += order;
				else
					getCurrentOperationData()->value[axis] = slidingTSData[axis] + order;
			}
			else {
				getCurrentOperationData()->value[axis] = currentOperation == Scale ? 1.0f : 0.0f;
			}
		}
	}

	void PoserController::SliderInfo::Reset() {
		rotation.reset();
		for (int i = 0; i < 3; i++) {
			translate.value[i] = 0;
			scale.value[i] = 1;
		}
	}

	void PoserController::SliderInfo::RotationData::getEulerAngles(FLOAT* angle) {
		D3DXQUATERNION* pQ = &data;
		float yy = pQ->y * pQ->y;

		float t0 = 2.0f * (pQ->w * pQ->x + pQ->y * pQ->z);
		float t1 = 1.0f - 2.0f * (pQ->x * pQ->x + yy);
		angle[0] = std::atan2(t0, t1);

		float t2 = 2.0f * (pQ->w * pQ->y - pQ->z * pQ->x);
		t2 = t2 > 1.0f ? 1.0f : t2;
		t2 = t2 < -1.0f ? -1.0f : t2;
		angle[1] = std::asin(t2);

		float t3 = 2.0f * (pQ->w * pQ->z + pQ->x * pQ->y);
		float t4 = 1.0f - 2.0f * (yy + pQ->z * pQ->z);
		angle[2] = std::atan2(t3, t4);
	}

	PoserController::PoserCharacter::PoserCharacter(ExtClass::CharacterStruct* c) :
		m_character(c)
	{
	}

	PoserController::PoserCharacter::~PoserCharacter() {
		for (auto it = m_sliders.begin(); it != m_sliders.end(); it++) {
			delete it->second;
		}
	}
	/*
	void PoserController::SetHidden(const char* name, bool hidden) {
		ExtClass::Frame** frame = CurrentCharacter()->m_character->m_bonePtrArray;
		ExtClass::Frame** arrayEnd= CurrentCharacter()->m_character->m_bonePtrArrayEnd;
		while (frame < arrayEnd) {
			if (*frame != nullptr) {
				if (strstr((*frame)->m_name, name)) {
					SetHiddenFrame(*frame, hidden);
				}
			}
			frame++;
		}
	}
	*/
	PoserController::PoserController() :
		m_isActive(false) {
	}

	PoserController::~PoserController()	{
	}

	void PoserController::StartPoser() {
		m_isActive = true;
	}

	void PoserController::StopPoser() {
		Clear();
		m_isActive = false;
	}

	void PoserController::Clear() {
		for (PoserCharacter* c : m_characters)
			delete c;
		m_characters.clear();
	}

	void PoserController::SliderInfo::Apply() {
		if (frame[0] && frame[1]) {
			//note that somehow those frame manipulations dont quite work as expected;
			//by observation, rotation happens around the base of the bone whos frame got manipulated,
			//rather than the tip.
			//so to correct that, we gotta translate back

			ExtClass::Frame* origFrame = &frame[1]->m_children[0];

			D3DMATRIX transMatrix;
			(*Shared::D3DXMatrixTranslation)(&transMatrix, translate.value[X], translate.value[Y], translate.value[Z]);
			frame[0]->m_matrix1 = transMatrix;

			D3DMATRIX rotMatrix;
			//(*Shared::D3DXMatrixRotationYawPitchRoll)(&rotMatrix, targetSlider->rotate.value[Y], targetSlider->rotate.value[X], targetSlider->rotate.value[Z]);
			(*Shared::D3DXMatrixRotationQuaternion)(&rotMatrix, &getRotation());
			D3DMATRIX scaleMatrix;
			(*Shared::D3DXMatrixScaling)(&scaleMatrix, scale.value[X], scale.value[Y], scale.value[Z]);
			D3DMATRIX matrix = origFrame->m_matrix5;
			(*Shared::D3DXMatrixMultiply)(&matrix, &matrix, &rotMatrix);
			(*Shared::D3DXMatrixMultiply)(&rotMatrix, &scaleMatrix, &rotMatrix);

			D3DXVECTOR3 translations;
			translations.x = origFrame->m_matrix5._41 - matrix._41;
			translations.y = origFrame->m_matrix5._42 - matrix._42;
			translations.z = origFrame->m_matrix5._43 - matrix._43;

			D3DMATRIX resultMatrix = rotMatrix;
			resultMatrix._41 += translations.x;
			resultMatrix._42 += translations.y;
			resultMatrix._43 += translations.z;

			frame[1]->m_matrix1 = resultMatrix;
		}
	}

	PoserController::SliderInfo* PoserController::PoserCharacter::GetSlider(const char* name) {
		if (name)
			return GetSlider(std::string(name));
		return nullptr;
	}

	PoserController::SliderInfo* PoserController::PoserCharacter::GetSlider(const std::string& name) {
		if (m_sliders.count(name)) {
			return m_sliders.at(name);
		}
		return 0;
	}

	static const char prefix[]{ "pose_" };
	static const char prefixTrans[]{ "pose_tr_" };
	static const char prefixRot[]{ "pose_rot_" };
	static const char propFramePrefix[]{ "AS00_N_Prop" };
	void PoserController::FrameModEvent(ExtClass::XXFile* xxFile) {
		LOGPRIONC(Logger::Priority::SPAM) "FrameModEvent " << xxFile;
		if (xxFile->m_name)
			LOGPRIONC(Logger::Priority::SPAM) "(" << xxFile->m_name << ")";
		LOGPRIONC(Logger::Priority::SPAM) "\r\n";
		ExtClass::CharacterStruct::Models model = General::GetModelFromName(xxFile->m_name);

		if (model == ExtClass::CharacterStruct::H3DROOM)
			FrameModRoom(xxFile);

		if (m_loadCharacter == nullptr) return;
		bool modded = false;
		if (model == ExtClass::CharacterStruct::SKELETON) {
			m_loadCharacter->FrameModSkeleton(xxFile);
			modded = true;
		}
		else if (model == ExtClass::CharacterStruct::FACE || model == ExtClass::CharacterStruct::TONGUE) {
			m_loadCharacter->FrameModFace(xxFile);
			modded = true;
		}
		else if (model == ExtClass::CharacterStruct::SKIRT) {
			m_loadCharacter->FrameModSkirt(xxFile);
			modded = true;
		}

		if (modded) {
			xxFile->EnumBonesPostOrder([&](ExtClass::Frame* frame) {
				for (unsigned int i = 0; i < frame->m_nBones; i++) {
					ExtClass::Bone* bone = &frame->m_bones[i];
					ExtClass::Frame* boneFrame = bone->GetFrame();
					if (boneFrame != NULL && strncmp(boneFrame->m_name, prefixTrans, sizeof(prefixTrans) - 1) == 0) {
						bone->SetFrame(&boneFrame->m_children[0].m_children[0]);
					}
				}
			});
		}
	}

	void FrameModInsertFrame(ExtClass::Frame** frame, const char* prefix) {
		//make copy of the bone first
		ExtClass::Frame* newMatch = (ExtClass::Frame*)Shared::IllusionMemAlloc(sizeof(ExtClass::Frame));
		memcpy_s(newMatch, sizeof(ExtClass::Frame), (*frame), sizeof(ExtClass::Frame));

		//turn match into a copy of the root for now, since there are a lot of members i dont know
		memcpy_s((*frame), sizeof(ExtClass::Frame), (*frame)->m_xxPartOf->m_root, sizeof(ExtClass::Frame));

		//change parent and child stuff
		(*frame)->m_parent = newMatch->m_parent;
		(*frame)->m_nChildren = 1;
		(*frame)->m_children = newMatch;
		newMatch->m_parent = *frame;
		for (unsigned int i = 0; i < newMatch->m_nChildren; i++) {
			newMatch->m_children[i].m_parent = newMatch;
		}

		//change name
		int namelength = newMatch->m_nameBufferSize + strlen(prefix);
		(*frame)->m_name = (char*)Shared::IllusionMemAlloc(namelength);
		(*frame)->m_nameBufferSize = namelength;
		strcpy_s((*frame)->m_name, (*frame)->m_nameBufferSize, prefix);
		strcat_s((*frame)->m_name, (*frame)->m_nameBufferSize, newMatch->m_name);
		*frame = newMatch;
	};

	void PoserController::SwapTransientSliders(bool skipSkeleton) {
		{
			auto it = m_loadCharacter->m_transientSliders.begin();
			while (it != m_loadCharacter->m_transientSliders.end()) {
				delete it->second;
				it = m_loadCharacter->m_transientSliders.erase(it);
			}
		}
		{
			auto it = m_loadCharacter->m_sliders.begin();
			while (it != m_loadCharacter->m_sliders.end()) {
				if (skipSkeleton && it->second->source == ExtClass::CharacterStruct::SKELETON) {
					m_loadCharacter->m_transientSliders[it->first] = it->second;
					it = m_loadCharacter->m_sliders.erase(it);
				}
				else {
					it++;
				}
			}
		}
		std::swap(m_loadCharacter->m_transientSliders, m_loadCharacter->m_sliders);
	}

	inline void FrameMod(ExtClass::Frame** origFrame, ExtClass::Frame** transFrame, ExtClass::Frame** rotFrame) {
		ExtClass::Frame* frame = *origFrame;
		*transFrame = frame;
		FrameModInsertFrame(&frame, prefixTrans);
		*rotFrame = frame;
		FrameModInsertFrame(&frame, prefixRot);
		*origFrame = frame;
	}

	void PoserController::PoserCharacter::FrameModTree(ExtClass::Frame* tree, ExtClass::CharacterStruct::Models source, const char* filter) {
		PoserController::SliderInfo* slider;
		ExtClass::Frame* transFrame;
		ExtClass::Frame* rotFrame;
		size_t len = filter? strlen(filter) : 0;

		tree->EnumTreeLevelOrder([this, &slider, &transFrame, &rotFrame, &source, &filter, &len](ExtClass::Frame* frame) {
			// limit the frames we work on based on a starts-with filter criteria
			if (!filter || strncmp(frame->m_name, filter, len) == 0) {
				// Search for this frame slider if it exists
				slider = GetSlider(frame->m_name);
				// Search for and recover from the transient slider list
				if (!slider) {
					auto match = m_transientSliders.find(frame->m_name);
					if (match != m_transientSliders.end() && match->second->source == source) {
						slider = match->second;
						m_transientSliders.erase(match);
					}
				}
				// If it doesn't exist we create a new one and claim it for this model source
				// A bone shall not be shared between different sources. The first one to claim it has priority. i.e. skeleton
				if (!slider) {
					slider = new SliderInfo;
					m_sliders.emplace(std::string(frame->m_name), slider);
					slider->setCurrentOperation(PoserController::SliderInfo::Operation::Rotate);
					slider->Reset();
					slider->source = source;
				}

				// If the models match we can mod this frame as it either:
				// * has not been claimed yet
				// * we're updating this model
				if (slider->source == source) {
					FrameMod(&frame, &transFrame, &rotFrame);

					slider->frame[0] = transFrame;
					slider->frame[1] = rotFrame;
					slider->Apply();
				}
				slider = nullptr;
			}
			return true;
		});
	}

	void PoserController::PoserCharacter::FrameModSkeleton(ExtClass::XXFile* xxFile) {
		ExtClass::Frame* root = xxFile->m_root;
		ExtClass::Frame* world = root->FindFrame("a01_N_Zentai_010");

		if (world) {
			FrameModTree(world, ExtClass::CharacterStruct::SKELETON, "a01");
		}

		ExtClass::Frame* dankon = root->FindFrame("a00_N_Dankon_01");
		if (dankon) {
			FrameModTree(dankon, ExtClass::CharacterStruct::SKELETON);
		}
	}

	void PoserController::PoserCharacter::FrameModFace(ExtClass::XXFile* xxFile) {
		std::queue<std::string> targets;
		targets.push("A00_O_mimi");
		targets.push("A00_J_mayumaba");
		targets.push("A00_J_kao");

		ExtClass::Frame* root = xxFile->m_root;
		ExtClass::Frame* t;
		std::string tn;
		while (!targets.empty()) {
			tn = targets.front();
			t = root->FindFrame(tn.c_str());
			if (t)
				FrameModTree(t, ExtClass::CharacterStruct::FACE);
			targets.pop();
		}

		t = root->FindFrame("A00_J_sita00");
		if (t) {
			FrameModTree(t, ExtClass::CharacterStruct::TONGUE);
		}
	}

	void PoserController::PoserCharacter::FrameModSkirt(ExtClass::XXFile* xxFile) {
		ExtClass::Frame* root = xxFile->m_root;
		ExtClass::Frame* sukato = root->FindFrame("A00_N_sukato");

		if (sukato) {
			FrameModTree(sukato, ExtClass::CharacterStruct::SKIRT, "a01_J_SK");
		}
	}

	void PoserController::FrameModRoom(ExtClass::XXFile* xxFile) {

	}

	void PoserController::LoadCharacter(ExtClass::CharacterStruct* charStruct) {
		PoserCharacter* character = nullptr;
		for (PoserCharacter* c : m_characters) {
			if (c->m_character == charStruct)
				character = c;
		}
		if (!character) {
			character = new PoserCharacter(charStruct);
			m_characters.push_back(character);
		}
		character->VoidFramePointers();
		m_loadCharacter = character;
	}

	void PoserController::UpdateCharacter(ExtClass::CharacterStruct* charStruct) {
		PoserCharacter* character = nullptr;
		for (PoserCharacter* c : m_characters) {
			if (c->m_character == charStruct)
				character = c;
		}
		//character->VoidFramePointers();
		m_loadCharacter = character;
	}

	void PoserController::RemoveCharacter(ExtClass::CharacterStruct* charStruct) {
		for (auto it = m_characters.begin(); it != m_characters.end(); it++) {
			if ((*it)->m_character == charStruct) {
				delete *it;
				m_characters.erase(it);
				break;
			}
		}
	}

	PoserController::PoserCharacter* PoserController::GetPoserCharacter(ExtClass::CharacterStruct* c) {
		for (auto charas : m_characters) {
			if (charas->m_character == c)
				return charas;
		}
		return nullptr;
	}
	
	void PoserController::LoadCloth(std::vector<BYTE> &file) {
		ClothFile load(file);
		if (!load.IsValid()) return;
		ExtClass::CharacterData::Clothes* cloth = nullptr;
		cloth->slot = load.m_slot;
		cloth->skirtLength = load.m_shortSkirt;
		cloth->socks = load.m_socksId;
		cloth->indoorShoes = load.m_shoesIndoorId;
		cloth->outdoorShoes = load.m_shoesOutdoorId;
		cloth->isOnePiece = load.m_isOnePiece;
		cloth->hasUnderwear = load.m_hasUnderwear;
		cloth->hasSkirt = load.m_hasSkirt;
		cloth->colorTop1 = load.m_colorTop1;
		cloth->colorTop2 = load.m_colorTop2;
		cloth->colorTop3 = load.m_colorTop3;
		cloth->colorTop4 = load.m_colorTop4;
		cloth->colorBottom1 = load.m_colorBottom1;
		cloth->colorBottom2 = load.m_colorBottom2;
		cloth->colorUnderwear = load.m_colorUnderwear;
		cloth->colorSocks = load.m_colorSocks;
		cloth->colorIndoorShoes = load.m_colorIndoorShoes;
		cloth->colorOutdoorShoes = load.m_colorOutdoorShoes;
		cloth->textureBottom1 = load.m_skirtTextureId;
		cloth->textureUnderwear = load.m_underwearTextureId;
		cloth->textureBottom1Hue = load.m_skirtHue;
		cloth->textureBottom1Lightness = load.m_skirtBrightness;
		cloth->shadowBottom1Hue = load.m_skirtShadowHue;
		cloth->shadowBottom1Lightness = load.m_skirtShadowBrightness;
		cloth->textureUnderwearHue = load.m_underwearHue;
		cloth->textureUnderwearLightness = load.m_underwearBrightness;
		cloth->shadowUnderwearHue = load.m_underwearShadowHue;
		cloth->shadowUnderwearLightness = load.m_underwearShadowBrightness;
	}

	std::wstring PoserController::GetOverride(const std::wstring& file) {
		//A00_00_01_00h.xx - skeleton
		/*if (m_useGuides && file.length() == 16 && General::StartsWith(file, L"A00_00") && file.substr(10, 6) == L"00h.xx") {
			return L"poser\\skeleton\\female_guides.xx";
		}*/
		auto it = m_overrides.find(file);
		if (it != m_overrides.end()) {
			return it->second;
		}
		return std::wstring();
	}

	void PoserController::SetOverride(const std::wstring& file, const std::wstring& override) {
		if (override.empty()) {
			m_overrides.erase(file);
		}
		else {
			m_overrides[file] = override;
		}
	}
}
