#pragma once

#include <Windows.h>
#include <map>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "../../External/ExternalClasses/CharacterStruct.h"
#include "../../External/ExternalClasses/Frame.h"
#include "../../External/ExternalClasses/XXFileFace.h"
#include "../../General/DirectXStructs.h"
#include "../../Files/PoseMods.h"
#include "../Shared/Globals.h"
#include "../../3rdparty/picojson/picojson.h"
#include "Script/glua_stl.h"

namespace Poser {

	const D3DXQUATERNION unitVec[3] = { 1, 0, 0, 0,
	                                    0, 1, 0, 0,
	                                    0, 0, 1, 0 };

	class PoserController
	{
	public:
		struct SliderInfo {

			SliderInfo()
				: guide(nullptr) {
				frame.resize(2);
				frame[0] = nullptr;
				frame[1] = nullptr;
				Reset();
				sliding = false;
			}

			void Apply();

			std::vector<ExtClass::Frame*> frame;
			ExtClass::Frame* guide;

			bool sliding;
			D3DXQUATERNION slidingRotData;
			float slidingTSData[3];

			enum Operation {
				Rotate,
				Translate,
				Scale
			} currentOperation;

			struct TranslationScaleData {
				float value[3];
			} translate, scale;

			struct RotationData {
				D3DXQUATERNION data;
				
				inline void reset() {
					Shared::D3DXQuaternionIdentity(&data);
					for (int i = 0; i < 3; i++) {
						rotAxes[i].set(unitVec[i]);
					}
				}

				inline void rotateAxis(int axis, float delta, const bool& sliding, const D3DXQUATERNION& slideData) {
					D3DXQUATERNION deltaRotation;
					(*Shared::D3DXQuaternionRotationAxis)(&deltaRotation, rotAxes[axis].vector(), delta);
					if (sliding) {
						(*Shared::D3DXQuaternionMultiply)(&data, &slideData, &deltaRotation);
						setRotationQuaternion(data);
					}
					else {
						(*Shared::D3DXQuaternionMultiply)(&data, &data, &deltaRotation);
						for (int i = 0; i < 3; i++) {
							if (i != axis) {
								rotAxes[i].rotate(deltaRotation);
							}
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

			inline void startSlide() {
				sliding = true;
				if (currentOperation == Rotate) {
					slidingRotData = rotation.data;
				}
				if (currentOperation == Translate) {
					slidingTSData[0] = translate.value[0];
					slidingTSData[1] = translate.value[1];
					slidingTSData[2] = translate.value[2];
				}
				if (currentOperation == Scale) {
					slidingTSData[0] = scale.value[0];
					slidingTSData[1] = scale.value[1];
					slidingTSData[2] = scale.value[2];
				}
			}
			inline void stopSlide() {
				sliding = false;
			}

			void increment(float order, int axis);

			void Reset();

#define LUA_CLASS PoserController::SliderInfo
			static inline void bindLua() {
				LUA_NAME;
				LUA_METHOD(Apply, {
					_self->Apply();
				});
				LUA_METHOD(SetCurrentOperation, {
					_self->setCurrentOperation((Operation)(int)_gl.get(2));
				});
				LUA_METHOD(Increment, {
					_self->increment(_gl.get(2), _gl.get(3));
					_self->Apply();
				});
				LUA_METHOD(StartSlide, {
					_self->startSlide();
				});
				LUA_METHOD(StopSlide, {
					_self->stopSlide();
				});
				LUA_BINDARRE(translate, .value, 3);
				LUA_BINDARRE(scale, .value, 3);
				GLUA_BIND(LUA_GLOBAL, METHOD, LUA_CLASS, rotation, { \
					unsigned _idx = _gl.get(2); \
					if (_idx > 4) return 0; \
					if (_gl.top() == 2) { \
						_gl.push(((float*)(&_self->rotation.data))[_idx]); \
						return 1; \
					} \
					((float*)(&_self->rotation.data))[_idx] = _gl.get(3); \
				});
			}
#undef LUA_CLASS
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

			PoserCharacter(ExtClass::CharacterStruct* c);
			~PoserCharacter();

			inline void ResetSliders() {
				for (auto it = m_sliders.begin(), end = m_sliders.end(); it != end; it++) {
					it->second->Reset();
					it->second->Apply();
				}
			}

			inline void VoidFramePointers() {
				SliderInfo* slider = nullptr;
				for (auto it = m_sliders.begin(), end = m_sliders.end(); it != end; it++) {
					slider = it->second;
					slider->frame[0] = nullptr;
					slider->frame[1] = nullptr;
					slider->guide = nullptr;
				}
			}

			Face GetFace() {
				return Face(reinterpret_cast<XXFileFace*>(m_character->m_xxFace));
			}

			void FrameModTree(ExtClass::Frame* tree, const char* filter = nullptr);
			void FrameModSkeleton(ExtClass::XXFile* xxFile);
			void FrameModFace(ExtClass::XXFile* xxFile);
			void FrameModSkirt(ExtClass::XXFile* xxFile);
			SliderInfo* GetSlider(const char* name);
			SliderInfo* GetSlider(const std::string& name);

			ExtClass::CharacterStruct* m_character;
			std::unordered_map<std::string, SliderInfo*> m_sliders;

#define LUA_CLASS PoserController::PoserCharacter
			static inline void bindLua() {
				LUA_NAME;
				LUA_MGETTER1(GetSlider);
				LUA_MAPITERATOR(Sliders, m_sliders)
			}
#undef LUA_CLASS
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

		void SetCurrentCharacter(ExtClass::CharacterStruct* character) {
			for (auto it = m_characters.begin(); it != m_characters.end(); it++) {
				if ((*it)->m_character == character) {
					m_currentCharacter = *it;
				}
			}
		}

		void LoadCharacter(ExtClass::CharacterStruct* c);
		void UpdateCharacter(ExtClass::CharacterStruct* c);
		void RemoveCharacter(ExtClass::CharacterStruct* c);
		PoserCharacter* GetPoserCharacter(ExtClass::CharacterStruct* c);

		bool IsActive() {
			return m_isActive;
		}
		void StartPoser();
		void StopPoser();
		void Clear();

		void LoadPose(const TCHAR* path);
		void SavePose(const TCHAR* path);
		void LoadScene(const TCHAR* path);
		void SaveScene(const TCHAR* path);
		void LoadCloth(std::vector<BYTE> &file);
		void jsonToPose(PoserCharacter* c, picojson::value json);
		picojson::value poseToJson(PoserCharacter* c);

		void FrameModEvent(ExtClass::XXFile* xxFile);
		void FrameModRoom(ExtClass::XXFile* xxFile);

		std::wstring GetOverride(const std::wstring& file);
		void SetOverride(const std::wstring& file, const std::wstring& override);

		bool m_isActive;
		std::vector<PoserCharacter*> m_characters;
		PoserCharacter* m_loadCharacter;
		PoserCharacter* m_currentCharacter;
		std::map<std::wstring, std::wstring> m_overrides;
		
#define LUA_CLASS Poser::PoserController
		static inline void bindLua() {
			LUA_NAME;
		};
#undef LUA_CLASS

	};
}
