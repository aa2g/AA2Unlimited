#pragma once

// The default debug iterator level has corruption issues with this Lua bridge
//
// Achkctually, NEVER, EVER do this. The debug level changes the layout of STL
// structure which will now mismatch between compile units you didnt cherrypick
// define it in like this. Instead, add this as global define to the build target.

/*
#ifdef _DEBUG
#undef _ITERATOR_DEBUG_LEVEL
#define _ITERATOR_DEBUG_LEVEL 1
#endif
*/

#include <Windows.h>
#include <map>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "../../External/ExternalClasses/CharacterStruct.h"
#include "../../External/ExternalClasses/Frame.h"
#include "../../External/ExternalClasses/XXFileFace.h"
#include "../../General/DirectXStructs.h"
#include "../Shared/Globals.h"
#include "../../3rdparty/picojson/picojson.h"
#include "Script/glua_stl.h"
#include <DirectXMath.h>

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
				frame = nullptr;
				Reset();
				sliding = false;
				source = ExtClass::CharacterStruct::INVALID;
				currentOperation = Rotate;
			}

			void Apply();

			ExtClass::CharacterStruct::Models source;
			ExtClass::Frame* frame;
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

			void setValue(int axis, float value);
			void setValue(float newValueX, float newValueY, float newValueZ);

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
				LUA_METHOD(Reset, {
					_self->Reset();
				});
				LUA_METHOD(SetCurrentOperation, {
					_self->setCurrentOperation((Operation)(int)_gl.get(2));
				});
				LUA_METHOD(Increment, {
					_self->increment(_gl.get(2), _gl.get(3));
					_self->Apply();
				});
				LUA_METHOD(Values, {
					if (_self->currentOperation == Rotate) {
						float angles[3];
						_self->rotation.getEulerAngles(angles);
						_gl.push(angles[0]);
						_gl.push(angles[1]);
						_gl.push(angles[2]);
					}
					else if (_self->currentOperation == Translate) {
						_gl.push(_self->translate.value[0]);
						_gl.push(_self->translate.value[1]);
						_gl.push(_self->translate.value[2]);
}
					else {
						_gl.push(_self->scale.value[0]);
						_gl.push(_self->scale.value[1]);
						_gl.push(_self->scale.value[2]);
					}
					return 3;
				});
				LUA_METHOD(SetValues, {
					_self->setValue(_gl.get(2), _gl.get(3), _gl.get(4));
					return 0;
				});
				LUA_METHOD(StartSlide, {
					_self->startSlide();
				});
				LUA_METHOD(StopSlide, {
					_self->stopSlide();
				});
				LUA_METHOD(eulerangle, {
					float angles[3];
					_self->rotation.getEulerAngles(angles);
					auto idx = _gl.get(2);
					_gl.push(angles[idx]);
					return 1;
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
				GLUA_BIND(LUA_GLOBAL, ACCESOR, LUA_CLASS, frame, {
					_gl.push(_self->frame->m_children);
					return 1;
				});
			}
#undef LUA_CLASS
		}; //SliderInfo

		class PoserCharacter {
		public:

			PoserCharacter();
			~PoserCharacter();

			void Clear();

			inline void ResetSliders() {
				for (auto it = m_sliders.begin(), end = m_sliders.end(); it != end; it++) {
					it->second->Reset();
					it->second->Apply();
				}
			}

			void FrameModTree(ExtClass::Frame* tree, ExtClass::CharacterStruct::Models source, const char* filter = nullptr);
			void FrameModSkeleton(ExtClass::XXFile* xxFile);
			void FrameModFace(ExtClass::XXFile* xxFile);
			void FrameModSkirt(ExtClass::XXFile* xxFile);
			SliderInfo* GetSlider(const char* name);
			SliderInfo* GetSlider(const std::string& name);
			SliderInfo* GetPropSlider(const char* name);
			void SetHidden(const char* name, bool hidden);

			void LoadCloth(const char *file);

			void quat2euler(D3DXQUATERNION q, FLOAT* out);
			D3DXQUATERNION euler2quat(FLOAT* angle);

			void QuatSliderSlerp(SliderInfo* slider, D3DXQUATERNION rotQ, float value);
			D3DXQUATERNION QuatSlerp(D3DXQUATERNION q1, D3DXQUATERNION q2, float value);

			ExtClass::CharacterStruct* m_character;
			std::unordered_map<std::string, SliderInfo*> m_sliders;
			std::unordered_map<std::string, SliderInfo*> m_transientSliders;
			std::unordered_map<std::string, SliderInfo*> m_propSliders;
			std::unordered_map<std::wstring, std::wstring> m_overrides;

#define LUA_CLASS PoserController::PoserCharacter
			static inline void bindLua() {
				LUA_NAME;
				LUA_MGETTER1(GetSlider);
				LUA_MAPITERATOR(Sliders, m_sliders);
				LUA_MAPITERATOR(Props, m_propSliders);
				LUA_METHOD(Override, {
					if (_gl.top() == 3) {
						_self->m_overrides.erase(General::utf8.from_bytes((const char*)_gl.get(2)));
						LOGSPAM << "Poser: Unregistered PoserCharacter Override\n";
						if (!_gl.get(3).isnil()) {
							_self->m_overrides.emplace(General::utf8.from_bytes((const char*)_gl.get(2)), General::utf8.from_bytes((const char*)_gl.get(3)));
							LOGSPAM << "Poser: Registered PoserCharacter Override\n";
						}
					}
					else if (_gl.top() == 2) {
						LOGSPAM << "Poser: Queried PoserCharacter Override\n";
						auto match = _self->m_overrides.find(General::utf8.from_bytes((const char*)_gl.get(2)));
						if (match != _self->m_overrides.end()) {
							_gl.push(General::utf8.to_bytes(match->first).c_str());
							return 1;
						}
					}
					return 0;
				});
				LUA_METHOD(QuatSliderSlerp, {
					auto a1 = _gl.get(2); // slider
					auto a2 = _gl.get(3); // rotQ
					D3DXQUATERNION q2;
					q2.x = a2[1];
					q2.y = a2[2];
					q2.z = a2[3];
					q2.w = a2[4];
					auto val = _gl.get(4);// value
					_self->QuatSliderSlerp(a1, q2, val);					
				});
				LUA_METHOD(QuatSlerp, {
					auto a1 = _gl.get(2); //
					D3DXQUATERNION q1;
					q1.x = a1[1];
					q1.y = a1[2];
					q1.z = a1[3];
					q1.w = a1[4];
					auto a2 = _gl.get(3); // rotQ
					D3DXQUATERNION q2;
					q2.x = a2[1];
					q2.y = a2[2];
					q2.z = a2[3];
					q2.w = a2[4];
					auto val = _gl.get(4);// value
					auto out = _self->QuatSlerp(q1, q2, val);
					_gl.push(out.x).push(out.y).push(out.z).push(out.w);
					return 4;
				});
				LUA_METHOD(quat2euler, {
					auto q1 = _gl.get(2);
					float angles[3];
					D3DXQUATERNION q;
					q.x = q1[1];
					q.y = q1[2];
					q.z = q1[3];
					q.w = q1[4];
					_self->quat2euler(q, angles);
					_gl.push(angles[0]).push(angles[1]).push(angles[2]);
					return 3;
				});
				LUA_METHOD(euler2quat, {
					auto a = _gl.get(2);
					float angles[3];
					angles[0] = a[1];
					angles[1] = a[2];
					angles[2] = a[3];
					auto q = _self->euler2quat(angles);
					_gl.push(q.x).push(q.y).push(q.z).push(q.w);
					return 4;
				});
				LUA_METHOD(SetHidden, {
					_self->SetHidden(_gl.get(2), _gl.get(3));
				});
				LUA_METHOD(LoadCloth, {
					_self->LoadCloth(_gl.get(2));
				})
			}
#undef LUA_CLASS
		}; // PoserCharacter

		class PoserProp {
		public:
			PoserProp(ExtClass::XXFile* xxFile) : m_xxFile(xxFile) {
			}

			SliderInfo* GetSlider(const char* name);
			//SliderInfo* GetSlider(const std::string& name);

			ExtClass::XXFile* m_xxFile;
			std::unordered_map<std::string, SliderInfo*> m_sliders;
#define LUA_CLASS PoserController::PoserProp
			static inline void bindLua() {
				LUA_NAME;
				LUA_MGETTER1(GetSlider);
				LUA_MAPITERATOR(Sliders, m_sliders)
			}
#undef LUA_CLASS
		}; // PoserProp

		public:
		PoserController();

		~PoserController();

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

		void SwapTransientSliders(bool skipSkeleton);
		void FrameModEvent(ExtClass::XXFile* xxFile);
		void FrameModProp(PoserProp* xxFile);

		std::wstring GetOverride(const std::wstring& file);
		void SetOverride(const std::wstring& file, const std::wstring& override);

		bool m_isActive;
		std::vector<PoserCharacter> m_characters;
		PoserCharacter* m_loadCharacter;
		std::unordered_map<std::wstring, std::wstring> m_overrides;
	};
}
