#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <map>
#include <d3d9.h>

#include "TextureImage.h"
#include "OverrideFile.h"
#include "XXObjectFile.h"
#include "Shared\Triggers\Module.h"
#include "External\ExternalClasses\CharacterStruct.h"
#include "Functions\Shared\Triggers\Triggers.h"

namespace Shared {
	struct Slider;
}

/*
 * Additional card data.
 * In the file, it is saved as a png chunk.
 * The chunks data represents this struct, though it only
 * saves data that is actually different from its default state.
 * Each member has a unique 4byte identifier, followed by its data.
 *
 * normal data is saved as its in memory (therefor little endian);
 * strings are saved as a 4 byte length, followed by exactly length 1-byte characters (NO null-termination)
 */
class AAUCardData
{
public:
	static const DWORD PngChunkId = 'aaUd';
	static const DWORD PngChunkIdBigEndian = 'dUaa';
	enum MeshModFlag {
		MODIFY_FRAME = 1,MODIFY_BONE = 2
	};
	static const int CurrentVersion = 2;
public:
	AAUCardData();
	~AAUCardData();

	//the part of the buffer that contains the file-chunk and has to be removed.
	//[0] is eye texture left, [1] is eye tetxture right, [2] is eye highlights, [3] are files.
	//todo: make this into a better system. this is a retarded way to handle this
	struct {
		char* fileStart;
		char* fileEnd;

		inline DWORD size() { return fileEnd - fileStart; }
	}  ret_files[4];
	char* ret_chunkSize;

	//searches for AAUnlimited data inside file, then reads it.
	bool FromFileBuffer(char* buffer, DWORD size);
	//writes data to a buffer, including png chunk. Returns size of buffer filled,
	//or 0 if it failed (because the buffer was too small and resize was false)
	int ToBuffer(char** buffer, int* size, bool resize, bool pngChunks);
	void Reset();

	//add/remove stuffs

	//overrides
	bool AddMeshOverride(const TCHAR* texture, const TCHAR* override);
	bool RemoveMeshOverride(int index);

	bool AddArchiveOverride(const TCHAR* archice, const TCHAR* archivefile, const TCHAR* override);
	bool RemoveArchiveOverride(int index);

	bool AddArchiveRedirect(const TCHAR* archive, const TCHAR* archivefile, const TCHAR* redirectarchive, const TCHAR* redirectfile);
	bool RemoveArchiveRedirect(int index);

	bool AddObjectOverride(const TCHAR* object,const TCHAR* file);
	bool RemoveObjectOverride(int index);

	//bone transformations (deprecated)
	bool AddBoneTransformation(const TCHAR* boneName,D3DMATRIX transform);
	bool RemoveBoneTransformation(int index);

	//hairs
	bool AddHair(BYTE kind,BYTE slot,BYTE adjustment,bool flip);
	bool RemoveHair(int index);

	struct BoneMod;
	bool AddBoneRule(MeshModFlag flags, const TCHAR* xxFileName,const TCHAR* boneName,BoneMod mod);
	bool RemoveBoneRule(int index);

	void SetSliderValue(int sliderTarget,int sliderIndex,float value);

	bool SetEyeTexture(int leftright, const TCHAR* texName, bool save);
	bool SetEyeHighlight(const TCHAR* texName);

	bool SetHairHighlight(const TCHAR* name);

	bool SetTan(const TCHAR* name);

	bool AddAAUDataSet(const TCHAR* name);
	bool CopyAAUDataSet(const TCHAR* name);
	bool RemoveAAUDataSet(int index);
	void SwitchActiveAAUDataSet(int newSet);

	bool AddModule(const TCHAR* moduleName);
	bool AddModule(const Shared::Triggers::Module& mod);
	bool RemoveModule(int index);

	void SaveOverrideFiles();
	bool DumpSavedOverrideFiles();

	void ConvertToNewVersion();

	void ClearOverrides();

	//rule types
	typedef std::pair<std::wstring, std::wstring> MeshOverrideRule;
	typedef std::pair<std::pair<std::wstring, std::wstring>, std::wstring> ArchiveOverrideRule;
	typedef std::pair<std::pair<std::wstring, std::wstring>, std::pair<std::wstring, std::wstring>> ArchiveRedirectRule;
	typedef std::pair<std::wstring,std::wstring> ObjectOverrideRule;
	typedef std::pair<std::wstring,D3DMATRIX> BoneRule;
	typedef std::pair<std::pair<int,std::wstring>,std::vector<BYTE>> SavedFile; //int identifying base path (aaplay = 0 or aaedit = 1)
	struct HairPart {
		BYTE kind; //0-3
		BYTE slot;
		BYTE flip;
		BYTE adjustment;
	};
	struct BoneMod {
		union {
			struct {
				float scales[3];
				float rotations[3];
				float transformations[3];
			};
			float mods[3][3];
			float data[9];
		};
		inline bool operator==(const BoneMod& rhs) {
			for (int i = 0; i < 9; i++) if (data[i] != rhs.data[i]) return false; return true;
		}
	};
	
	typedef std::pair<std::pair<int,std::pair<std::wstring,std::wstring>>,BoneMod> BoneRuleV2;
	typedef std::pair<std::pair<int,int>,float> SliderRule; //int is slider index in global array, float is selected value

	//getter functions
	BYTE GetTanSlot() const;

	std::vector<std::wstring>					GetAAUSetDataList() const;
	int											GetCurrAAUSet() const;
	//aauset data start
	const std::vector<MeshOverrideRule>&		GetMeshOverrideList() const;
	const TextureImage*							GetMeshOverrideTexture(const TCHAR* texture) const;

	const std::vector<ArchiveOverrideRule>&		GetArchiveOverrideList() const;
	const OverrideFile*							GetArchiveOverrideFile(const TCHAR* archive,const TCHAR* texture) const;

	const std::vector<ArchiveRedirectRule>&		GetArchiveRedirectList() const;
	const std::pair<std::wstring,std::wstring>* GetArchiveRedirectFile(const TCHAR* archive,const TCHAR* texture) const;

	const std::vector<ObjectOverrideRule>&		GetObjectOverrideList() const;
	const XXObjectFile*							GetObjectOverrideFile(const char* objectName) const;

	const std::wstring&							GetEyeTexture(int leftright);
	const std::vector<BYTE>&					GetEyeTextureBuffer(int leftright);

	const std::wstring							GetEyeHighlightTexture();
	const std::vector<BYTE>&					GetEyeHighlightTextureBuffer();

	const std::wstring&							GetHairHighlightName();
	const TextureImage&							GetHairHighlightTex();

	const std::wstring&							GetTanName();
	const TextureImage&							GetTanTex(int i);

	const DWORD									GetOutlineColor();
	const DWORD									SetOutlineColor(COLORREF color);
	const bool									HasOutlineColor();
	const DWORD									SetHasOutlineColor(bool has);

	const DWORD									GetTanColor();
	const DWORD									SetTanColor(COLORREF color);
	const bool									HasTanColor();
	const DWORD									SetHasTanColor(bool has);

	const std::vector<BoneRule>					GetBoneTransformationList();
	const D3DMATRIX*							GetBoneTransformationRule(const TCHAR* boneName);

	bool										HasFilesSaved();

	const std::vector<HairPart>&				GetHairs(BYTE kind);
	//aausetdata end

	std::vector<Shared::Triggers::Trigger>&			GetTriggers();
	std::vector<Shared::Triggers::GlobalVariable>&	GetGlobalVariables();
	const std::vector<Shared::Triggers::Module>&	GetModules() const;
	std::map<std::wstring,Shared::Triggers::Value>& GetCardStorage();
		
	const std::vector<BoneRuleV2>				GetMeshRuleList();
	const std::map<std::wstring,std::vector<BoneMod>>* GetBoneRule(const TCHAR* xxFileName);
	const std::map<std::wstring,std::vector<BoneMod>>* GetFrameRule(const TCHAR* xxFileName);

	const std::vector<SliderRule> GetSliderList();
	const std::map<std::wstring,std::vector<std::pair<const Shared::Slider*,BoneMod>>>& GetSliderBoneRuleMap(int type);
	const std::map<std::wstring,std::vector<std::pair<const Shared::Slider*,BoneMod>>>& GetSliderFrameRuleMap(int type);
	const std::vector<std::pair<const Shared::Slider*,BoneMod>>* GetSliderBoneRule(ExtClass::CharacterStruct::Models model,std::wstring bone);
	const std::vector<std::pair<const Shared::Slider*,BoneMod>>* GetSliderFrameRule(ExtClass::CharacterStruct::Models model,std::wstring bone);

private:
	int m_version; //saved in FIRST chunk; no chunk means version 1
	BYTE m_tanSlot;						//used tan slot, if slot is >5.


	//modifications can be saved in multiple sets.
	struct AAUDataSet {
		std::wstring m_name;

		//DATA

		std::vector<MeshOverrideRule> m_meshOverrides;	//replaces textures by other textures
		std::map<std::wstring,TextureImage> m_meshOverrideMap;	//map-representation of vector above for actual use

		std::vector<ArchiveOverrideRule> m_archiveOverrides; //<archive,file>->file
		std::map<std::pair<std::wstring,std::wstring>,OverrideFile> m_archiveOverrideMap;

		std::vector<ArchiveRedirectRule> m_archiveRedirects; //<archive,file>-><archive,file>
		std::map<std::pair<std::wstring,std::wstring>,std::pair<std::wstring,std::wstring>> m_archiveRedirectMap;

		std::vector<ObjectOverrideRule> m_objectOverrides;
		std::map<std::string,XXObjectFile> m_objectOverrideMap;

		struct {
			std::wstring texName;
			std::vector<BYTE> texFile; //contains file if it should be saved inside the card
		} m_eyeTextures[2]; //0 is the left (default), 1 is the right (the extra eye texture)

		std::wstring m_eyeHighlightName;
		std::vector<BYTE> m_eyeHighlightFile; //contains file if it should be saved inside the card

		std::wstring m_hairHighlightName; //hair highlight
		TextureImage m_hairHighlightImage;

		std::wstring m_tanName; //tan settings
		TextureImage m_tanImages[5];

		bool m_bOutlineColor;
		DWORD m_outlineColor;

		bool m_bTanColor;
		DWORD m_tanColor;

		std::vector<BoneRule> m_boneTransforms;
		std::map<std::wstring,D3DMATRIX> m_boneTransformMap;

		std::vector<HairPart> m_hairs[4];

		AAUDataSet();
	};
	std::vector<AAUDataSet> m_aauSets;
	int m_currAAUSet;

	std::vector<Shared::Triggers::GlobalVariable> m_globalVars;
	std::vector<Shared::Triggers::Trigger> m_triggers;
	std::vector<Shared::Triggers::Module> m_modules;
	std::map<std::wstring,Shared::Triggers::Value> m_cardStorage;

	std::vector<SavedFile> m_savedFiles;

	std::vector<BoneRuleV2> m_boneRules;
	std::map<std::wstring,std::map<std::wstring,std::vector<BoneMod>>> m_boneRuleMap;
	std::map<std::wstring,std::map<std::wstring,std::vector<BoneMod>>> m_frameRuleMap;

	std::vector<SliderRule> m_sliders;
	std::map<std::wstring,std::vector<std::pair<const Shared::Slider*,BoneMod>>> m_boneSliderMap[ExtClass::CharacterStruct::N_MODELS];
	std::map<std::wstring,std::vector<std::pair<const Shared::Slider*,BoneMod>>> m_frameSliderMap[ExtClass::CharacterStruct::N_MODELS];

private:
	//fills data from buffer. buffer should point to start of the png chunk (the length member)
	void FromBuffer(char* buffer, int size);

	DWORD m_currReadMemberId;	//used exclusively by FromBuffer, so that ReadData can print a precise error message
	static const AAUCardData g_defaultValues; //used to determine if a variable is not default and should be written to buffer/file

	//generate the maps from the vectors read from the file
	void GenMeshOverrideMap();
	void GenArchiveOverrideMap();
	void GenArchiveRedirectMap();
	void GenObjectOverrideMap();
	void GenBoneRuleMap();
	void GenSliderMap();

	void GenAllFileMaps();
};

inline BYTE AAUCardData::GetTanSlot() const { return m_tanSlot; }

inline int AAUCardData::GetCurrAAUSet() const {
	return m_currAAUSet;
}
inline std::vector<std::wstring> AAUCardData::GetAAUSetDataList() const {
	std::vector<std::wstring> vec(m_aauSets.size());
	for(int i = 0; i < m_aauSets.size(); i++) {
		vec[i] = m_aauSets[i].m_name;
	}
	return vec;
}

inline const std::vector<AAUCardData::MeshOverrideRule>& AAUCardData::GetMeshOverrideList() const { return m_aauSets[m_currAAUSet].m_meshOverrides; }
inline const TextureImage* AAUCardData::GetMeshOverrideTexture(const TCHAR* texture) const {
	auto it = m_aauSets[m_currAAUSet].m_meshOverrideMap.find(texture);
	return it == m_aauSets[m_currAAUSet].m_meshOverrideMap.end() ? NULL : &it->second;
}
inline const std::vector<AAUCardData::ArchiveOverrideRule>& AAUCardData::GetArchiveOverrideList() const { return m_aauSets[m_currAAUSet].m_archiveOverrides; }
inline const OverrideFile* AAUCardData::GetArchiveOverrideFile(const TCHAR* archive,const TCHAR* texture) const {
	auto it = m_aauSets[m_currAAUSet].m_archiveOverrideMap.find(std::pair<std::wstring,std::wstring>(archive,texture));
	return it == m_aauSets[m_currAAUSet].m_archiveOverrideMap.end() ? NULL : &it->second;
}
inline const std::vector<AAUCardData::ArchiveRedirectRule>& AAUCardData::GetArchiveRedirectList() const { return m_aauSets[m_currAAUSet].m_archiveRedirects; }
inline const std::pair<std::wstring,std::wstring>* AAUCardData::GetArchiveRedirectFile(const TCHAR* archive,const TCHAR* texture) const {
	auto it = m_aauSets[m_currAAUSet].m_archiveRedirectMap.find(std::pair<std::wstring,std::wstring>(archive,texture));
	return it == m_aauSets[m_currAAUSet].m_archiveRedirectMap.end() ? NULL : &it->second;
}

inline const std::vector<AAUCardData::ObjectOverrideRule>& AAUCardData::GetObjectOverrideList() const { return m_aauSets[m_currAAUSet].m_objectOverrides; }
inline const XXObjectFile* AAUCardData::GetObjectOverrideFile(const char* objectName) const {
	auto it = m_aauSets[m_currAAUSet].m_objectOverrideMap.find(objectName);
	return it == m_aauSets[m_currAAUSet].m_objectOverrideMap.end() ? NULL : &it->second;
}

inline const std::wstring& AAUCardData::GetEyeTexture(int leftright) { return m_aauSets[m_currAAUSet].m_eyeTextures[leftright].texName; }
inline const std::vector<BYTE>& AAUCardData::GetEyeTextureBuffer(int leftright) { return m_aauSets[m_currAAUSet].m_eyeTextures[leftright].texFile; }

inline const std::wstring AAUCardData::GetEyeHighlightTexture() { return m_aauSets[m_currAAUSet].m_eyeHighlightName; }
inline const std::vector<BYTE>& AAUCardData::GetEyeHighlightTextureBuffer() { return m_aauSets[m_currAAUSet].m_eyeHighlightFile; }

inline const std::wstring& AAUCardData::GetHairHighlightName() { return m_aauSets[m_currAAUSet].m_hairHighlightName; }
inline const TextureImage& AAUCardData::GetHairHighlightTex() { return m_aauSets[m_currAAUSet].m_hairHighlightImage; }

inline const std::wstring& AAUCardData::GetTanName() { return m_aauSets[m_currAAUSet].m_tanName; }
inline const TextureImage& AAUCardData::GetTanTex(int i) {
	if (i >= 0 && i < 5) return m_aauSets[m_currAAUSet].m_tanImages[i];
	return m_aauSets[m_currAAUSet].m_tanImages[0];
}

inline const DWORD AAUCardData::GetOutlineColor() { return m_aauSets[m_currAAUSet].m_outlineColor; }
inline const DWORD AAUCardData::SetOutlineColor(COLORREF color) { return m_aauSets[m_currAAUSet].m_outlineColor = color; }
inline const bool AAUCardData::HasOutlineColor() { return m_aauSets[m_currAAUSet].m_bOutlineColor; }
inline const DWORD AAUCardData::SetHasOutlineColor(bool has) { return m_aauSets[m_currAAUSet].m_bOutlineColor = has; }

inline const DWORD AAUCardData::GetTanColor() { return m_aauSets[m_currAAUSet].m_tanColor; }
inline const DWORD AAUCardData::SetTanColor(COLORREF color) { return m_aauSets[m_currAAUSet].m_tanColor = color; }
inline const bool AAUCardData::HasTanColor() { return m_aauSets[m_currAAUSet].m_bTanColor; }
inline const DWORD AAUCardData::SetHasTanColor(bool has) { return m_aauSets[m_currAAUSet].m_bTanColor = has; }

inline const std::vector<AAUCardData::BoneRule> AAUCardData::GetBoneTransformationList() { return m_aauSets[m_currAAUSet].m_boneTransforms; }
inline const D3DMATRIX* AAUCardData::GetBoneTransformationRule(const TCHAR* boneName) {
	auto it = m_aauSets[m_currAAUSet].m_boneTransformMap.find(boneName);
	return it == m_aauSets[m_currAAUSet].m_boneTransformMap.end() ? NULL : &it->second;
}

inline bool AAUCardData::HasFilesSaved() { return m_savedFiles.size() > 0; }

inline const std::vector<AAUCardData::HairPart>& AAUCardData::GetHairs(BYTE kind) { return m_aauSets[m_currAAUSet].m_hairs[kind]; }

inline std::vector<Shared::Triggers::Trigger>& AAUCardData::GetTriggers() { return m_triggers; }

inline std::vector<Shared::Triggers::GlobalVariable>& AAUCardData::GetGlobalVariables() { return m_globalVars; }

inline const std::vector<Shared::Triggers::Module>& AAUCardData::GetModules() const { return m_modules; }

inline std::map<std::wstring,Shared::Triggers::Value>& AAUCardData::GetCardStorage() { return m_cardStorage; }

inline const std::vector<AAUCardData::BoneRuleV2> AAUCardData::GetMeshRuleList() { return m_boneRules; }
inline const std::map<std::wstring,std::vector<AAUCardData::BoneMod>>* AAUCardData::GetBoneRule(const TCHAR* xxFileName) {
	auto it = m_boneRuleMap.find(xxFileName);
	return it == m_boneRuleMap.end() ? NULL : &it->second;
}
inline const std::map<std::wstring,std::vector<AAUCardData::BoneMod>>* AAUCardData::GetFrameRule(const TCHAR* xxFileName) {
	auto it = m_frameRuleMap.find(xxFileName);
	return it == m_frameRuleMap.end() ? NULL : &it->second;
}

inline const std::vector<AAUCardData::SliderRule> AAUCardData::GetSliderList() { return m_sliders; }
inline const std::map<std::wstring,std::vector<std::pair<const Shared::Slider*,AAUCardData::BoneMod>>>& AAUCardData::GetSliderBoneRuleMap(int type) {
	return m_boneSliderMap[type];
}
inline const std::map<std::wstring,std::vector<std::pair<const Shared::Slider*,AAUCardData::BoneMod>>>& AAUCardData::GetSliderFrameRuleMap(int type) {
	return m_frameSliderMap[type];
}
inline const std::vector<std::pair<const Shared::Slider*,AAUCardData::BoneMod>>* AAUCardData::GetSliderBoneRule(ExtClass::CharacterStruct::Models model,std::wstring bone) {
	auto it = m_boneSliderMap[model].find(bone);
	return (it != m_boneSliderMap[model].end()) ? &it->second : NULL;
}
inline const std::vector<std::pair<const Shared::Slider*,AAUCardData::BoneMod>>* AAUCardData::GetSliderFrameRule(ExtClass::CharacterStruct::Models model,std::wstring bone) {
	auto it = m_frameSliderMap[model].find(bone);
	return (it != m_frameSliderMap[model].end()) ? &it->second : NULL;
}

/*
Important differences in versions:

OvrT: in version 1, mesh textures were relative to aaedit/data/texture/override (VER1_OVERRIDE_IMAGE_PATH)
	  in version 2, mesh textures are relative to the override path
AOvT: in version 1, override files were relative to aaplay or aaedits data folder (VER1_OVERRIDE_ARCHIVE_PATH)
	  in version 2, override files are relative to the override path 
OOvr: in version 1, object overrides were relative to aaplay or aaedits data folder (VER1_OVERRIDE_ARCHIVE_PATH)
	  in version 2, object overrides are relative to the override path 
TnRd: in version 1, the tan image were relative to data\\texture\\override\\tan\\ (VER1_TAN_PATH TEXT)
	  in version 2, the file is always relative from the override path
HrHl: in version 1, the hair highlight images were relative to data\\texture\\override\\hair_highlight\\ (VER1_HAIR_HIGHLIGHT_PATH)
	  in version 2, the hair highlight images are relative from the override path

File: in version 1, the integer denotes the root path in which the file is located (aaplay = 0, aaedit = 1).
	  in version 2, the file is always relative from the override path


*/