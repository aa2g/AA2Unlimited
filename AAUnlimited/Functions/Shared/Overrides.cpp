#include "Overrides.h"

#include <Windows.h>
#include <algorithm>

#include "Files\Logger.h"
#include "Files\Config.h"
#include "General\Util.h"
#include "General\IllusionUtil.h"
#include "General\ModuleInfo.h"
#include "Functions\CharInstData.h"
#include "Functions\AAEdit\Globals.h"
#include "Functions\AAPlay\Globals.h"
#include "Functions\TextureImage.h"
#include "Functions\AAUCardData.h"
#include "Functions\Shared\Globals.h"
#include "Functions\Shared\Slider.h"
#include "External\ExternalClasses\CharacterStruct.h"
#include "MemMods\AAPlay\Events\ClothingDialog.h"
#include "Functions\AAPlay\GameState.h"

namespace Shared {


	CharInstData* g_currentChar = &AAEdit::g_currChar;

	/********************/
	/* Poly Load Events */
	/********************/
	//note that these events are only fireing in AAPlay, so the current card always stays the global
	//AAEdit::g_cardData if we are in AAEdit

	void MeshTextureCharLoadStart(ExtClass::CharacterStruct* loadCharacter) {
		//the preview always has seat 0. this means we can identify a preview character
		//by checking its seat number and then comparing with the character currently in seat 0
		if(loadCharacter->m_seat == 0 && AAPlay::g_characters[0].m_char != loadCharacter) {
			AAPlay::SetPreviewChar(loadCharacter);
			g_currentChar = &AAPlay::g_previewChar;
		}
		else {
			g_currentChar = &AAPlay::g_characters[loadCharacter->m_seat];
		}

		Shared::GameState::setIsHighPolyLoaded(false);	//not yet loaded
		Shared::GameState::setIsOverriding(true);
	}

	void MeshTextureCharLoadEnd() {
		Shared::GameState::setIsHighPolyLoaded(true);
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

	void OverrideTanColor(DWORD* tanColor,DWORD* unknown) {
		if (!Shared::GameState::getIsOverriding()) return;
		if (!g_currentChar->m_cardData.HasTanColor()) return;
		COLORREF color = Shared::g_currentChar->m_cardData.GetTanColor();
		//colors are sequentially in rgba order in *colors
		*tanColor = color | 0xFF000000;
	}

	/**************************/
	/* Mesh-Texture overrides */
	/**************************/
	//for more information, look at the MemMods/AAEdit/MeshTexture.cpp

	const TextureImage* MeshTextureOverrideRules(wchar_t* fileName) {
		return g_currentChar->m_cardData.GetMeshOverrideTexture(fileName);
	}

	/*********************/
	/* Object  overrides */
	/*********************/

	const XXObjectFile * ObjectOverrideRules(char * fileName) {
		return g_currentChar->m_cardData.GetObjectOverrideFile(fileName);
	}

	/**********************/
	/* body modifications */
	/**********************/

	std::vector<std::pair<ExtClass::Frame*,D3DMATRIX>> g_xxMods[ExtClass::CharacterStruct::N_MODELS];
	std::vector<Loc_BoneSaveDataV2> g_xxBoneParents[ExtClass::CharacterStruct::N_MODELS];
	void XXFileModification(ExtClass::XXFile* xxFile, bool saveMods) {
		using namespace ExtClass;
		if (xxFile == NULL) return;
		if (!Shared::GameState::getIsOverriding()) return;

		static const char prefix[] {"artf_"};

		//first do the frame rules, then the bone rules

		//find xx file in bone rules
		size_t written;
		TCHAR name[256];
		mbstowcs_s(&written,name,xxFile->m_name,256);
		const auto* rmatch = Shared::g_currentChar->m_cardData.GetFrameRule(name);

		//find model type of xx file and slider rule if existant
		const std::map<std::wstring,std::vector<std::pair<const Shared::Slider*,AAUCardData::BoneMod>>>* smatch = NULL;

		ExtClass::CharacterStruct::Models model;
		model = General::GetModelFromName(xxFile->m_name);
		if(model <= ExtClass::CharacterStruct::N_MODELS) {
			smatch = &Shared::g_currentChar->m_cardData.GetSliderFrameRuleMap(model);
			if(saveMods) {
				g_xxMods[model].clear();
				g_xxBoneParents[model].clear();
			}
		}
		
		

		if (rmatch != NULL || smatch != NULL) {
			//adjust bone matrizes
			xxFile->EnumBonesPostOrder([&](ExtClass::Frame* bone) {
				mbstowcs_s(&written,name,bone->m_name,256);

				//try to find matches in both the bone rules and slider rules
				bool match = false;
				bool makeSave = false;
				std::map<std::wstring,std::vector<AAUCardData::BoneMod>>::const_iterator mit;
				std::map<std::wstring,std::vector<std::pair<const Shared::Slider*,AAUCardData::BoneMod>>>::const_iterator sit;

				if (rmatch) {
					mit = rmatch->find(name);
					if (mit != rmatch->end()) match = true;
				}
				if (saveMods) {
					//if save mods, we have to make a bone for every slider, always
					for (const auto& elem : g_sliders[model]) {
						if (elem.boneName == name) {
							match = true;
							makeSave = true;
							break;
						}
					}
				}
				if (smatch) {
					sit = smatch->find(name);
					if (sit != smatch->end()) {
						match = true;
					}
				}

				//apply matches by adding matrizes
				if (match) {
					//make copy of the bone first
					Frame* newMatch = (Frame*)Shared::IllusionMemAlloc(sizeof(Frame));
					memcpy_s(newMatch,sizeof(Frame),bone,sizeof(Frame));

					//turn match into a copy of the root for now, since there are a lot of members i dont know
					memcpy_s(bone,sizeof(Frame),xxFile->m_root,sizeof(Frame));

					//change parent and child stuff
					bone->m_parent = newMatch->m_parent;
					bone->m_nChildren = 1;
					bone->m_children = newMatch;
					newMatch->m_parent = bone;
					for (int i = 0; i < newMatch->m_nChildren; i++) {
						newMatch->m_children[i].m_parent = newMatch;
					}

					//change name
					int namelength = newMatch->m_nameBufferSize + sizeof(prefix)-1;
					bone->m_name = (char*)Shared::IllusionMemAlloc(namelength);
					bone->m_nameBufferSize = namelength;
					strcpy_s(bone->m_name,bone->m_nameBufferSize,prefix);
					strcat_s(bone->m_name,bone->m_nameBufferSize,newMatch->m_name);


					//change matchNews matrix
					D3DMATRIX matr;
					D3DVECTOR3 scales = { 1.0f,1.0f,1.0f };
					D3DVECTOR3 trans = { 0,0,0 };
					D3DVECTOR3 vecRot = { 0,0,0 };
					//add our values
					if (rmatch && mit != rmatch->end()) {
						for (auto mod : mit->second) {
							Shared::Slider::ModifySRT(&scales,&vecRot,&trans,Shared::Slider::ADD,mod);
						}
					}
					if (makeSave) {
						std::pair<ExtClass::Frame*,D3DMATRIX> p1;
						D3DMATRIX str = { scales.x, scales.y, scales.z, 0, vecRot.x, vecRot.y, vecRot.z, 0, trans.x, trans.y, trans.z, 0 };
						p1 = std::make_pair(bone,std::move(str));
						g_xxMods[model].push_back(std::move(p1));
					}
					if (smatch && sit != smatch->end()) {
						for (auto mod : sit->second) {
							Shared::Slider::ModifySRT(&scales,&vecRot,&trans,mod.first->op,mod.second);
						}
					}
					matr = General::MatrixFromSRT(scales,vecRot,trans);
					bone->m_matrix1 = matr;
					bone->m_matrix5 = matr;
				}
			});
		}

		

		//now, frames that represent a mesh have a bunch of bones; each bone has a pointer to its frame (more precisely,
		//its frames matrix2), which it uses to position its mesh. after this, those pointers will point to the artificial matrizes,
		//so we have to change that as well
		xxFile->EnumBonesPostOrder([&](ExtClass::Frame* frame) {
			for(int i = 0; i < frame->m_nBones; i++) {
				Bone* bone = &frame->m_bones[i]; 
				Frame* boneFrame = bone->GetFrame();
				if (boneFrame != NULL && strncmp(boneFrame->m_name,prefix, sizeof(prefix)-1) == 0) {
					bone->SetFrame(&boneFrame->m_children[0]);
				}
			}
		});

		//lastly, do the bone stuff.
		//find xx file in bone rules
		{
			mbstowcs_s(&written,name,xxFile->m_name,256);
			const auto* rmatch = Shared::g_currentChar->m_cardData.GetBoneRule(name);

			//find model type of xx file and slider rule if existant
			const std::map<std::wstring,std::vector<std::pair<const Shared::Slider*,AAUCardData::BoneMod>>>* smatch = NULL;

			ExtClass::CharacterStruct::Models model;
			model = General::GetModelFromName(xxFile->m_name);
			if (model <= ExtClass::CharacterStruct::N_MODELS) {
				smatch = &Shared::g_currentChar->m_cardData.GetSliderBoneRuleMap(model);
			}

			if (rmatch == NULL && smatch == NULL) return;

			xxFile->EnumBonesPostOrder([&](ExtClass::Frame* frame) {
				if (frame->m_nBones == 0) return;
				std::wstring frameName;
				mbstowcs_s(&written,name,frame->m_name,256);
				frameName = name;
				for (int i = 0; i < frame->m_nBones; i++) {
					Bone* bone = &frame->m_bones[i];
					mbstowcs_s(&written,name,bone->m_name,256);
					
					//try to find matches in both the bone rules and slider rules
					bool match = false;
					bool makeSave = false;
					std::map<std::wstring,std::vector<AAUCardData::BoneMod>>::const_iterator mit;
					std::map<std::wstring,std::vector<std::pair<const Shared::Slider*,AAUCardData::BoneMod>>>::const_iterator sit;

					if (rmatch) {
						mit = rmatch->find(name);
						if (mit != rmatch->end()) match = true;
					}
					if (saveMods) {
						//if save mods, we have to make a bone for every slider, always
						for (const auto& elem : g_sliders[model]) {
							if (elem.boneName == name) {
								match = true;
								makeSave = true;
								break;
							}
						}
					}
					if (smatch) {
						sit = smatch->find(name);
						if (sit != smatch->end()) {
							match = true;
						}
					}

					//apply matches by adding matrizes
					if (match) {
						//change matchNews matrix
						D3DMATRIX matr;
						D3DVECTOR3 scales = { 1.0f,1.0f,1.0f };
						D3DVECTOR3 trans = { 0,0,0 };
						D3DVECTOR3 vecRot = { 0,0,0 };
						//add our values
						if (rmatch && mit != rmatch->end()) {
							for (auto mod : mit->second) {
								Shared::Slider::ModifySRT(&scales,&vecRot,&trans,Shared::Slider::ADD,mod);
							}
						}
						if (makeSave) {
							bool added = false;
							for(auto& elem : g_xxBoneParents[model]) {
								if(elem.boneName == name) {
									added = true;
									elem.parents.push_back(frame);
									break;
								}
							}
							if(!added) {
								Loc_BoneSaveDataV2 data;
								D3DMATRIX str = { scales.x, scales.y, scales.z, 0, vecRot.x, vecRot.y, vecRot.z, 0, trans.x, trans.y, trans.z, 0 };
								data.srtMatrix = str;
								data.origMatrix = bone->m_matrix;
								data.boneName = name;
								data.parents.push_back(frame);
								g_xxBoneParents[model].push_back(data);
							}
						}
						if (smatch && sit != smatch->end()) {
							for (auto mod : sit->second) {
								Shared::Slider::ModifySRT(&scales,&vecRot,&trans,mod.first->op,mod.second);
							}
						}
						matr = General::MatrixFromSRT(scales,vecRot,trans);
						(*Shared::D3DXMatrixMultiply)(&bone->m_matrix,&matr,&bone->m_matrix);
					}
				}
			});
		}
		
	}


	std::vector<Loc_BoneSaveData> g_xxBoneMods[ExtClass::CharacterStruct::N_MODELS];
	void XXBoneModification(ExtClass::Frame* boneParent,bool saveMods) {
		using namespace ExtClass;
		if (!Shared::GameState::getIsOverriding()) return;

		static const char prefix[]{ "artf_" };
		ExtClass::XXFile* xxFile = boneParent->m_xxPartOf;


		//first do the frame rules, then the bone rules

		//find xx file in bone rules
		size_t written;
		TCHAR name[256];
		mbstowcs_s(&written,name,xxFile->m_name,256);
		const auto* rmatch = Shared::g_currentChar->m_cardData.GetBoneRule(name);

		//find model type of xx file and slider rule if existant
		const std::map<std::wstring,std::vector<std::pair<const Shared::Slider*,AAUCardData::BoneMod>>>* smatch = NULL;

		ExtClass::CharacterStruct::Models model;
		model = General::GetModelFromName(xxFile->m_name);
		if (model <= ExtClass::CharacterStruct::N_MODELS) {
			smatch = &Shared::g_currentChar->m_cardData.GetSliderBoneRuleMap(model);
		}

		if (rmatch == NULL && smatch == NULL) return;

		ExtClass::Frame* frame = boneParent;
		for (int i = 0; i < frame->m_nBones; i++) {
			Bone* bone = &frame->m_bones[i];
			std::wstring frameName;
			mbstowcs_s(&written,name,frame->m_name,256);
			frameName = name;

			mbstowcs_s(&written,name,bone->m_name,256);
			

			//try to find matches in both the bone rules and slider rules
			bool match = false;
			bool makeSave = false;
			std::map<std::wstring,std::vector<AAUCardData::BoneMod>>::const_iterator mit;
			std::map<std::wstring,std::vector<std::pair<const Shared::Slider*,AAUCardData::BoneMod>>>::const_iterator sit;

			if (rmatch) {
				mit = rmatch->find(name);
				if (mit != rmatch->end()) match = true;
			}
			if (saveMods) {
				//if save mods, we have to make a bone for every slider, always
				for (const auto& elem : g_sliders[model]) {
					if (elem.boneName == name) {
						match = true;
						makeSave = true;
						break;
					}
				}
			}
			if (smatch) {
				sit = smatch->find(name);
				if (sit != smatch->end()) {
					match = true;
				}
			}

			//apply matches by adding matrizes
			if (match) {
				//change matchNews matrix
				D3DMATRIX matr;
				D3DVECTOR3 scales = { 1.0f,1.0f,1.0f };
				D3DVECTOR3 trans = { 0,0,0 };
				D3DVECTOR3 vecRot = { 0,0,0 };
				//add our values
				if (rmatch && mit != rmatch->end()) {
					for (auto mod : mit->second) {
						Shared::Slider::ModifySRT(&scales,&vecRot,&trans,Shared::Slider::ADD,mod);
					}
				}
				if (makeSave) {
					//save this bone in the global struct for the gui to access
					D3DMATRIX str = { scales.x, scales.y, scales.z, 0, vecRot.x, vecRot.y, vecRot.z, 0, trans.x, trans.y, trans.z, 0 };
					//search for the bone first
					int foundIndex;
					for (foundIndex = 0; foundIndex < g_xxBoneMods[model].size(); foundIndex++) {
						auto& elem = g_xxBoneMods[model][foundIndex];
						if(elem.name == name) {
							//bone name known; search for this bone specifically
							bool boneReplaced = false;
							for(auto savedBone : elem.bones) {
								if(savedBone.meshParentName == frameName && savedBone.parent != boneParent) {
									//found this bone specifically. that means we reread this mesh
									/*for(int i = 0; i < g_xxBoneMods[model].size(); i++) {
										auto& boneVec = g_xxBoneMods[model][i].bones;
										for(int j = 0; j < boneVec.size(); j++) {
											auto& bone = boneVec[j];
											if(bone.meshParentName == frameName) {
												boneVec.erase(boneVec.begin() + j);
												j--;
											}
										}
										if(boneVec.size() == 0) {
											g_xxBoneMods[model].erase(g_xxBoneMods[model].begin() + i);
											i--;
											foundIndex = -1;
										}
									}*/
									foundIndex = 0;
									g_xxBoneMods[model].clear();
									break;
								}
							}
							break;
						}
					}
					if(foundIndex == g_xxBoneMods[model].size() || foundIndex < 0) {
						Loc_BoneSaveData data;
						data.name = name;
						data.origMatrix = bone->m_matrix;
						data.srtMatrix = str;
						data.bones.push_back({ frameName, frame, bone });
						g_xxBoneMods[model].push_back(data);
					}
					else {
						g_xxBoneMods[model][foundIndex].bones.push_back({ frameName, frame, bone });
					}
					
					
				}
				if (smatch && sit != smatch->end()) {
					for (auto mod : sit->second) {
						Shared::Slider::ModifySRT(&scales,&vecRot,&trans,mod.first->op,mod.second);
					}
				}
				matr = General::MatrixFromSRT(scales,vecRot,trans);
				(*Shared::D3DXMatrixMultiply)(&bone->m_matrix,&matr,&bone->m_matrix);
			}
		}
	}



	void XXFileModification_BackupCopy(ExtClass::XXFile* xxFile,bool backup) {
		if (!Shared::GameState::getIsOverriding()) return;
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
		const std::map<std::wstring,std::vector<std::pair<const Shared::Slider*,AAUCardData::BoneMod>>>* smatch = NULL;
		if (Shared::g_currentChar->m_char != NULL) {
			for (int i = 0; i < ExtClass::CharacterStruct::N_MODELS; i++) {
				if (xxFile == Shared::g_currentChar->m_char->GetXXFile((ExtClass::CharacterStruct::Models)i)) {
					smatch = &Shared::g_currentChar->m_cardData.GetSliderFrameRuleMap(i);
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
		xxFile->EnumBonesPreOrder([&](ExtClass::Frame* bone) {
			mbstowcs_s(&written,name,bone->m_name,256);

			//try to find matches in both the bone rules and slider rules
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
						auto it = thisBackup->matrixMap.find(bone);
						if (it != thisBackup->matrixMap.end()) {
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
				D3DVECTOR3 vecRot = { 0,0,0 };
				(*Shared::D3DXMatrixDecompose)(&scales,&rot,&trans,&bone->m_matrix5);
				//add our values
				if (rmatch && mit != rmatch->end()) {
					for (auto mod : mit->second) {
						Shared::Slider::ModifySRT(&scales,&vecRot,&trans,Shared::Slider::ADD,mod);
					}
				}
				if (smatch && sit != smatch->end()) {
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
							for (DWORD j = 0; j < it->second.size(); j++) {
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