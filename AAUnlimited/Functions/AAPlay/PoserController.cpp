#include "StdAfx.h"
#include "Files/PoseMods.h"

#include <string.h>

namespace Poser {

#define X 0
#define Y 1
#define Z 2
#define W 3

	void PoserController::SliderInfo::increment(float order, int axis) {
		if (currentOperation == Rotate) {
			if (order) {
				rotation.rotateAxis(axis, (float)M_PI * order / 180.0f);
			}
			else {
				rotation.reset();
			}
		}
		else {
			if (order) {
				float delta = currentOperation == Translate ? order / 10.0f : order / 50.0f;
				getCurrentOperationData()->value[axis] += delta;
				getCurrentOperationData()->delta[axis] += delta;
			}
			else {
				getCurrentOperationData()->value[axis] = currentOperation == Scale ? 1.0f : 0.0f;
				getCurrentOperationData()->delta[axis] = 0;
			}
		}
	}

	void PoserController::SliderInfo::Reset() {
		for (int i = 0; i < 3; i++) {
			rotation.reset();
			rotation.minEuler[i] = (float)-M_PI / 2;
			rotation.maxEuler[i] = (float)M_PI / 2;
			translate.value[i] = 0;
			translate.min[i] = -2;
			translate.max[i] = 2;
			translate.delta[i] = 0;
			scale.value[i] = 1;
			scale.min[i] = 0;
			scale.max[i] = 2;
			scale.delta[i] = 0;
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
		m_currentCharacter(nullptr), m_isActive(false) {
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
		m_currentCharacter = nullptr;
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
			//resultMatrix._41 += translations.x;
			//resultMatrix._42 += translations.y;
			//resultMatrix._43 += translations.z;

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
		else if (m_targetBodyFrames.count(name)) {
			return FrameMod(m_targetBodyFrames.at(name), name);
		}
		return 0;
	}

	void PoserController::FrameModEvent(ExtClass::XXFile* xxFile) {
		if (xxFile == nullptr || m_loadCharacter == nullptr) return;
		ExtClass::CharacterStruct::Models model = General::GetModelFromName(xxFile->m_name);
		if (model == ExtClass::CharacterStruct::SKELETON || model == ExtClass::CharacterStruct::FACE ||
			model == ExtClass::CharacterStruct::TONGUE || model == ExtClass::CharacterStruct::SKIRT) {
			xxFile->EnumBonesPostOrder([&](ExtClass::Frame* bone) {
				if (bone->m_nBones) {
					for (size_t i = 0; i < bone->m_nBones; i++) {
						m_loadCharacter->m_targetBodyBones.push_back(&bone->m_bones[i]);
					}
				}
				else {
					m_loadCharacter->m_targetBodyFrames.emplace(bone->m_name, bone);
				}
			});
		}
	}

	PoserController::SliderInfo* PoserController::PoserCharacter::FrameMod(ExtClass::Frame* frame, const std::string& origName) {
		static const char prefix[]{ "pose_" };
		static const char prefixTrans[]{ "pose_tr_" };
		static const char prefixRot[]{ "pose_rot_" };
		static const char propFramePrefix[]{ "AS00_N_Prop" };

		auto InsertFrame = [](ExtClass::Frame** frame, const char* prefix) {
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

		ExtClass::Frame* transFrame = frame;
		InsertFrame(&frame, prefixTrans);
		ExtClass::Frame* rotFrame = frame;
		InsertFrame(&frame, prefixRot);

		//now, frames that represent a mesh have a bunch of bones; each bone has a pointer to its frame (more precisely,
		//its frames matrix2), which it uses to position its mesh. after this, those pointers will point to the artificial matrizes,
		//so we have to change that as well
		for (auto bone : m_targetBodyBones) {
			ExtClass::Frame* boneFrame = bone->GetFrame();
			if (boneFrame != NULL && strncmp(boneFrame->m_name, prefix, sizeof(prefix) - 1) == 0) {
				bone->SetFrame(&boneFrame->m_children[0].m_children[0]);
			}
		}

		SliderInfo* slider = new SliderInfo;
		slider->frame[0] = transFrame;
		slider->frame[1] = rotFrame;
		slider->Reset();
		slider->setCurrentOperation(PoserController::SliderInfo::Operation::Rotate);
		slider->Apply();

		m_sliders.emplace(std::string(origName), slider);

		return slider;
	}








			/*
			xxFile->EnumBonesPostOrder([&](ExtClass::Frame* bone) {

				//make copy of the bone first
				Frame* trans, *rot;
				Frame* child = nullptr;
				InsertFrame(bone, &child, prefixTrans);
				trans = bone;
				bone = child;
				InsertFrame(bone, &child, prefixRot);
				rot = bone;

				auto match = m_roomSliders.find(bone->m_name);
				if (match != m_roomSliders.end()) {
					match->second.xxFrame = bone;
					match->second.Apply();
				}
				else {
				}
			});
			*/

			/*if (General::StartsWith(bone->m_name, "guide_")) {
				auto match = sliders.find(bone->m_name + 6);
				if (match != sliders.end()) {
					match->second->guide = bone;
					SetHiddenFrame(bone, !m_showGuides);
				}
			}*/


	void PoserController::AddCharacter(ExtClass::CharacterStruct* charStruct) {
		LOGPRIO(Logger::Priority::INFO) << "Adding character" << charStruct;
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

	void PoserController::RemoveCharacter(ExtClass::CharacterStruct* charStruct) {
		LOGPRIO(Logger::Priority::INFO) << "Removing character" << charStruct;
		for (auto it = m_characters.begin(); it != m_characters.end(); it++) {
			if ((*it)->m_character == charStruct) {
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
	
	void PoserController::LoadPose(const TCHAR* path) {
		using namespace picojson;
		value json;

		PoserCharacter* character = m_currentCharacter;
		if (!character)
			return;
		character->ResetSliders();

		std::ifstream in(path);
		in >> json;

		auto& sliders = character->m_sliders;
		if (picojson::get_last_error().empty() && json.is<object>()) {
			jsonToPose(character, json);
		}
		else {
			PoseFile openFile(path);
			std::wstring str;
			SliderInfo* slider = NULL;
			for (auto elem : openFile.GetMods()) {
				auto match = sliders.find(elem.frameName);
				if (match != sliders.end()) {
					slider = match->second;
					float x, y, z;
					x = elem.matrix[0];
					y = elem.matrix[1];
					z = elem.matrix[2];
					(*Shared::D3DXQuaternionRotationYawPitchRoll)(&slider->rotation.data, y, x, z);
					slider->translate.value[X] = elem.matrix[3];
					slider->translate.value[Y] = elem.matrix[4];
					slider->translate.value[Z] = elem.matrix[5];
					slider->scale.value[X] = elem.matrix[6];
					slider->scale.value[Y] = elem.matrix[7];
					slider->scale.value[Z] = elem.matrix[8];
					slider->Apply();
				}
			}
		}
	}

	void PoserController::SavePose(const TCHAR* path) {
		PoserCharacter* character = m_currentCharacter;
		if (!character)
			return;
		std::ofstream out(path);
		out << poseToJson(character).serialize(true);
	}

	void PoserController::jsonToPose(PoserCharacter* c, picojson::value json) {
		using namespace picojson;

		if (json.is<object>()) {
			const object load = json.get<object>();
			int version = 1;
			auto& sliders = c->m_sliders;
			try {
				auto versionCheck = load.find("_VERSION_");
				if (versionCheck != load.end()) {
					version = (int)versionCheck->second.get<double>();
				}
				c->m_character->m_xxSkeleton->m_poseNumber = (int)load.at("pose").get<double>();
				c->m_character->m_xxSkeleton->m_animFrame = (float)load.at("frame").get<double>();
				object posesliders = load.at("sliders").get<object>();
				c->ResetSliders();

				auto LoadSlider = [this,&version](SliderInfo* slider, array& mods) {
					if ((mods.size() == 9 && version == 1) || (mods.size() == 10 && version == 2)) {
						float x, y, z, w;
						int index = 0;
						x = (float)mods[index++].get<double>();
						y = (float)mods[index++].get<double>();
						z = (float)mods[index++].get<double>();
						if (version == 1) {
							(*Shared::D3DXQuaternionRotationYawPitchRoll)(&slider->rotation.data, y, x, z);
						}
						else if (version == 2) {
							w = (float)mods[index++].get<double>();
							slider->rotation.data.x = x;
							slider->rotation.data.y = y;
							slider->rotation.data.z = z;
							slider->rotation.data.w = w;
						}
						slider->translate.value[X] = (float)mods[index++].get<double>();
						slider->translate.value[Y] = (float)mods[index++].get<double>();
						slider->translate.value[Z] = (float)mods[index++].get<double>();
						slider->scale.value[X] = (float)mods[index++].get<double>();
						slider->scale.value[Y] = (float)mods[index++].get<double>();
						slider->scale.value[Z] = (float)mods[index++].get<double>();
						slider->Apply();
					}
					else {
						//invalid json data
					}
				};
				for (auto s = posesliders.cbegin(); s != posesliders.cend(); s++) {
					auto match = sliders.find(s->first);
					if (match != sliders.end()) {
						array mods = (*s).second.get<array>();
						LoadSlider(match->second, mods);
					}
				}
			}
			catch (std::out_of_range& e) {
				//key doesn't exist
			}
			catch (std::runtime_error& e) {
				//invalid json data
			}

			try {
				object face = load.at("face").get<object>();
				auto it = face.find("mouth");
				if (it != face.end())
					c->GetFace().SetMouthShape((int)it->second.get<double>());
				it = face.find("mouthopen");
				if (it != face.end())
					c->GetFace().SetMouthOpen((float)it->second.get<double>());
				it = face.find("eye");
				if (it != face.end())
					c->GetFace().SetEyeShape((int)it->second.get<double>());
				it = face.find("eyeopen");
				if (it != face.end())
					c->GetFace().SetEyeOpen((float)it->second.get<double>());
				it = face.find("eyebrow");
				if (it != face.end()) {
					int currentEyebrow = c->GetFace().GetEyebrows();
					currentEyebrow = currentEyebrow - (currentEyebrow % 7);
					int newEyebrow = (int)it->second.get<double>() % 7;
					c->GetFace().SetEyebrows(currentEyebrow + newEyebrow);
				}
				it = face.find("blush");
				if (it != face.end())
					c->GetFace().SetBlush((int)(it->second.get<double>()));
				it = face.find("blushlines");
				if (it != face.end())
					c->GetFace().SetBlushLines((int)(it->second.get<double>()));
			}
			catch (std::out_of_range& e) {
				//key doesn't exist
			}
			catch (std::runtime_error& e) {
				//invalid json data
			}
		}

	}

	picojson::value PoserController::poseToJson(PoserCharacter* c) {
		using namespace picojson;
		object json;
		json["_VERSION_"] = value((double)2);
		json["pose"] = value((double)c->m_character->m_xxSkeleton->m_poseNumber);
		json["frame"] = value((double)c->m_character->m_xxSkeleton->m_animFrame);
		value::object sliders;
		auto SaveSlider = [&sliders](auto& frameName, auto& slider) {
			value::array values(10);
			values[0] = value((double)slider->rotation.data.x);
			values[1] = value((double)slider->rotation.data.y);
			values[2] = value((double)slider->rotation.data.z);
			values[3] = value((double)slider->rotation.data.w);
			values[4] = value((double)slider->translate.value[X]);
			values[5] = value((double)slider->translate.value[Y]);
			values[6] = value((double)slider->translate.value[Z]);
			values[7] = value((double)slider->scale.value[X]);
			values[8] = value((double)slider->scale.value[Y]);
			values[9] = value((double)slider->scale.value[Z]);
			sliders[frameName] = value(values);
		};
		for (auto it = c->m_sliders.begin(); it != c->m_sliders.end(); it++) {
			SaveSlider(it->first, it->second);
		}
		json["sliders"] = value(sliders);

		value::object face;
		face["eye"] = value((double)c->GetFace().GetEyeShape());
		face["eyeopen"] = value((double)c->GetFace().GetEyeOpen());
		face["eyebrow"] = value((double)c->GetFace().GetEyebrows());
		face["mouth"] = value((double)c->GetFace().GetMouthShape());
		face["mouthopen"] = value((double)c->GetFace().GetMouthOpen());
		face["blush"] = value(round((double)(c->GetFace().GetBlush())));
		face["blushlines"] = value(round((double)(c->GetFace().GetBlushLines())));
		json["face"] = value(face);
		return value(json);
	}

	void PoserController::LoadCloth(std::vector<BYTE> &file) {
		ClothFile load(file);
		if (!load.IsValid()) return;
		ExtClass::CharacterData::Clothes* cloth = &m_currentCharacter->m_character->m_charData->m_clothes[m_currentCharacter->m_character->m_currClothes];
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
