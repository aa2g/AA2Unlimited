#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <map>


#include "General/Buffer.h"
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
		MODIFY_FRAME = 1, MODIFY_BONE = 2, SUBMESH_OUTLINE = 4, SUBMESH_SHADOW = 8
	};
	static const int CurrentVersion = 3;

	AAUCardData();
	~AAUCardData();

	char *Blob;

	// TODO: this is lazy, move to .cpp
	int BlobAt;
	int BlobSize;
	bool BlobAppendBuffer(BYTE *buf, size_t len);
	bool BlobAppendEntry(std::wstring &name, int typ, BYTE *buf, size_t len);
	bool BlobAppendFile(std::wstring &name, int typ, std::wstring fullPath);
	void BlobReset();


	//writes data to a buffer, including png chunk. Returns size of buffer filled,
	//or 0 if it failed (because the buffer was too small and resize was false)
	int ToBuffer(char** buffer);
	void Reset();

	//add/remove stuffs

	//overrides
	bool AddMeshOverride(const TCHAR* texture, const TCHAR* override);
	bool RemoveMeshOverride(int index);

	bool AddArchiveOverride(const TCHAR* archice, const TCHAR* archivefile, const TCHAR* override);
	bool RemoveArchiveOverride(int index);

	bool AddArchiveRedirect(const TCHAR* archive, const TCHAR* archivefile, const TCHAR* redirectarchive, const TCHAR* redirectfile);
	bool RemoveArchiveRedirect(int index);

	bool AddObjectOverride(const TCHAR* object, const TCHAR* file);
	bool RemoveObjectOverride(int index);

	//bone transformations (deprecated)
	bool AddBoneTransformation(const TCHAR* boneName, D3DMATRIX transform);
	bool RemoveBoneTransformation(int index);

	//hairs
	bool AddHair(BYTE kind, BYTE slot, BYTE adjustment, bool flip);
	bool RemoveHair(int index);

	struct BoneMod;
	bool AddBoneRule(MeshModFlag flags, const TCHAR* xxFileName, const TCHAR* boneName, BoneMod mod);
	bool RemoveBoneRule(int index);

	bool AddSubmeshRule(MeshModFlag flags, const TCHAR * xxFileName, const TCHAR * boneName, const TCHAR * materialName, std::vector<DWORD> color);
	bool RemoveSubmeshRule(int index, MeshModFlag flags);

	void SetSliderValue(int sliderTarget, int sliderIndex, float value);

	bool SetEyeTexture(int leftright, const TCHAR* texName, bool save);
	bool SetEyeHighlight(const TCHAR* texName);

	bool SetHairHighlight(const TCHAR* name, int style = -1);

	bool SetTan(const TCHAR* name, int style = -1);

	bool UpdateCardStyle(int set, ExtClass::CharacterData* charData);
	bool CopyCardStyle(const TCHAR* name, ExtClass::CharacterData* charData);
	bool RemoveCardStyle(int index);
	void SwitchActiveCardStyle(int newSet, ExtClass::CharacterData* charData);
	int FindStyleIdxByName(std::wstring * name);

	bool AddModule(const TCHAR* moduleName);
	bool AddModule(const Shared::Triggers::Module& mod);
	bool RemoveModule(int index);

	void PrepareSaveBlob();
	void PrepareSaveBlob(int i);

	bool ConvertFilesToBlob();
	bool PrepareDumpBlob();
	bool DumpBlob();

	//rule types
	typedef std::pair<std::wstring, std::wstring> MeshOverrideRule;
	typedef std::pair<std::pair<std::wstring, std::wstring>, std::wstring> ArchiveOverrideRule;
	typedef std::pair<std::pair<std::wstring, std::wstring>, std::pair<std::wstring, std::wstring>> ArchiveRedirectRule;
	typedef std::pair<std::wstring, std::wstring> ObjectOverrideRule;
	typedef std::pair<std::wstring, D3DMATRIX> BoneRule;
	//first.first.first - file name
	//first.first.second - frame name
	//first.second - material name
	//second - color
	typedef std::pair<std::pair<std::pair<std::wstring, std::wstring>, std::wstring>, std::vector<DWORD>> SubmeshColorRule;
	typedef std::pair<std::pair<int, std::wstring>, std::vector<BYTE>> SavedFile; //int identifying base path (aaplay = 0 or aaedit = 1)
	// first.first - type
	// first.second - name
	// second - size
	typedef std::pair<std::pair<int, std::wstring>, size_t> BlobInfo;

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

	typedef std::pair<std::pair<int, std::pair<std::wstring, std::wstring>>, BoneMod> BoneRuleV2;
	typedef std::pair<std::pair<int, int>, float> SliderRule; //int is slider index in global array, float is selected value

	//getter functions
	BYTE GetTanSlot() const;

	std::vector<std::wstring>					GetAAUSetDataList() const;
	int											GetCurrAAUSet() const;
	//aauset data start
	const std::vector<MeshOverrideRule>&		GetMeshOverrideList() const;
	const TextureImage*							GetMeshOverrideTexture(const TCHAR* texture) const;

	const std::vector<ArchiveOverrideRule>&		GetArchiveOverrideList() const;
	const OverrideFile*							GetArchiveOverrideFile(const TCHAR* archive, const TCHAR* texture) const;

	const std::vector<ArchiveRedirectRule>&		GetArchiveRedirectList() const;
	const std::pair<std::wstring, std::wstring>* GetArchiveRedirectFile(const TCHAR* archive, const TCHAR* texture) const;

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

	const std::vector<DWORD>					GetSubmeshOutlineColor(std::wstring mesh, std::wstring frame, std::wstring material);
	const std::vector<DWORD>					SetSubmeshOutlineColor(std::wstring mesh, std::wstring frame, std::wstring material, std::vector<DWORD> color);
	const std::vector<DWORD>					GetSubmeshShadowColor(std::wstring mesh, std::wstring frame, std::wstring material);
	const std::vector<DWORD>					SetSubmeshShadowColor(std::wstring mesh, std::wstring frame, std::wstring material, std::vector<DWORD> color);

	const DWORD									GetTanColor();
	const DWORD									SetTanColor(COLORREF color);
	const bool									HasTanColor();
	const DWORD									SetHasTanColor(bool has);

	const std::vector<BoneRule>					GetBoneTransformationList();
	const D3DMATRIX*							GetBoneTransformationRule(const TCHAR* boneName);

	bool										HasFilesSaved();

	const std::vector<HairPart>&				GetHairs(BYTE kind);

	const std::vector<BoneRuleV2>				GetMeshRuleList();
	const std::map<std::wstring, std::vector<BoneMod>>* GetBoneRule(const TCHAR* xxFileName);
	const std::map<std::wstring, std::vector<BoneMod>>* GetFrameRule(const TCHAR* xxFileName);

	const std::vector<SliderRule> GetSliderList();
	const std::map<std::wstring, std::vector<std::pair<const Shared::Slider*, BoneMod>>>& GetSliderBoneRuleMap(int type);
	const std::map<std::wstring, std::vector<std::pair<const Shared::Slider*, BoneMod>>>& GetSliderFrameRuleMap(int type);
	const std::vector<std::pair<const Shared::Slider*, BoneMod>>* GetSliderBoneRule(ExtClass::CharacterStruct::Models model, std::wstring bone);
	const std::vector<std::pair<const Shared::Slider*, BoneMod>>* GetSliderFrameRule(ExtClass::CharacterStruct::Models model, std::wstring bone);
	//aausetdata end

	std::vector<Shared::Triggers::Trigger>&			GetTriggers();
	std::vector<Shared::Triggers::GlobalVariable>&	GetGlobalVariables();
	std::vector<Shared::Triggers::Module>&	GetModules();
	std::map<std::wstring, Shared::Triggers::Value>& GetCardStorage();

	struct CardStyle;
	std::vector<CardStyle> m_styles;

	int m_version; //saved in FIRST chunk; no chunk means version 1
	BYTE m_tanSlot;						//used tan slot, if slot is >5.


	//modifications can be saved in multiple sets.
	struct CardStyle {
		wchar_t m_name[32];

		//DATA

		std::vector<MeshOverrideRule> m_meshOverrides;	//replaces textures by other textures
		std::map<std::wstring, TextureImage> m_meshOverrideMap;	//map-representation of vector above for actual use

		std::vector<ArchiveOverrideRule> m_archiveOverrides; //<archive,file>->file
		std::map<std::pair<std::wstring, std::wstring>, OverrideFile> m_archiveOverrideMap;

		std::vector<ArchiveRedirectRule> m_archiveRedirects; //<archive,file>-><archive,file>
		std::map<std::pair<std::wstring, std::wstring>, std::pair<std::wstring, std::wstring>> m_archiveRedirectMap;

		std::vector<ObjectOverrideRule> m_objectOverrides;
		std::map<std::string, XXObjectFile> m_objectOverrideMap;

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

		std::vector<SubmeshColorRule> m_submeshOutlines;
		std::vector<SubmeshColorRule> m_submeshShadows;

		bool m_bTanColor;
		DWORD m_tanColor;

		std::vector<BoneRule> m_boneTransforms;
		std::map<std::wstring, D3DMATRIX> m_boneTransformMap;

		std::vector<HairPart> m_hairs[4];

		ExtClass::CardStyleData m_cardStyleData;

		std::vector<BoneRuleV2> m_boneRules;
		std::map<std::wstring, std::map<std::wstring, std::vector<BoneMod>>> m_boneRuleMap;
		std::map<std::wstring, std::map<std::wstring, std::vector<BoneMod>>> m_frameRuleMap;

		std::vector<SliderRule> m_sliders;
		std::map<std::wstring, std::vector<std::pair<const Shared::Slider*, BoneMod>>> m_boneSliderMap[ExtClass::CharacterStruct::N_MODELS];
		std::map<std::wstring, std::vector<std::pair<const Shared::Slider*, BoneMod>>> m_frameSliderMap[ExtClass::CharacterStruct::N_MODELS];

		CardStyle();
	};
	//std::vector<CardStyle> m_styles;
	int m_currCardStyle;

	std::vector<Shared::Triggers::GlobalVariable> m_globalVars;
	std::vector<Shared::Triggers::Trigger> m_triggers;
	std::vector<Shared::Triggers::Module> m_modules;
	std::map<std::wstring, Shared::Triggers::Value> m_cardStorage;

	std::vector<SavedFile> m_savedFiles;

	// Blobs work similiar to m_savedFiles, however they carry only name, type, filesizes - making stripping
	// of bulk data less messy (we'll simply always retain blobinfo, and cut off the blob it refers to).
	// In addition to overrides, blobs also store eye/hilite textures, leaving the old storage tags for those files unused.
	enum {
		BLOB_EYE = 0,      // eye in edit folder, must not contain \ and ..
		BLOB_HI = 1,       // hilite in edit folder, must not contain \ and ..
		BLOB_OVERRIDE = 2, // override folder in edit, must not contain ..
	};
	std::vector<BlobInfo> m_blobInfo;

	//fills data from buffer. buffer should point to start of the png chunk (the length member)
	void FromBuffer(char* buffer, int size);
	bool FromPNGBuffer(char* buffer, DWORD size);

	DWORD m_currReadMemberId;	//used exclusively by FromBuffer, so that ReadData can print a precise error message
	static const AAUCardData g_defaultValues; //used to determine if a variable is not default and should be written to buffer/file

	//generate the maps from the vectors read from the file
	void GenBoneRuleMap();
	void GenSliderMap();
	void GenAllFileMaps();
};

inline BYTE AAUCardData::GetTanSlot() const { return m_tanSlot; }

inline int AAUCardData::GetCurrAAUSet() const {
	return m_currCardStyle;
}
inline std::vector<std::wstring> AAUCardData::GetAAUSetDataList() const {
	std::vector<std::wstring> vec(m_styles.size());
	for (int i = 0; i < m_styles.size(); i++) {
		vec[i] = m_styles[i].m_name;
	}
	return vec;
}

inline const std::vector<AAUCardData::MeshOverrideRule>& AAUCardData::GetMeshOverrideList() const { return m_styles[m_currCardStyle].m_meshOverrides; }
inline const TextureImage* AAUCardData::GetMeshOverrideTexture(const TCHAR* texture) const {
	auto it = m_styles[m_currCardStyle].m_meshOverrideMap.find(texture);
	return it == m_styles[m_currCardStyle].m_meshOverrideMap.end() ? NULL : &it->second;
}
inline const std::vector<AAUCardData::ArchiveOverrideRule>& AAUCardData::GetArchiveOverrideList() const { return m_styles[m_currCardStyle].m_archiveOverrides; }
inline const OverrideFile* AAUCardData::GetArchiveOverrideFile(const TCHAR* archive, const TCHAR* texture) const {
	auto it = m_styles[m_currCardStyle].m_archiveOverrideMap.find(std::pair<std::wstring, std::wstring>(archive, texture));
	if (it == m_styles[m_currCardStyle].m_archiveOverrideMap.end())
		it = m_styles[m_currCardStyle].m_archiveOverrideMap.find(std::make_pair<std::wstring, std::wstring>(std::wstring(), texture));
	return it == m_styles[m_currCardStyle].m_archiveOverrideMap.end() ? NULL : &it->second;
}
inline const std::vector<AAUCardData::ArchiveRedirectRule>& AAUCardData::GetArchiveRedirectList() const { return m_styles[m_currCardStyle].m_archiveRedirects; }
inline const std::pair<std::wstring, std::wstring>* AAUCardData::GetArchiveRedirectFile(const TCHAR* archive, const TCHAR* texture) const {
	auto it = m_styles[m_currCardStyle].m_archiveRedirectMap.find(std::pair<std::wstring, std::wstring>(archive, texture));
	if (it == m_styles[m_currCardStyle].m_archiveRedirectMap.end())
		it = m_styles[m_currCardStyle].m_archiveRedirectMap.find(std::make_pair<std::wstring, std::wstring>(std::wstring(), texture));
	return it == m_styles[m_currCardStyle].m_archiveRedirectMap.end() ? NULL : &it->second;
}

inline const std::vector<AAUCardData::ObjectOverrideRule>& AAUCardData::GetObjectOverrideList() const { return m_styles[m_currCardStyle].m_objectOverrides; }
inline const XXObjectFile* AAUCardData::GetObjectOverrideFile(const char* objectName) const {
	auto it = m_styles[m_currCardStyle].m_objectOverrideMap.find(objectName);
	return it == m_styles[m_currCardStyle].m_objectOverrideMap.end() ? NULL : &it->second;
}

inline const std::wstring& AAUCardData::GetEyeTexture(int leftright) { return m_styles[m_currCardStyle].m_eyeTextures[leftright].texName; }
inline const std::vector<BYTE>& AAUCardData::GetEyeTextureBuffer(int leftright) { return m_styles[m_currCardStyle].m_eyeTextures[leftright].texFile; }

inline const std::wstring AAUCardData::GetEyeHighlightTexture() { return m_styles[m_currCardStyle].m_eyeHighlightName; }
inline const std::vector<BYTE>& AAUCardData::GetEyeHighlightTextureBuffer() { return m_styles[m_currCardStyle].m_eyeHighlightFile; }

inline const std::wstring& AAUCardData::GetHairHighlightName() { return m_styles[m_currCardStyle].m_hairHighlightName; }
inline const TextureImage& AAUCardData::GetHairHighlightTex() { return m_styles[m_currCardStyle].m_hairHighlightImage; }

inline const std::wstring& AAUCardData::GetTanName() { return m_styles[m_currCardStyle].m_tanName; }
inline const TextureImage& AAUCardData::GetTanTex(int i) {
	if (i >= 0 && i < 5) return m_styles[m_currCardStyle].m_tanImages[i];
	return m_styles[m_currCardStyle].m_tanImages[0];
}

inline const DWORD AAUCardData::GetOutlineColor() {
	auto color = GetSubmeshOutlineColor(L"", L"", L"");
	auto result = RGB(color[0], color[1], color[2]);
	return result;
}

inline const DWORD AAUCardData::SetOutlineColor(COLORREF color) { 
	std::vector<DWORD> colorArray;
	colorArray.push_back(GetRValue(color));
	colorArray.push_back(GetGValue(color));
	colorArray.push_back(GetBValue(color));
	union {
		DWORD i;
		float f;
	} floatyDWORD;
	floatyDWORD.f = 1;
	colorArray.push_back(floatyDWORD.i);
	SetSubmeshOutlineColor(L"", L"", L"", colorArray);
	return color;
}
inline const bool AAUCardData::HasOutlineColor() { return m_styles[m_currCardStyle].m_bOutlineColor; }
inline const DWORD AAUCardData::SetHasOutlineColor(bool has) { return m_styles[m_currCardStyle].m_bOutlineColor = has; }

inline const std::vector<DWORD> AAUCardData::GetSubmeshOutlineColor(std::wstring mesh, std::wstring frame, std::wstring material){

	auto newMeshSize = mesh.size() % 2 == 0 ? mesh.size() : (mesh.size() + 1);
	auto newFrameSize = frame.size() % 2 == 0 ? frame.size() : (frame.size() + 1);
	auto newMaterialSize = material.size() % 2 == 0 ? material.size() : (material.size() + 1);
	mesh.resize(newMeshSize);
	frame.resize(newFrameSize);
	material.resize(newMaterialSize);

	union {
		DWORD i;
		float f;
	} floatyDWORD;
	floatyDWORD.f = 1;

	std::vector<DWORD> blankColor{ 0, 0, 0, floatyDWORD.i };
	std::pair<std::pair<std::wstring, std::wstring>, std::wstring> key{ {mesh, frame}, material };
	for (int i = 0; i < m_styles[m_currCardStyle].m_submeshOutlines.size(); i++) {
		if (key == m_styles[m_currCardStyle].m_submeshOutlines[i].first) return m_styles[m_currCardStyle].m_submeshOutlines[i].second;
	}

	return blankColor;
}

inline const std::vector<DWORD> AAUCardData::SetSubmeshOutlineColor(std::wstring mesh, std::wstring frame, std::wstring material, std::vector<DWORD> color){

	auto newMeshSize = mesh.size() % 2 == 0 ? mesh.size() : (mesh.size() + 1);
	auto newFrameSize = frame.size() % 2 == 0 ? frame.size() : (frame.size() + 1);
	auto newMaterialSize = material.size() % 2 == 0 ? material.size() : (material.size() + 1);
	mesh.resize(newMeshSize);
	frame.resize(newFrameSize);
	material.resize(newMaterialSize);

	std::pair<std::pair<std::wstring, std::wstring>, std::wstring> key{ { mesh, frame }, material };
	SubmeshColorRule newColor{ key, color };
	for (int i = 0; i < m_styles[m_currCardStyle].m_submeshOutlines.size(); i++) {
		if (key == m_styles[m_currCardStyle].m_submeshOutlines[i].first) {
			m_styles[m_currCardStyle].m_submeshOutlines.erase(m_styles[m_currCardStyle].m_submeshOutlines.begin() + i);
		}
	}
	m_styles[m_currCardStyle].m_submeshOutlines.push_back(newColor);

	return color;
}

inline const std::vector<DWORD> AAUCardData::GetSubmeshShadowColor(std::wstring mesh, std::wstring frame, std::wstring material) {
	
	auto newMeshSize = mesh.size() % 2 == 0 ? mesh.size() : (mesh.size() + 1);
	auto newFrameSize = frame.size() % 2 == 0 ? frame.size() : (frame.size() + 1);
	auto newMaterialSize = material.size() % 2 == 0 ? material.size() : (material.size() + 1);
	mesh.resize(newMeshSize);
	frame.resize(newFrameSize);
	material.resize(newMaterialSize);
		
	union {
		DWORD i;
		float f;
	} floatyDWORDAT;
	union {
		DWORD i;
		float f;
	} floatyDWORDSH1;
	union {
		DWORD i;
		float f;
	} floatyDWORDSH2;
	floatyDWORDAT.f = 0.196078f;
	floatyDWORDSH1.f = 0.6;
	floatyDWORDSH2.f = 0.0015;

	std::vector<DWORD> blankColor{ 100, 30, 30, floatyDWORDAT.i, floatyDWORDSH1.i, floatyDWORDSH2.i };
	std::pair<std::pair<std::wstring, std::wstring>, std::wstring> key{ { mesh, frame }, material };
	for (int i = 0; i < m_styles[m_currCardStyle].m_submeshShadows.size(); i++) {
		if (key == m_styles[m_currCardStyle].m_submeshShadows[i].first) return m_styles[m_currCardStyle].m_submeshShadows[i].second;
	}

	return blankColor;
}

inline const std::vector<DWORD> AAUCardData::SetSubmeshShadowColor(std::wstring mesh, std::wstring frame, std::wstring material, std::vector<DWORD> color) {

	auto newMeshSize = mesh.size() % 2 == 0 ? mesh.size() : (mesh.size() + 1);
	auto newFrameSize = frame.size() % 2 == 0 ? frame.size() : (frame.size() + 1);
	auto newMaterialSize = material.size() % 2 == 0 ? material.size() : (material.size() + 1);
	mesh.resize(newMeshSize);
	frame.resize(newFrameSize);
	material.resize(newMaterialSize);

	std::pair<std::pair<std::wstring, std::wstring>, std::wstring> key{ { mesh, frame }, material };
	SubmeshColorRule newColor{ key, color };
	for (int i = 0; i < m_styles[m_currCardStyle].m_submeshShadows.size(); i++) {
		if (key == m_styles[m_currCardStyle].m_submeshShadows[i].first) {
			m_styles[m_currCardStyle].m_submeshShadows.erase(m_styles[m_currCardStyle].m_submeshShadows.begin() + i);
		}
	}
	m_styles[m_currCardStyle].m_submeshShadows.push_back(newColor);

	return color;
}


inline const DWORD AAUCardData::GetTanColor() { return m_styles[m_currCardStyle].m_tanColor; }
inline const DWORD AAUCardData::SetTanColor(COLORREF color) { return m_styles[m_currCardStyle].m_tanColor = color; }
inline const bool AAUCardData::HasTanColor() { return m_styles[m_currCardStyle].m_bTanColor; }
inline const DWORD AAUCardData::SetHasTanColor(bool has) { return m_styles[m_currCardStyle].m_bTanColor = has; }

inline const std::vector<AAUCardData::BoneRule> AAUCardData::GetBoneTransformationList() { return m_styles[m_currCardStyle].m_boneTransforms; }
inline const D3DMATRIX* AAUCardData::GetBoneTransformationRule(const TCHAR* boneName) {
	auto it = m_styles[m_currCardStyle].m_boneTransformMap.find(boneName);
	return it == m_styles[m_currCardStyle].m_boneTransformMap.end() ? NULL : &it->second;
}

inline bool AAUCardData::HasFilesSaved() {
	return m_savedFiles.size() > 0;
}

inline const std::vector<AAUCardData::HairPart>& AAUCardData::GetHairs(BYTE kind) { return m_styles[m_currCardStyle].m_hairs[kind]; }

inline std::vector<Shared::Triggers::Trigger>& AAUCardData::GetTriggers() { return m_triggers; }

inline std::vector<Shared::Triggers::GlobalVariable>& AAUCardData::GetGlobalVariables() { return m_globalVars; }

inline std::vector<Shared::Triggers::Module>& AAUCardData::GetModules() { return m_modules; }

inline std::map<std::wstring, Shared::Triggers::Value>& AAUCardData::GetCardStorage() { return m_cardStorage; }

inline const std::vector<AAUCardData::BoneRuleV2> AAUCardData::GetMeshRuleList() { return m_styles[m_currCardStyle].m_boneRules; }
inline const std::map<std::wstring, std::vector<AAUCardData::BoneMod>>* AAUCardData::GetBoneRule(const TCHAR* xxFileName) {
	auto it = m_styles[m_currCardStyle].m_boneRuleMap.find(xxFileName);
	return it == m_styles[m_currCardStyle].m_boneRuleMap.end() ? NULL : &it->second;
}
inline const std::map<std::wstring, std::vector<AAUCardData::BoneMod>>* AAUCardData::GetFrameRule(const TCHAR* xxFileName) {
	auto it = m_styles[m_currCardStyle].m_frameRuleMap.find(xxFileName);
	return it == m_styles[m_currCardStyle].m_frameRuleMap.end() ? NULL : &it->second;
}

inline const std::vector<AAUCardData::SliderRule> AAUCardData::GetSliderList() { return m_styles[m_currCardStyle].m_sliders; }
inline const std::map<std::wstring, std::vector<std::pair<const Shared::Slider*, AAUCardData::BoneMod>>>& AAUCardData::GetSliderBoneRuleMap(int type) {
	return m_styles[m_currCardStyle].m_boneSliderMap[type];
}
inline const std::map<std::wstring, std::vector<std::pair<const Shared::Slider*, AAUCardData::BoneMod>>>& AAUCardData::GetSliderFrameRuleMap(int type) {
	return m_styles[m_currCardStyle].m_frameSliderMap[type];
}
inline const std::vector<std::pair<const Shared::Slider*, AAUCardData::BoneMod>>* AAUCardData::GetSliderBoneRule(ExtClass::CharacterStruct::Models model, std::wstring bone) {
	auto it = m_styles[m_currCardStyle].m_boneSliderMap[model].find(bone);
	return (it != m_styles[m_currCardStyle].m_boneSliderMap[model].end()) ? &it->second : NULL;
}
inline const std::vector<std::pair<const Shared::Slider*, AAUCardData::BoneMod>>* AAUCardData::GetSliderFrameRule(ExtClass::CharacterStruct::Models model, std::wstring bone) {
	auto it = m_styles[m_currCardStyle].m_frameSliderMap[model].find(bone);
	return (it != m_styles[m_currCardStyle].m_frameSliderMap[model].end()) ? &it->second : NULL;
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