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

	//fils data from buffer. buffer should point to start of the png chunk (the length member)
	void FromBuffer(char* buffer);
	//writes data to a buffer, including png chunk. Returns size of buffer filled,
	//or 0 if it failed (because the buffer was too small and resize was false)
	int ToBuffer(char** buffer, int* size, bool resize);
	void Reset();

	bool AddMeshOverride(const char* texture, const char* override);
	bool RemoveMeshOverride(const char* texture, const char* override);
	bool RemoveMeshOverride(int index);
	bool AddArchiveOverride(const char* archice, const char* archivefile, const char* override);
	bool RemoveArchiveOverride(const char* archive, const char* archivefile, const char* override);
	bool RemoveArchiveOverride(int index);

	//getter functions
	inline BYTE GetTanSlot() const { return m_tanSlot; }
	inline const std::vector<std::pair<std::string, std::string>> GetMeshOverrideList() const { return m_meshOverrides; }
	inline const TextureImage* GetMeshOverrideTexture(const char* texture) const {
		auto it = m_meshOverrideMap.find(texture);
		return it == m_meshOverrideMap.end() ? NULL : &it->second;
	}
	inline const std::vector<std::pair<std::pair<std::string, std::string>,std::string>> GetArchiveOverrideList() const { return m_archiveOverrides; }
	inline const OverrideFile* GetArchiveOverrideFile(const char* archive, const char* texture) const {
		auto it = m_archiveOverrideMap.find(std::pair<std::string, std::string>(archive, texture));
		return it == m_archiveOverrideMap.end() ? NULL : &it->second;
	}


private:
	BYTE m_tanSlot;						//used tan slot, if slot is >5.
	std::vector<std::pair<std::string, std::string>> m_meshOverrides;	//replaces textures by other textures
	std::multimap<std::string, TextureImage> m_meshOverrideMap;	//map-representation of vector above for actual use
	
	std::vector<std::pair<std::pair<std::string,std::string>,std::string>> m_archiveOverrides; //<archive,file>->file
	std::multimap<std::pair<std::string, std::string>, OverrideFile> m_archiveOverrideMap;
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
		std::string ReadData_sub(char*& buffer,int& size,std::string*);

	//write help functions
	template<typename T>
	bool WriteData(char** buffer,int* size,int& at,const T& data, bool resize);
		template<typename T>
		bool WriteData_sub(char** buffer,int* size,int& at, const T& data,bool resize, T*);
		bool WriteData_sub(char** buffer,int* size,int& at, const std::string& data,bool resize,std::string*);
		template<typename T>
		bool WriteData_sub(char** buffer,int* size,int& at, const std::vector<T>& data,bool resize,std::vector<T>*);
		template<typename T, typename U>
		bool WriteData_sub(char** buffer,int* size,int& at, const std::pair<T,U>& data,bool resize,std::pair<T,U>*);
};

