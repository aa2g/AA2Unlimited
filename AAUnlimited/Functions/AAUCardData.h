#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <map>

#include "TextureImage.h"
#include "OverrideFile.h"

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
	static const DWORD PngChunkId = 'AAUD';
	static const DWORD PngChunkIdBigEndian = 'DUAA';
public:
	AAUCardData();
	~AAUCardData();

	//fills data from buffer. buffer should point to start of the png chunk (the length member)
	void FromBuffer(char* buffer);
	//searches for AAUnlimited data inside file, then reads it.
	void FromFileBuffer(char* buffer, DWORD size);
	//writes data to a buffer, including png chunk. Returns size of buffer filled,
	//or 0 if it failed (because the buffer was too small and resize was false)
	int ToBuffer(char** buffer, int* size, bool resize);
	void Reset();

	bool AddMeshOverride(const TCHAR* texture, const TCHAR* override);
	bool RemoveMeshOverride(int index);
	bool AddArchiveOverride(const TCHAR* archice, const TCHAR* archivefile, const TCHAR* override);
	bool RemoveArchiveOverride(int index);
	bool AddArchiveRedirect(const TCHAR* archive, const TCHAR* archivefile, const TCHAR* redirectarchive, const TCHAR* redirectfile);
	bool RemoveArchiveRedirect(int index);

	bool SetEyeTexture(int leftright, const TCHAR* texName, bool save);

	//getter functions
	inline BYTE GetTanSlot() const { return m_tanSlot; }
	inline const std::vector<std::pair<std::wstring, std::wstring>> GetMeshOverrideList() const { return m_meshOverrides; }
	inline const TextureImage* GetMeshOverrideTexture(const TCHAR* texture) const {
		auto it = m_meshOverrideMap.find(texture);
		return it == m_meshOverrideMap.end() ? NULL : &it->second;
	}
	inline const std::vector<std::pair<std::pair<std::wstring, std::wstring>,std::wstring>> GetArchiveOverrideList() const { return m_archiveOverrides; }
	inline const OverrideFile* GetArchiveOverrideFile(const TCHAR* archive, const TCHAR* texture) const {
		auto it = m_archiveOverrideMap.find(std::pair<std::wstring, std::wstring>(archive, texture));
		return it == m_archiveOverrideMap.end() ? NULL : &it->second;
	}
	inline const std::vector<std::pair<std::pair<std::wstring, std::wstring>, std::pair<std::wstring, std::wstring>>> 
		GetArchiveRedirectList() const { return m_archiveRedirects; }
	inline const std::pair<std::wstring,std::wstring>* GetArchiveRedirectFile(const TCHAR* archive, const TCHAR* texture) const {
		auto it = m_archiveRedirectMap.find(std::pair<std::wstring, std::wstring>(archive, texture));
		return it == m_archiveRedirectMap.end() ? NULL : &it->second;
	}

	inline BYTE GetHairRedirect(BYTE category) { return m_hairRedirects.arr[category]; }
	inline void SetHairRedirect(BYTE value, BYTE category) { m_hairRedirects.arr[category] = value; }

	inline const std::wstring& GetEyeTexture(int leftright) { return m_eyeTextures[leftright].texName; }

private:
	BYTE m_tanSlot;						//used tan slot, if slot is >5.
	std::vector<std::pair<std::wstring, std::wstring>> m_meshOverrides;	//replaces textures by other textures
	std::map<std::wstring, TextureImage> m_meshOverrideMap;	//map-representation of vector above for actual use
	
	std::vector<std::pair<std::pair<std::wstring,std::wstring>,std::wstring>> m_archiveOverrides; //<archive,file>->file
	std::map<std::pair<std::wstring, std::wstring>, OverrideFile> m_archiveOverrideMap;

	std::vector<std::pair<std::pair<std::wstring, std::wstring>, std::pair<std::wstring, std::wstring>>> 
		m_archiveRedirects; //<archive,file>-><archive,file>
	std::map<std::pair<std::wstring, std::wstring>, std::pair<std::wstring, std::wstring>> m_archiveRedirectMap;

	struct {
		std::wstring texName;
		std::vector<BYTE> texFile; //contains file if it should be saved inside the card
	} m_eyeTextures[2];

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

private:
	DWORD m_currReadMemberId;	//used exclusively by FromBuffer, so that ReadData can print a precise error message
	static const AAUCardData g_defaultValues; //used to determine if a variable is not default and should be written to buffer/file

	//read help functions
	template<typename T>
	T ReadData(char*& buffer,int& size);
		template<typename T>
		T ReadData_sub(char*& buffer,int& size, T*);
		template<typename T>
		std::vector<T> ReadData_sub(char*& buffer,int& size,std::vector<T>*);
		template<typename T, typename U>
		std::pair<T,U> ReadData_sub(char*& buffer,int& size,std::pair<T,U>*);
		std::wstring ReadData_sub(char*& buffer,int& size,std::wstring*);

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
};

