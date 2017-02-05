#pragma once

#include <Windows.h>

#include "Functions\AAUCardData.h"
#include "Functions\CharInstData.h"
#include "Functions\XXObjectFile.h"
#include "External\ExternalClasses\CharacterStruct.h"

namespace Shared {

	/*
	* Override rules are taken from this card.
	*/
	extern CharInstData* g_currentChar;
	

	bool ArchiveReplaceRules(wchar_t** archive, wchar_t** file, DWORD* readBytes, BYTE** outBuffer);
	/*
	* Event-Callback for archive-file opening
	*/
	bool ArchiveOverrideRules(wchar_t* archive, wchar_t* file, DWORD* readBytes, BYTE** outBuffer);

	/*
	* Event-Callback for Hi-Poly-Load to set the current card to get override rules from
	*/
	void MeshTextureCharLoadStart(ExtClass::CharacterStruct* loadCharacter);
	void MeshTextureCharLoadEnd();

	void __stdcall EyeTextureStart(int leftRight, TCHAR** texture);
	BYTE* EyeTextureDump(wchar_t* fileName, DWORD* readBytes);
	void __stdcall EyeTextureEnd(int leftRight, TCHAR** texture);

	const TextureImage* MeshTextureOverrideRules(wchar_t* fileName);

	const XXObjectFile* ObjectOverrideRules(char* fileName);

	void OverrideTanColor(DWORD* tanColor,DWORD* unknown);

	/*
	 * XX file modification
	 */ 
	void XXBoneModification(ExtClass::Frame* boneParent, bool backup);
	void XXFileModification(ExtClass::XXFile* file, bool backup);
		//this data is used in aaedit only to make sliders instantly visible. the matrix is an SRT matrix
		extern std::vector<std::pair<ExtClass::Frame*,D3DMATRIX>> g_xxMods[ExtClass::CharacterStruct::N_MODELS];
		//the bone variant needs the name as well as they might become invalid without an event to notice that.
		//there are also multiple bones of the same name
		//also, we need to save the original matrix since its not always id
		struct Loc_BoneSaveData{
			std::wstring name;
			struct MeshBone { //every mesh has its own copy of the bone
				std::wstring meshParentName;
				ExtClass::Frame* parent;
				ExtClass::Bone* ptr;
			};
			std::vector<MeshBone> bones;
			D3DMATRIX origMatrix;
			D3DMATRIX srtMatrix;
		};
		struct Loc_BoneSaveDataV2 {
			std::wstring boneName;
			std::vector<ExtClass::Frame*> parents;
			D3DMATRIX origMatrix;
			D3DMATRIX srtMatrix;
		};
		extern std::vector<Loc_BoneSaveData> g_xxBoneMods[ExtClass::CharacterStruct::N_MODELS];
		extern std::vector<Loc_BoneSaveDataV2> g_xxBoneParents[ExtClass::CharacterStruct::N_MODELS];

}