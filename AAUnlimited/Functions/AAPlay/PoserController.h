#pragma once

#include <Windows.h>
#include <map>
#include <vector>

#include "../../External/ExternalClasses/CharacterStruct.h"
#include "../../External/ExternalClasses/Frame.h"
#include "../../External/ExternalClasses/XXFileFace.h"
#include "../../General/DirectXStructs.h"
#include "../../Files/PoseMods.h"
#include "../Shared/Globals.h"
#include "../../3rdparty/picojson/picojson.h"

namespace Poser {

	const D3DXQUATERNION unitVec[3] = { 1, 0, 0, 0,
	                                    0, 1, 0, 0,
	                                    0, 0, 1, 0 };

	class PoserController
	{
	public:
		struct SliderInfo {

			SliderInfo()
				: xxFrame(nullptr), guide(nullptr) {

			}

			void Apply();

			std::wstring frameName;
			std::wstring descr;
			PoseMods::FrameCategory category;

			ExtClass::Frame* xxFrame;
			ExtClass::Frame* guide;

			enum Operation {
				Rotate,
				Translate,
				Scale
			} currentOperation;

			struct TranslationScaleData {
				inline float rangeMin(int axis) {
					return min[axis] + delta[axis];
				}

				inline float rangeMax(int axis) {
					return max[axis] + delta[axis];
				}

				float value[3];
				float min[3];
				float max[3];
				float delta[3];
			} translate, scale;

			struct RotationData {
				D3DXQUATERNION data;
				float minEuler[3];
				float maxEuler[3];
				inline void reset() {
					Shared::D3DXQuaternionIdentity(&data);
					for (int i = 0; i < 3; i++) {
						rotAxes[i].set(unitVec[i]);
					}
				}

				inline void rotateAxis(int axis, float delta) {
					D3DXQUATERNION deltaRotation;
					(*Shared::D3DXQuaternionRotationAxis)(&deltaRotation, rotAxes[axis].vector(), delta);
					(*Shared::D3DXQuaternionMultiply)(&data, &data, &deltaRotation);
					for (int i = 0; i < 3; i++) {
						if (i != axis) {
							rotAxes[i].rotate(deltaRotation);
						}
					}
				}

				inline void setRotationQuaternion(const D3DXQUATERNION& q) {
					data = q;
					for (int i = 0; i < 3; i++) {
						rotAxes[i].set(unitVec[i]);
						rotAxes[i].rotate(q);
					}
				}

				inline void setRotationYawPitchRoll(float x, float y, float z) {
					D3DXQUATERNION rotation;
					(*Shared::D3DXQuaternionRotationYawPitchRoll)(&rotation, y, x, z);
					data = rotation;
					for (int i = 0; i < 3; i++) {
						rotAxes[i].set(unitVec[i]);
						rotAxes[i].rotate(rotation);
					}
				}

				void getEulerAngles(FLOAT* angle);

				struct RotationAxis {
					D3DXQUATERNION data;

					inline D3DXVECTOR3* vector() {
						return (D3DXVECTOR3*)&data;
					}

					inline D3DXQUATERNION* quaternion() {
						return &data;
					}

					inline void set(const D3DXVECTOR3& v) {
						data.x = v.x;
						data.y = v.y;
						data.z = v.z;
					}

					inline void set(const D3DXQUATERNION& q) {
						data = q;
					}

					inline void rotate(const D3DXQUATERNION& q) {
						D3DXQUATERNION qC;
						(*Shared::D3DXQuaternionConjugate)(&qC, &q);
						(*Shared::D3DXQuaternionMultiply)(&data, &qC, &data);
						(*Shared::D3DXQuaternionMultiply)(&data, &data, &q);
					}
				} rotAxes[3];

			} rotation;

			inline D3DXQUATERNION& getRotation() {
				return rotation.data;
			}

			void setCurrentOperation(Operation operation) {
				currentOperation = operation;
			}

			inline TranslationScaleData* getCurrentOperationData() {
				return currentOperation == Translate ? &translate : &scale;
			}

			inline float getCurrentOperationRangeMin(int axis) {
				if (currentOperation == Rotate)
					return rotation.minEuler[axis];
				else
					return getCurrentOperationData()->rangeMin(axis);
			}

			inline float getCurrentOperationRange(int axis) {
				if (currentOperation == Rotate)
					return rotation.maxEuler[axis] - rotation.minEuler[axis];
				else
					return getCurrentOperationData()->rangeMax(axis) - getCurrentOperationData()->rangeMin(axis); //map from [min,max] to [0,0x10000]
			}

			inline float getValue(int axis) {
				if (currentOperation == Rotate) {
					float angle[3];
					rotation.getEulerAngles(angle);
					return angle[axis];
				}
				return getCurrentOperationData()->value[axis];
			}

			inline void setValue(int axis, float value) {
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

			inline void setValue(float newValueX, float newValueY, float newValueZ) {
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

			inline int toSlider(int axis) {
				float coeff = 0x10000 / getCurrentOperationRange(axis);
				return int(coeff * (getValue(axis) - getCurrentOperationRangeMin(axis)));
			}

			void increment(float order, int axis);

			inline void translateSlider(float delta, int axis) {
				getCurrentOperationData()->delta[axis] = delta;
			}

			void Reset();

		}; //SliderInfo

		class PoserCharacter {
		public:

			struct Face {
				Face(XXFileFace* face) {
					m_faceStruct = face;
				}

				inline int GetEyeShape() {
					return m_faceStruct->m_eye;
				}

				inline void SetEyeShape(int shape) {
					m_faceStruct->m_eye = shape;
				}

				inline int GetEyebrows() {
					return m_faceStruct->m_eyebrow;
				}

				inline void SetEyebrows(int brows) {
					m_faceStruct->m_eyebrow = brows;
				}

				inline float GetEyeOpen() {
					return m_faceStruct->m_eyeOpen;
				}

				inline void SetEyeOpen(float open) {
					m_faceStruct->m_eyeOpen = open;
				}

				inline int GetMouthShape() {
					return m_faceStruct->m_mouth;
				}

				inline void SetMouthShape(int shape) {
					m_faceStruct->m_mouth = shape;
				}

				inline float GetMouthOpen() {
					return m_faceStruct->m_mouthOpen;
				}

				inline void SetMouthOpen(float open) {
					m_faceStruct->m_mouthOpen = open;
				}

				inline int GetBlush() {
					// default face doesn't have blushpointer set so check for that
					return m_faceStruct->blushPointer ? (int)(*m_faceStruct->GetBlush() * 9) : 0;
				}

				inline void SetBlush(int blush) {
					if (m_faceStruct->blushPointer) *m_faceStruct->GetBlush() = (float)blush / 9.0f;
				}

				inline int GetBlushLines() {
					// default face doesn't have blushpointer set so check for that
					return m_faceStruct->blushPointer ? (int)(*m_faceStruct->GetBlushLines() * 9) : 0;
				}

				inline void SetBlushLines(int lines) {
					if (m_faceStruct->blushPointer)*m_faceStruct->GetBlushLines() = (float)lines / 9.0f;
				}

				inline bool GetEyeTracking() {
					return m_faceStruct->m_eyeTracking;
				}

				inline void SetEyeTracking(bool value) {
					m_faceStruct->m_eyeTracking = value;
				}

				XXFileFace *m_faceStruct;
			}; // struct Face

			PoserCharacter(ExtClass::CharacterStruct* c, std::vector<SliderInfo> sliders);

			inline SliderInfo* CurrentSlider(PoseMods::FrameCategory category) {
							if (category == PoseMods::FrameCategory::Prop) {
								return &m_propSliders[m_currentProp];
							}
							else {
								return m_currentSlider;
							}
			}

			inline void SetCurrentSlider(PoseMods::FrameCategory category, int index) {
				if (category == PoseMods::Room) return;
				if (category == PoseMods::FrameCategory::Prop)
					m_currentProp = m_propFrames[index];
				else
					m_currentSlider = &m_sliders[index];
			}

			inline void SetCurrentProp(std::string prop) {
				m_currentProp = prop;
			}

			inline std::string GetCurrentProp() {
				return m_currentProp;
			}

			void ResetSliders() {
				for (auto it = m_sliders.begin(), end = m_sliders.end(); it != end; it++) {
					it->Reset();
					it->Apply();
				}
			}

			Face GetFace() {
				return Face(reinterpret_cast<XXFileFace*>(m_character->m_xxFace));
			}

			// index of each slider according to its frame name
			static std::map<std::string, unsigned int> s_frameMap;
			int id;
			ExtClass::CharacterStruct* m_character;
			std::vector<SliderInfo> m_sliders;
			SliderInfo *m_currentSlider;
			std::map<PoseMods::FrameCategory, int> m_CategoryCurrentSlider;
			std::map<std::string, SliderInfo> m_propSliders;
			std::string m_currentProp;
			std::vector<std::string> m_propFrames;
		}; // PoserCharacter

	public:
		PoserController();

		~PoserController();

		ExtClass::XXFile* GetXXFile(ExtClass::CharacterStruct::Models model);

		inline bool GetIsHiddenFrame(ExtClass::Frame* frame) {
			return frame->m_renderFlag == 2;
		}

		inline void SetHiddenFrame(ExtClass::Frame* frame, bool hidden) {
			frame->m_renderFlag = hidden ? 2 : 0;
		}

		void SetHidden(const char* name, bool hidden);

		inline void SetTears(bool show) {
			SetHidden("A00_O_namida", !show);
		}

		inline void SetDimEyes(bool dim) {
			SetHidden("A00_O_mehi", dim);
		}

		inline void SetTongueJuice(bool show) {
			SetHidden("A00_O_kutisiru", !show);
			SetHidden("A00_O_sitasiru", !show);
		}

		inline PoserCharacter* CurrentCharacter() {
			if (m_characters.size())
				return m_characters[m_currentCharacter];
			else
				return nullptr;
		}

		void SetCurrentCharacter(int character) {
			m_currentCharacter = character % m_characters.size();
		}
		void NewCharacter(int index);
		void SetTargetCharacter(ExtClass::CharacterStruct* c);

		inline SliderInfo* CurrentSlider() {
			if (m_currentCategory == PoseMods::FrameCategory::Room) {
				return &m_roomSliders[m_roomCurrentFrame];
			}
			else {
				return m_characters.size() ? CurrentCharacter()->CurrentSlider(m_currentCategory) : nullptr;
			}
		}

		inline void SetCurrentSlider(PoseMods::FrameCategory category, int index) {
			if (category == PoseMods::Room || category == PoseMods::Prop) return;
			CurrentCharacter()->SetCurrentSlider(category, m_sliderCategories[category][index]);
		}

		inline void SetCurrentRoomSlider(std::string room) {
			auto match = m_roomSliders.find(room);
			if (match != m_roomSliders.end()) {
				m_roomCurrentFrame = room;
			}
		}

		inline SliderInfo* GetPropSlider(std::string slider) {
			auto s = CurrentCharacter()->m_propSliders.find(slider);
			if (s != CurrentCharacter()->m_propSliders.end()) {
				return &s->second;
			}
			else
				return nullptr;
		}

		inline SliderInfo::Operation currentOperation() {
			return CurrentSlider()->currentOperation;
		}

		inline void SetCurrentCategory(PoseMods::FrameCategory category) {
			m_currentCategory = category;
		}

		bool IsActive() {
			return m_isActive;
		}
		void StartPoser();
		void StopPoser();
		void Clear();
		void GenSliderInfo();
		void UpdateUI();

		void SliderUpdate(int axis, int order, int position);
		inline void ApplyIncrement(int axis, float order) {
			CurrentSlider()->increment(order, axis);
		}

		void LoadPose(const TCHAR* path);
		void SavePose(const TCHAR* path);
		void LoadCloth(std::vector<BYTE> &file);
		void jsonToPose(PoserCharacter* c, picojson::value json);
		picojson::value poseToJson(PoserCharacter* c);

		void FrameModEvent(ExtClass::XXFile* xxFile);

		std::wstring GetOverride(const std::wstring& file);
		void SetOverride(const std::wstring& file, const std::wstring& override);
		bool IsUseGuidesEnabled();
		void SetUseGuides(bool enabled);
		bool ShowGuides();
		void SetShowGuides(bool show);

		bool m_isActive;
		PoseMods::FrameCategory m_currentCategory;
		std::vector<PoserCharacter*> m_characters;
		PoserCharacter* m_targetCharacter; // the most recent loaded character
		int m_currentCharacter;

		// holds the index of each slider in the main slider vector
		// according the the category they reside into
		// loc_sliderCategories[category][categoryIndex] = index
		std::map<PoseMods::FrameCategory, std::vector<unsigned int>> m_sliderCategories; 
		std::vector<SliderInfo> m_sliders;
		std::map<std::string, SliderInfo> m_roomSliders;
		std::string m_roomCurrentFrame;
		std::map<std::wstring, std::wstring> m_overrides;

		bool m_useGuides;
		bool m_showGuides;
};

}
