#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <map>
#include <d3d9.h>

#include "TextureImage.h"
#include "OverrideFile.h"
#include "XXObjectFile.h"
#include "External\ExternalClasses\CharacterStruct.h"

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

	bool AddMeshOverride(const TCHAR* texture, const TCHAR* override);
	bool RemoveMeshOverride(int index);
	bool AddArchiveOverride(const TCHAR* archice, const TCHAR* archivefile, const TCHAR* override);
	bool RemoveArchiveOverride(int index);
	bool AddArchiveRedirect(const TCHAR* archive, const TCHAR* archivefile, const TCHAR* redirectarchive, const TCHAR* redirectfile);
	bool RemoveArchiveRedirect(int index);
	bool AddObjectOverride(const TCHAR* object,const TCHAR* file);
	bool RemoveObjectOverride(int index);
	bool AddBoneTransformation(const TCHAR* boneName,D3DMATRIX transform);
	bool RemoveBoneTransformation(int index);
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

	void SaveOverrideFiles();
	bool DumpSavedOverrideFiles();

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
	inline BYTE GetTanSlot() const { return m_tanSlot; }
	inline const std::vector<MeshOverrideRule>& GetMeshOverrideList() const { return m_meshOverrides; }
	inline const TextureImage* GetMeshOverrideTexture(const TCHAR* texture) const {
		auto it = m_meshOverrideMap.find(texture);
		return it == m_meshOverrideMap.end() ? NULL : &it->second;
	}
	inline const std::vector<ArchiveOverrideRule>& GetArchiveOverrideList() const { return m_archiveOverrides; }
	inline const OverrideFile* GetArchiveOverrideFile(const TCHAR* archive, const TCHAR* texture) const {
		auto it = m_archiveOverrideMap.find(std::pair<std::wstring, std::wstring>(archive, texture));
		return it == m_archiveOverrideMap.end() ? NULL : &it->second;
	}
	inline const std::vector<ArchiveRedirectRule>& GetArchiveRedirectList() const { return m_archiveRedirects; }
	inline const std::pair<std::wstring,std::wstring>* GetArchiveRedirectFile(const TCHAR* archive, const TCHAR* texture) const {
		auto it = m_archiveRedirectMap.find(std::pair<std::wstring, std::wstring>(archive, texture));
		return it == m_archiveRedirectMap.end() ? NULL : &it->second;
	}

	inline const std::vector<ObjectOverrideRule>& GetObjectOverrideList() const { return m_objectOverrides; }
	inline const XXObjectFile* GetObjectOverrideFile(const char* objectName) const {
		auto it = m_objectOverrideMap.find(objectName);
		return it == m_objectOverrideMap.end() ? NULL : &it->second;
	}

	inline BYTE GetHairRedirect(BYTE category) { return m_hairRedirects.arr[category]; }
	inline void SetHairRedirect(BYTE category, BYTE value) { m_hairRedirects.arr[category] = value; }

	inline const std::wstring& GetEyeTexture(int leftright) { return m_eyeTextures[leftright].texName; }
	inline const std::vector<BYTE>& GetEyeTextureBuffer(int leftright) { return m_eyeTextures[leftright].texFile; }

	inline const std::wstring GetEyeHighlightTexture() { return m_eyeHighlightName; }
	inline const std::vector<BYTE>& GetEyeHighlightTextureBuffer() { return m_eyeHighlightFile; }

	inline const std::wstring& GetHairHighlightName() { return m_hairHighlightName; }
	inline const TextureImage& GetHairHighlightTex() { return m_hairHighlightImage; }

	inline const std::wstring& GetTanName() { return m_tanName; }
	inline const TextureImage& GetTanTex(int i) {
		if (i >= 0 && i < 5) return m_tanImages[i];
		return m_tanImages[0];
	}
	
	inline const DWORD GetOutlineColor() { return m_outlineColor; }
	inline const DWORD SetOutlineColor(COLORREF color) { return m_outlineColor = color; }
	inline const bool HasOutlineColor() { return m_bOutlineColor; }
	inline const DWORD SetHasOutlineColor(bool has) { return m_bOutlineColor = has; }

	inline const DWORD GetTanColor() { return m_tanColor; }
	inline const DWORD SetTanColor(COLORREF color) { return m_tanColor = color; }
	inline const bool HasTanColor() { return m_bTanColor; }
	inline const DWORD SetHasTanColor(bool has) { return m_bTanColor = has; }

	inline const std::vector<BoneRule> GetBoneTransformationList() { return m_boneTransforms; }
	inline const D3DMATRIX* GetBoneTransformationRule(const TCHAR* boneName) {
		auto it = m_boneTransformMap.find(boneName);
		return it == m_boneTransformMap.end() ? NULL : &it->second;
	}

	inline bool HasFilesSaved() { return m_savedFiles.size() > 0; }

	inline const std::vector<HairPart>& GetHairs(BYTE kind) { return m_hairs[kind]; }

	inline const std::vector<BoneRuleV2> GetMeshRuleList() { return m_boneRules; }
	inline const std::map<std::wstring, std::vector<BoneMod>>* GetBoneRule(const TCHAR* xxFileName) {
		auto it = m_boneRuleMap.find(xxFileName);
		return it == m_boneRuleMap.end() ? NULL : &it->second;
	}
	inline const std::map<std::wstring,std::vector<BoneMod>>* GetFrameRule(const TCHAR* xxFileName) {
		auto it = m_frameRuleMap.find(xxFileName);
		return it == m_frameRuleMap.end() ? NULL : &it->second;
	}

	inline const std::vector<SliderRule> GetSliderList() { return m_sliders; }
	inline const std::map<std::wstring,std::vector<std::pair<const Shared::Slider*,BoneMod>>>& GetSliderBoneRuleMap(int type) {
		return m_boneSliderMap[type];
	}
	inline const std::map<std::wstring,std::vector<std::pair<const Shared::Slider*,BoneMod>>>& GetSliderFrameRuleMap(int type) {
		return m_frameSliderMap[type];
	}
	inline const std::vector<std::pair<const Shared::Slider*,BoneMod>>* GetSliderBoneRule(ExtClass::CharacterStruct::Models model, std::wstring bone) {
		auto it = m_boneSliderMap[model].find(bone);
		return (it != m_boneSliderMap[model].end()) ? &it->second : NULL;
	}
	inline const std::vector<std::pair<const Shared::Slider*,BoneMod>>* GetSliderFrameRule(ExtClass::CharacterStruct::Models model,std::wstring bone) {
		auto it = m_frameSliderMap[model].find(bone);
		return (it != m_frameSliderMap[model].end()) ? &it->second : NULL;
	}

private:
	BYTE m_tanSlot;						//used tan slot, if slot is >5.
	std::vector<MeshOverrideRule> m_meshOverrides;	//replaces textures by other textures
	std::map<std::wstring, TextureImage> m_meshOverrideMap;	//map-representation of vector above for actual use
	
	std::vector<ArchiveOverrideRule> m_archiveOverrides; //<archive,file>->file
	std::map<std::pair<std::wstring, std::wstring>, OverrideFile> m_archiveOverrideMap;

	std::vector<ArchiveRedirectRule> m_archiveRedirects; //<archive,file>-><archive,file>
	std::map<std::pair<std::wstring, std::wstring>, std::pair<std::wstring, std::wstring>> m_archiveRedirectMap;

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

	union {
		DWORD full;
		BYTE arr[4];
		struct {
			BYTE front;
			BYTE side;
			BYTE back;
			BYTE extension;
		};
	} m_hairRedirects;

	bool m_bOutlineColor;
	DWORD m_outlineColor;

	bool m_bTanColor;
	DWORD m_tanColor;

	std::vector<BoneRule> m_boneTransforms;
	std::map<std::wstring,D3DMATRIX> m_boneTransformMap;

	std::vector<SavedFile> m_savedFiles;

	std::vector<HairPart> m_hairs[4];

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

	//read help functions
	template<typename T>
	T ReadData(char*& buffer,int& size);
		template<typename T>
		T ReadData_sub(char*& buffer,int& size, T*);
		std::vector<BYTE> ReadData_sub(char*& buffer,int& size,std::vector<BYTE>*);
		template<typename T>
		std::vector<T> ReadData_sub(char*& buffer,int& size,std::vector<T>*);
		template<typename T, typename U>
		std::pair<T,U> ReadData_sub(char*& buffer,int& size,std::pair<T,U>*);
		std::wstring ReadData_sub(char*& buffer,int& size,std::wstring*);
		template<typename T, typename U>
		std::map<T, U> ReadData_sub(char*& buffer, int& size, std::map<T, U>*);

	//write help functions
	template<typename T>
	bool WriteData(char** buffer,int* size,int& at,const T& data, bool resize);
		template<typename T>
		bool WriteData_sub(char** buffer,int* size,int& at, const T& data,bool resize, T*);
		bool WriteData_sub(char** buffer,int* size,int& at, const std::wstring& data,bool resize,std::wstring*);
		template<typename T>
		bool WriteData_sub(char** buffer,int* size,int& at, const std::vector<T>& data,bool resize,std::vector<T>*);
		template<typename T, typename U>
		bool WriteData_sub(char** buffer,int* size,int& at, const std::pair<T,U>& data,bool resize,std::pair<T,U>*);
		template<typename T, typename U>
		bool WriteData_sub(char** buffer,int* size,int& at, const std::map<T, U>& data, bool resize, std::map<T, U>*);

	//generate the maps from the vectors read from the file
	void GenMeshOverrideMap();
	void GenArchiveOverrideMap();
	void GenArchiveRedirectMap();
	void GenObjectOverrideMap();
	void GenBoneRuleMap();
	void GenSliderMap();
};

