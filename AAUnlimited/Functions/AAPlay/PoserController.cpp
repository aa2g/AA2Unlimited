#include "PoserController.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <fstream>
#include <Windows.h>

#include "Files\ClothFile.h"
#include "Files\PoseFile.h"
#include "General\IllusionUtil.h"
#include "General\Util.h"

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

	PoserController::PoserCharacter::PoserCharacter(ExtClass::CharacterStruct* c, std::vector<PoserController::SliderInfo> sliders) :
		m_character(c), m_sliders(sliders)
	{
		m_currentSlider = &m_sliders[0];
	}

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

	PoserController::PoserController() : m_currentCharacter(0) {
	}

	PoserController::~PoserController()	{
	}

	void PoserController::StartPoser() {
		GenSliderInfo();
	}

	void PoserController::StopPoser() {
		Clear();
	}

	void PoserController::Clear() {
		for (PoserCharacter* c : m_characters)
			delete c;
		m_characters.clear();
		m_currentCharacter = 0;
		m_roomSliders.clear();
	}

	void PoserController::SliderInfo::Apply() {
		if (xxFrame) {
			//note that somehow those frame manipulations dont quite work as expected;
			//by observation, rotation happens around the base of the bone whos frame got manipulated,
			//rather than the tip.
			//so to correct that, we gotta translate back

			ExtClass::Frame* origFrame = &xxFrame->m_children[0];

			D3DMATRIX transMatrix;
			(*Shared::D3DXMatrixTranslation)(&transMatrix, translate.value[X], translate.value[Y], translate.value[Z]);
			D3DMATRIX rotMatrix;
			//(*Shared::D3DXMatrixRotationYawPitchRoll)(&rotMatrix, targetSlider->rotate.value[Y], targetSlider->rotate.value[X], targetSlider->rotate.value[Z]);
			(*Shared::D3DXMatrixRotationQuaternion)(&rotMatrix, &getRotation());
			D3DMATRIX scaleMatrix;
			(*Shared::D3DXMatrixScaling)(&scaleMatrix, scale.value[X], scale.value[Y], scale.value[Z]);
			D3DMATRIX matrix = origFrame->m_matrix5;
			(*Shared::D3DXMatrixMultiply)(&matrix, &matrix, &transMatrix);
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

			xxFrame->m_matrix1 = resultMatrix;
		}
	}

	void PoserController::FrameModEvent(ExtClass::XXFile* xxFile) {
		using namespace ExtClass;
		static const char prefix[]{ "pose_" };
		static const char prefixTrans[]{ "pose_tr_" };
		static const char prefixRot[]{ "pose_rot_" };
		static const char propFramePrefix[]{ "AS00_N_Prop" };
		static const char propPrefixTrans[]{ "prop_tr_" };
		static const char propPrefixRot[]{ "prop_rot_" };
		static const std::wstring suffixTrans(L" TR");
		static const std::wstring suffixRot(L" ROT");

		if (xxFile == NULL) return;
		if (m_characters.size() == 0) return;
		PoserCharacter* targetChar = m_targetCharacter;
		ExtClass::CharacterStruct::Models model;
		model = General::GetModelFromName(xxFile->m_name);
		//targetChar = loc_loadCharacter;

		auto InsertFrame = [&xxFile](ExtClass::Frame* bone, ExtClass::Frame** child, const char* prefix) {
			//make copy of the bone first
			Frame* newMatch = (Frame*)Shared::IllusionMemAlloc(sizeof(Frame));
			memcpy_s(newMatch, sizeof(Frame), bone, sizeof(Frame));

			//turn match into a copy of the root for now, since there are a lot of members i dont know
			memcpy_s(bone, sizeof(Frame), xxFile->m_root, sizeof(Frame));

			//change parent and child stuff
			bone->m_parent = newMatch->m_parent;
			bone->m_nChildren = 1;
			bone->m_children = newMatch;
			newMatch->m_parent = bone;
			for (unsigned int i = 0; i < newMatch->m_nChildren; i++) {
				newMatch->m_children[i].m_parent = newMatch;
			}

			//change name
			int namelength = newMatch->m_nameBufferSize + strlen(prefix);
			bone->m_name = (char*)Shared::IllusionMemAlloc(namelength);
			bone->m_nameBufferSize = namelength;
			strcpy_s(bone->m_name, bone->m_nameBufferSize, prefix);
			strcat_s(bone->m_name, bone->m_nameBufferSize, newMatch->m_name);
			*child = newMatch;
		};

		if (model == ExtClass::CharacterStruct::SKELETON) {
			if (targetChar->m_character->m_xxSkeleton != xxFile) {
				for (PoserCharacter* c : m_characters) {
					if (c->m_character->m_xxSkeleton == xxFile)
						targetChar = c;
				}
			}
		}
		else if (model == ExtClass::CharacterStruct::FACE || model == ExtClass::CharacterStruct::TONGUE || model == ExtClass::CharacterStruct::SKIRT) {
			//targetChar = loc_loadCharacter;
		}
		else if (model == ExtClass::CharacterStruct::H3DROOM) {
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
					SliderInfo slider;
					slider.descr = General::CastToWStringN(child->m_name, child->m_nameBufferSize) + suffixTrans;
					slider.frameName = slider.descr;
					slider.Reset();
					slider.xxFrame = trans;
					slider.category = PoseMods::FrameCategory::Room;
					slider.setCurrentOperation(PoserController::SliderInfo::Operation::Rotate);
					m_roomSliders[General::CastToString(slider.frameName)] = slider;
					slider.Apply();

					slider.descr = General::CastToWStringN(child->m_name, child->m_nameBufferSize) + suffixRot;
					slider.frameName = slider.descr;
					slider.xxFrame = rot;
					m_roomSliders[General::CastToString(slider.frameName)] = slider;
					slider.Apply();
				}
			});
		}
		else
			return; // model type isn't known so ignore

		//adjust bone matrizes
		xxFile->EnumBonesPostOrder([&](ExtClass::Frame* bone) {

			//try to find matches in both the bone rules and slider rules
			auto match = PoserCharacter::s_frameMap.find(bone->m_name);

			//apply matches by adding matrizes
			if (match != PoserCharacter::s_frameMap.end()) {
				if (model != ExtClass::CharacterStruct::SKIRT || targetChar->m_sliders[match->second].category == PoseMods::FrameCategory::Skirt) {
					//make copy of the bone first
					Frame* child = nullptr;
					InsertFrame(bone, &child, prefix);

					targetChar->m_sliders[match->second].xxFrame = bone;
					targetChar->m_sliders[match->second].Apply();
				}
			}
			else {
				if (strncmp(bone->m_name, propFramePrefix, sizeof(propFramePrefix) - 1) == 0) {
					auto& map = targetChar->m_propSliders;

					//make copy of the bone first
					Frame* trans, *rot;
					Frame* child = nullptr;
					InsertFrame(bone, &child, prefixTrans);
					trans = bone;
					bone = child;
					InsertFrame(bone, &child, prefixRot);
					rot = bone;

					auto sliderTrans = map.find(trans->m_name);
					auto sliderRot = map.find(rot->m_name);
					if (sliderTrans != map.end() && sliderRot != map.end()) {
						sliderTrans->second.xxFrame = trans;
						sliderRot->second.xxFrame = rot;
						sliderTrans->second.Apply();
						sliderRot->second.Apply();
					}
					else {
						SliderInfo info;
						info.descr = General::CastToWStringN(child->m_name, child->m_nameBufferSize) + suffixTrans;
						info.frameName = General::CastToWStringN(trans->m_name, trans->m_nameBufferSize);
						info.Reset();
						info.xxFrame = trans;
						info.setCurrentOperation(PoserController::SliderInfo::Operation::Rotate);
						info.category = PoseMods::FrameCategory::Prop;
						map[General::CastToString(info.descr)] = info;

						info.descr = General::CastToWStringN(child->m_name, child->m_nameBufferSize) + suffixRot;
						info.frameName = General::CastToWStringN(rot->m_name, rot->m_nameBufferSize);
						info.xxFrame = rot;
						map[General::CastToString(info.descr)] = info;

						info.Apply();
					}
				}
			}

		});



		//now, frames that represent a mesh have a bunch of bones; each bone has a pointer to its frame (more precisely,
		//its frames matrix2), which it uses to position its mesh. after this, those pointers will point to the artificial matrizes,
		//so we have to change that as well
		xxFile->EnumBonesPostOrder([&](ExtClass::Frame* frame) {
			for (unsigned int i = 0; i < frame->m_nBones; i++) {
				Bone* bone = &frame->m_bones[i];
				Frame* boneFrame = bone->GetFrame();
				if (boneFrame != NULL && strncmp(boneFrame->m_name, prefix, sizeof(prefix) - 1) == 0) {
					bone->SetFrame(&boneFrame->m_children[0]);
				}
			}
		});
	}

	std::map<std::string, unsigned int> PoserController::PoserCharacter::s_frameMap;

	void PoserController::GenSliderInfo() {
		if (!m_sliders.empty()) return;

		PoseMods mods(POSEMOD_FILE_PATH);
		auto& input = mods.GetInput();
		for (auto& elem : input) {
			SliderInfo info;
			PoseMods::FrameCategory category = std::get<0>(elem);
			std::string& strFrame = std::get<1>(elem);
			std::string& strDesc = std::get<2>(elem);

			std::wstring wstrFrame(strFrame.begin(), strFrame.end());
			info.frameName = wstrFrame;
			std::wstring wstrDescr(strDesc.begin(), strDesc.end());
			info.descr = wstrDescr;

			info.Reset();

			info.xxFrame = NULL;
			info.setCurrentOperation(PoserController::SliderInfo::Operation::Rotate);
			info.category = category;

			m_sliders.push_back(info);
			m_sliderCategories[category].push_back(m_sliders.size() - 1);
			PoserCharacter::s_frameMap.insert(std::make_pair(strFrame, m_sliders.size() - 1));
		}
	}

	void PoserController::SetTargetCharacter(ExtClass::CharacterStruct* charStruct) {
		PoserCharacter* character = nullptr;
		for (PoserCharacter* c : m_characters) {
			if (c->m_character == charStruct)
				character = c;
		}
		if (!character) {
			character = new PoserCharacter(charStruct, m_sliders);
			m_characters.push_back(character);
		}
		m_targetCharacter = character;
	}
	
	void PoserController::LoadPose(const TCHAR* path) {
		using namespace picojson;
		value json;

		PoserCharacter* character = CurrentCharacter();
		if (!character)
			return;
		for (SliderInfo& slider : character->m_sliders) {
			slider.Reset();
			slider.Apply();
		}

		std::ifstream in(path);
		in >> json;

		if (picojson::get_last_error().empty() && json.is<object>()) {
			jsonToPose(character, json);
		}
		else {
			PoseFile openFile(path);
			std::wstring str;
			SliderInfo* slider = NULL;
			for (auto elem : openFile.GetMods()) {
				auto match = character->s_frameMap.find(elem.frameName);
				if (match != character->s_frameMap.end()) {
					slider = &character->m_sliders[match->second];
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
		PoserCharacter* character = CurrentCharacter();
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
			try {
				auto versionCheck = load.find("_VERSION_");
				if (versionCheck != load.end()) {
					version = (int)versionCheck->second.get<double>();
				}
				c->m_character->m_xxSkeleton->m_poseNumber = (int)load.at("pose").get<double>();
				c->m_character->m_xxSkeleton->m_animFrame = (float)load.at("frame").get<double>();
				object sliders = load.at("sliders").get<object>();
				for (auto it = c->m_sliders.begin(); it != c->m_sliders.end(); it++) {
					it->Reset();
					(*it).Apply();
				}

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
				for (auto s = sliders.cbegin(); s != sliders.cend(); s++) {
					auto match = c->s_frameMap.find((*s).first);
					if (match != c->s_frameMap.end()) {
						array mods = (*s).second.get<array>();
						LoadSlider(&c->m_sliders[match->second], mods);
					}
					else {
						auto match = c->m_propSliders.find((*s).first);
						if (match != c->m_propSliders.end()) {
							array mods = (*s).second.get<array>();
							LoadSlider(&match->second, mods);
						}
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
		auto SaveSlider = [&sliders](auto& slider) {
			value::array values(10);
			values[0] = value((double)slider.rotation.data.x);
			values[1] = value((double)slider.rotation.data.y);
			values[2] = value((double)slider.rotation.data.z);
			values[3] = value((double)slider.rotation.data.w);
			values[4] = value((double)slider.translate.value[X]);
			values[5] = value((double)slider.translate.value[Y]);
			values[6] = value((double)slider.translate.value[Z]);
			values[7] = value((double)slider.scale.value[X]);
			values[8] = value((double)slider.scale.value[Y]);
			values[9] = value((double)slider.scale.value[Z]);
			sliders[std::string(slider.frameName.cbegin(), slider.frameName.cend())] = value(values);
		};
		for (SliderInfo& slider : c->m_sliders) {
			SaveSlider(slider);
		}
		for (auto i = c->m_propSliders.begin(); i != c->m_propSliders.end(); i++) {
			SaveSlider(i->second);
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
		ExtClass::CharacterData::Clothes* cloth = &CurrentCharacter()->m_character->m_charData->m_clothes[CurrentCharacter()->m_character->m_currClothes];
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

}
