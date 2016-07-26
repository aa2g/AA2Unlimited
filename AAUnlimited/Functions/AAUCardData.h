#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <map>

#include "TextureImage.h"

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

	bool AddOverride(const char* texture, const char* override);
	bool RemoveOverride(const char* texture, const char* override);

	//getter functions
	inline BYTE GetTanSlot() const { return m_tanSlot; }
	inline const std::vector<std::pair<std::string, std::string>> GetOverrideList() const {
		return m_overrides;
	}
	inline const TextureImage* GetOverrideTexture(const char* texture) const {
		auto it = m_overrideMap.find(texture);
		return it == m_overrideMap.end() ? NULL : &it->second;
	}

private:
	BYTE m_tanSlot;						//used tan slot, if slot is >5.
	std::vector<std::pair<std::string, std::string>> m_overrides;	//replaces textures by other textures
	std::multimap<std::string, TextureImage> m_overrideMap;	//map-representation of vector above for actual use

private:
	DWORD m_currReadMemberId;	//used exclusively by FromBuffer, so that ReadData can print a precise error message
	static const AAUCardData g_defaultValues; //used to determine if a variable is not default and should be written to buffer/file

	//read help functions
	template<typename T>
	T ReadData(char*& buffer,int& size);
		template<typename T>
		T ReadData_sub(char*& buffer,int& size, T*);
		template<typename T>
		std::vector<T> ReadData_sub(char*& buffer,int& size,std::vector<T*>*);
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

