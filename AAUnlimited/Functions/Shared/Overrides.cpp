#include "Overrides.h"

#include <Windows.h>

#include "Files\Logger.h"
#include "Files\Config.h"
#include "General\Util.h"
#include "General\ModuleInfo.h"
#include "Functions\CharInstData.h"
#include "Functions\AAEdit\Globals.h"
#include "Functions\AAPlay\Globals.h"
#include "Functions\TextureImage.h"
#include "Functions\AAUCardData.h"
#include "Functions\Shared\Globals.h"
#include "Functions\Shared\Slider.h"
#include "External\ExternalClasses\CharacterStruct.h"

namespace Shared {


	CharInstData* g_currentChar = &AAEdit::g_currChar;
	bool g_isOverriding = false;

	/********************/
	/* Poly Load Events */
	/********************/
	//note that these events are only fireing in AAPlay, so the current card always stays the global
	//AAEdit::g_cardData if we are in AAEdit

	void MeshTextureCharLoadStart(ExtClass::CharacterStruct* loadCharacter) {
		g_currentChar = &AAPlay::g_characters[loadCharacter->m_seat];
		g_isOverriding = true;
	}

	void MeshTextureCharLoadEnd() {
		g_isOverriding = false;
	}

	/*********************************/
	/* General Archive File Redirect */
	/*********************************/

	namespace {
		TCHAR loc_archiveBuffer[256];
		TCHAR loc_fileBuffer[256];
	}

	bool ArchiveReplaceRules(wchar_t** archive, wchar_t** file, DWORD* readBytes, BYTE** outBuffer) {
		TCHAR* strArchive = General::FindFileInPath(*archive);
		auto match = g_currentChar->m_cardData.GetArchiveRedirectFile(strArchive, *file);
		if (match != NULL) {
			size_t size = (strArchive - *archive); //note that the /2 is done automatically
			wcsncpy_s(loc_archiveBuffer, *archive, size);
			wcscpy_s(loc_archiveBuffer + size, 256 - size, match->first.c_str());
			wcscpy_s(loc_fileBuffer, match->second.c_str());
			*archive = loc_archiveBuffer;
			*file = loc_fileBuffer;
		}
		return false;
	}

	/*********************************/
	/* General Archive File Override */
	/*********************************/

	bool ArchiveOverrideRules(wchar_t* archive, wchar_t* file, DWORD* readBytes, BYTE** outBuffer) {
		TCHAR* strArchive = General::FindFileInPath(archive);

		const OverrideFile* match = g_currentChar->m_cardData.GetArchiveOverrideFile(strArchive, file);
		if (match == NULL) return false;

		void* fileBuffer = Shared::IllusionMemAlloc(match->GetFileSize());
		match->WriteToBuffer((BYTE*)fileBuffer);
		*outBuffer = (BYTE*)fileBuffer;
		*readBytes = match->GetFileSize();
		return true;
	}

	/*************************/
	/* Eye Texture Overrides */
	/*************************/

	namespace {
		TCHAR* loc_savedPointer = NULL;
		BYTE loc_replaceBuffer[1024];
		bool loc_dumpTexture = false;
		const std::vector<BYTE>* loc_textureBuffer;
	};

	void __stdcall EyeTextureStart(int leftRight, TCHAR** texture) {
		const std::wstring& eyeTexture = g_currentChar->m_cardData.GetEyeTexture(leftRight);
		if (eyeTexture.size() > 0) {
			//if usage is 2, texture should be dumped from the buffer directly
			const std::vector<BYTE>& fileSave = g_currentChar->m_cardData.GetEyeTextureBuffer(leftRight);
			if (fileSave.size() > 0 && g_Config.GetKeyValue(Config::SAVED_EYE_TEXTURE_USAGE).iVal == 2) {
				loc_dumpTexture = true;
				loc_textureBuffer = &fileSave;
			}
			else {
				loc_savedPointer = *texture;
				//*texture = (TCHAR*)eyeTexture.c_str();
				memcpy_s((void*)loc_replaceBuffer, 1024, (BYTE*)(*texture) - 16, 16);
				std::wstring fullPath = General::BuildEditPath(TEXT("data\\texture\\eye\\"), eyeTexture.c_str());
				wcscpy_s((TCHAR*)(loc_replaceBuffer + 16), 512 - 16 / 2, fullPath.c_str());
				*texture = (TCHAR*)(loc_replaceBuffer + 16);
			}
			
		}
	}

	BYTE* EyeTextureDump(wchar_t* fileName, DWORD* readBytes) {
		if (loc_dumpTexture) {
			loc_dumpTexture = false;
			void* mem = IllusionMemAlloc(loc_textureBuffer->size());
			memcpy_s(mem, loc_textureBuffer->size(), loc_textureBuffer->data(), loc_textureBuffer->size());
			*readBytes = loc_textureBuffer->size();
			return (BYTE*)mem;
		}
		return FALSE;
	}

	void __stdcall EyeTextureEnd(int leftRight, TCHAR** texture) {
		if (loc_savedPointer != NULL) {
			*texture = loc_savedPointer;
			loc_savedPointer = NULL;
		}
		loc_dumpTexture = false;
	}

	/**************************/
	/* Mesh-Texture overrides */
	/**************************/
	//for more information, look at the MemMods/AAEdit/MeshTexture.cpp

	const TextureImage* MeshTextureOverrideRules(wchar_t* fileName) {
		return g_currentChar->m_cardData.GetMeshOverrideTexture(fileName);
	}

	void XXFileModification(ExtClass::XXFile* xxFile, bool backup) {
		
		static struct Backups {
			ExtClass::XXFile* oldFile;
			std::map<void*,D3DMATRIX> matrixMap;
			std::map<void*,std::vector<ExtClass::Keyframe>> keyframeMap;
		} backups[ExtClass::CharacterStruct::N_MODELS];
		Backups* thisBackup = NULL;

		//find xx file in bone rules
		size_t written;
		TCHAR name[256];
		mbstowcs_s(&written,name,xxFile->m_name,256);
		const auto* rmatch = Shared::g_currentChar->m_cardData.GetBoneRule(name);

		//find model type of xx file and slider rule if existant
		const std::map<std::wstring,std::vector<std::pair<const Shared::Slider*, AAUCardData::BoneMod>>>* smatch = NULL;
		if (Shared::g_currentChar->m_char != NULL) {
			for (int i = 0; i < ExtClass::CharacterStruct::N_MODELS; i++) {
				if (xxFile == Shared::g_currentChar->m_char->GetXXFile((ExtClass::CharacterStruct::Models)i)) {
					smatch = &Shared::g_currentChar->m_cardData.GetSliderRuleMap(i);
					thisBackup = &backups[i];
					if (backup && xxFile != backups[i].oldFile) {
						backups[i].oldFile = xxFile;
						backups[i].keyframeMap.clear();
						backups[i].matrixMap.clear();
					}
					break;
				}
			}
		}
		
		if (rmatch == NULL && smatch == NULL) return;

		//adjust bone matrizes
		xxFile->EnumBonesPreOrder([&](ExtClass::Bone* bone) {
			mbstowcs_s(&written,name,bone->m_name,256);

			//try to find matches in both the bone rules and slider rules
			bool match = false;
			std::map<std::wstring,std::vector<AAUCardData::BoneMod>>::const_iterator mit;
			std::map<std::wstring,std::vector<std::pair<const Shared::Slider*,AAUCardData::BoneMod>>>::const_iterator sit;
			
			if (rmatch) {
				mit = rmatch->find(name);
				if (mit != rmatch->end()) match = true;
			}
			if(smatch) {
				sit = smatch->find(name);
				if (sit != smatch->end()) {
					match = true;
					//load or save backup if requested
					if(thisBackup) {
						auto it = thisBackup->matrixMap.find(bone);
						if(it != thisBackup->matrixMap.end()) {
							//allready exists, this means its the second time, so we restore
							bone->m_matrix1 = it->second;
							bone->m_matrix5 = it->second;
						}
						else {
							//save backups
							thisBackup->matrixMap.insert(std::make_pair(bone,bone->m_matrix5));
						}
					}
				}
			}
			
			if (match) {
				//decompose current matrix
				D3DVECTOR3 scales;
				D3DQUATERNION rot;
				D3DVECTOR3 trans;
				D3DVECTOR3 vecRot = {0,0,0};
				(*Shared::D3DXMatrixDecompose)(&scales,&rot,&trans,&bone->m_matrix5);
				//add our values
				if(rmatch && mit != rmatch->end()) {
					for (auto mod : mit->second) {
						Shared::Slider::ModifySRT(&scales,&vecRot,&trans,Shared::Slider::ADD,mod);
					}
				}
				if(smatch && sit != smatch->end()) {
					for (auto mod : sit->second) {
						Shared::Slider::ModifySRT(&scales,&vecRot,&trans,mod.first->op,mod.second);
					}
				}

				D3DQUATERNION modRot;
				(*Shared::D3DXQuaternionRotationYawPitchRoll)(&modRot,vecRot.y,vecRot.x,vecRot.z);
				D3DQUATERNION combinedRot;
				(*Shared::D3DXQuaternionMultiply)(&combinedRot,&rot,&modRot);
				//recompose matrix
				D3DMATRIX mscale = {
					scales.x,0,0,0,
					0,scales.y,0,0,
					0,0,scales.z,0,
					0,0,0,1.0f
				};
				D3DMATRIX mtrans = {
					1,0,0,0,
					0,1,0,0,
					0,0,1,0,
					trans.x,trans.y,trans.z,1.0f
				};
				D3DMATRIX mrot;
				(*Shared::D3DXMatrixRotationQuaternion)(&mrot,&combinedRot);

				D3DMATRIX combined;
				(*Shared::D3DXMatrixMultiply)(&combined,&mscale,&mrot);
				(*Shared::D3DXMatrixMultiply)(&combined,&combined,&mtrans);

				//apply new matrix
				bone->m_matrix1 = combined;
				bone->m_matrix5 = combined;
			}
		});

		//adjust animations
		for (DWORD i = 0; i < xxFile->m_animArraySize; i++) {
			mbstowcs_s(&written,name,xxFile->m_animArray[i].m_name,256);
			bool match = false;
			std::map<std::wstring,std::vector<AAUCardData::BoneMod>>::const_iterator mit;
			std::map<std::wstring,std::vector<std::pair<const Shared::Slider*,AAUCardData::BoneMod>>>::const_iterator sit;

			if (rmatch) {
				mit = rmatch->find(name);
				if (mit != rmatch->end()) match = true;
			}
			if (smatch) {
				sit = smatch->find(name);
				if (sit != smatch->end()) {
					match = true;
					//load or save backup if requested
					if (thisBackup) {
						auto it = thisBackup->keyframeMap.find(&xxFile->m_animArray[i]);
						if (it != thisBackup->keyframeMap.end()) {
							//allready exists, this means its the second time, so we restore
							for(DWORD j = 0; j < it->second.size(); j++) {
								xxFile->m_animArray[i].m_frameArray[j] = it->second[j];
							}
						}
						else {
							//save backups
							std::vector<ExtClass::Keyframe> vec;
							for (DWORD j = 0; j < xxFile->m_animArray[i].m_nFrames; j++) {
								vec.push_back(xxFile->m_animArray[i].m_frameArray[j]);
							}
							thisBackup->keyframeMap.insert(make_pair(&xxFile->m_animArray[i],vec));
						}
					}
				}
			}
			if (match) {
				for (DWORD j = 0; j < xxFile->m_animArray[i].m_nFrames; j++) {
					ExtClass::Keyframe& frame = xxFile->m_animArray[i].m_frameArray[j];
					if (rmatch && mit != rmatch->end()) {
						for (auto elem : mit->second) {
							Shared::Slider::ModifyKeyframe(&frame,Shared::Slider::ADD,elem);
						}
					}
					if (smatch && sit != smatch->end()) {
						for (auto elem : sit->second) {
							Shared::Slider::ModifyKeyframe(&frame,elem.first->op,elem.second);
						}
					}
				}
			}
		}
	}



}