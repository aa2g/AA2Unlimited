#include "AAUCardData.h"

#include <limits>
#include <algorithm>
#include <intrin.h>

#include "General\ModuleInfo.h"
#include "General\Buffer.h"
#include "General\Util.h"
#include "Files\Logger.h"

const AAUCardData AAUCardData::g_defaultValues;

AAUCardData::AAUCardData()
{
	m_tanSlot = 0;
	m_hairRedirects.front = 0;
	m_hairRedirects.side = 1;
	m_hairRedirects.back = 2;
	m_hairRedirects.extension = 3;
}


AAUCardData::~AAUCardData()
{

}

void AAUCardData::Reset() {
	*this = g_defaultValues;
}

//
//generic read functions
//
template<typename T>
T AAUCardData::ReadData(char*& buffer,int& size) {
	return ReadData_sub(buffer,size,(T*)0);
}

template<typename T>
T AAUCardData::ReadData_sub(char*& buffer,int& size,T*) {
	if (size < sizeof(T)) {
		char idName[5];
		*(DWORD*)(idName) = m_currReadMemberId;
		idName[4] = '\0';
		LOGPRIO(Logger::Priority::WARN) << "Not enough space left to parse member " << idName << "(" << m_currReadMemberId << "); expected " << sizeof(T) << ", but has " << size << "\r\n";
		return T();
	}
	T retVal = *(T*)(buffer);
	buffer += sizeof(T),size -= sizeof(T);
	return retVal;
}

//read for string
std::wstring AAUCardData::ReadData_sub(char*& buffer,int& size,std::wstring*) {
	DWORD length = ReadData<DWORD>(buffer,size);
	if (size < 0 || (DWORD)size < length) {
		char idName[5];
		*(DWORD*)(idName) = m_currReadMemberId;
		idName[4] = '\0';
		LOGPRIO(Logger::Priority::WARN) << "Not enough space left to parse member " << idName << "(" << m_currReadMemberId << "); expected " << length << ", but has " << size << "\r\n";
		return std::wstring(L"");
	}
	std::wstring retVal((TCHAR*)buffer,length);
	buffer += length*sizeof(TCHAR),size -= length*sizeof(TCHAR);
	return retVal;
}

//read for vectors
template<typename T>
std::vector<T> AAUCardData::ReadData_sub(char*&buffer,int& size,std::vector<T>*) {
	DWORD length = ReadData<DWORD>(buffer,size);
	std::vector<T> retVal;
	retVal.reserve(length);
	for (int i = 0; i < length; i++) {
		T val = ReadData<T>(buffer,size);
		retVal.push_back(std::move(val));
	}
	return retVal;
}
//for pairs
template<typename T,typename U>
std::pair<T,U> AAUCardData::ReadData_sub(char*&buffer,int& size,std::pair<T,U>*) {
	T val1 = ReadData<T>(buffer,size);
	U val2 = ReadData<U>(buffer,size);
	return std::make_pair(std::move(val1),std::move(val2));
}

//
// generic write functions
//

template<typename T>
bool AAUCardData::WriteData(char** buffer,int* size,int& at,const T& data,bool resize) {
	return WriteData_sub(buffer,size,at,data,resize,(T*)0);
}

//general
template<typename T>
bool AAUCardData::WriteData_sub(char** buffer,int* size,int& at,const T& data,bool resize,T*) {
	bool ret = General::BufferAppend(buffer,size,at,(char*)(&data),sizeof(data),resize);
	at += sizeof(data);
	return ret;
}
//for string
bool AAUCardData::WriteData_sub(char** buffer,int* size,int& at, const std::wstring& data,bool resize,std::wstring*) {
	bool ret = true;
	//write size first, then buffer
	DWORD ssize = data.size();
	ret &= General::BufferAppend(buffer,size,at,(const char*)(&ssize),4,resize);
	at += 4;
	ret &= General::BufferAppend(buffer,size,at,(const char*)data.c_str(),data.size()*sizeof(TCHAR),resize);
	at += data.size() * sizeof(TCHAR);
	return ret;
}
//for vector
template<typename T>
bool AAUCardData::WriteData_sub(char** buffer,int* size,int& at, const std::vector<T>& data,bool resize,std::vector<T>*) {
	DWORD length = data.size();
	bool ret = true;
	ret &= WriteData(buffer,size,at,length,resize);
	for (DWORD i = 0; i < length; i++) {
		ret &= WriteData(buffer,size,at,data[i],resize);
	}
	return ret;
}
//for pairs
template<typename T,typename U>
bool AAUCardData::WriteData_sub(char** buffer,int* size,int& at, const std::pair<T,U>& data,bool resize,std::pair<T,U>*) {
	bool ret = true;
	ret &= WriteData(buffer,size,at,data.first,resize);
	ret &= WriteData(buffer,size,at,data.second,resize);
	return ret;
}



void AAUCardData::FromBuffer(char* buffer) {
	Reset();
	LOGPRIO(Logger::Priority::SPAM) << "reading card data...\r\n";
	//first, read chunk size (big endian)
	int size = _byteswap_ulong(*(DWORD*)(buffer)); buffer += 4;
	//then, read png chunk (also big endian)
	DWORD chunk = _byteswap_ulong(*(DWORD*)(buffer)); buffer += 4;
	if (chunk != PngChunkId) {
		return;
	}
	//read members
	while(size > 4) {
		DWORD identifier = *(DWORD*)(buffer);
		m_currReadMemberId = identifier;
		buffer += 4,size -= 4;
		switch (identifier) {
		case 'TanS':
			m_tanSlot = ReadData<BYTE>(buffer, size);
			LOGPRIO(Logger::Priority::SPAM) << "...found TanS, value " << m_tanSlot << "\r\n";
			break;
		case 'OvrT': {
			m_meshOverrides = ReadData<decltype(m_meshOverrides)>(buffer, size);
			for (const auto& it : m_meshOverrides) {
				TextureImage img(it.second.c_str());
				if (img.IsGood()) {
					m_meshOverrideMap.emplace(it.first, std::move(img));
				}
			}
			LOGPRIO(Logger::Priority::SPAM) << "...found OvrT, loaded " << m_meshOverrides.size() << " elements; "
				<< m_meshOverrideMap.size() << " were valid\r\n";
			break; }
		case 'AOvT': {
			m_archiveOverrides = ReadData<decltype(m_archiveOverrides)>(buffer, size);
			for (const auto& it : m_archiveOverrides) {
				OverrideFile img(it.second.c_str());
				if (img.IsGood()) {
					m_archiveOverrideMap.emplace(it.first, std::move(img));
				}
			}
			LOGPRIO(Logger::Priority::SPAM) << "...found AOvT, loaded " << m_archiveOverrides.size() << " elements; "
				<< m_archiveOverrideMap.size() << " were valid\r\n";
			break; }
		case 'ARdr':
			m_archiveRedirects = ReadData<decltype(m_archiveRedirects)>(buffer, size);
			for (const auto& it : m_archiveRedirects) {
				if (m_archiveRedirectMap.find(it.first) == m_archiveRedirectMap.end()) {
					m_archiveRedirectMap.emplace(it.first, it.second);
				}
			}
			LOGPRIO(Logger::Priority::SPAM) << "...found ARdr, loaded " << m_archiveRedirects.size() << " elements.\r\n";
			break;
		case 'EtLN':
			m_eyeTextures[0].texName = ReadData<std::wstring>(buffer, size);
			LOGPRIO(Logger::Priority::SPAM) << "...found EtLN: " << m_eyeTextures[0].texName << "\r\n";
			break;
		case 'EtRN':
			m_eyeTextures[1].texName = ReadData<std::wstring>(buffer, size);
			LOGPRIO(Logger::Priority::SPAM) << "...found EtRN: " << m_eyeTextures[1].texName << "\r\n";
			break;
		case 'EtLF':
			m_eyeTextures[0].texFile = ReadData<std::vector<BYTE>>(buffer, size);
			LOGPRIO(Logger::Priority::SPAM) << "...found EtLF, size " << m_eyeTextures[0].texFile.size() << "\r\n";
			break;
		case 'EtRF':
			m_eyeTextures[1].texFile = ReadData<std::vector<BYTE>>(buffer, size);
			LOGPRIO(Logger::Priority::SPAM) << "...found EtRF, size " << m_eyeTextures[1].texFile.size() << "\r\n";
			break;
		case 'HrRd':
			m_hairRedirects.full = ReadData<DWORD>(buffer, size);
			LOGPRIO(Logger::Priority::SPAM) << "found HrRd, value " << m_hairRedirects.full << "\r\n";
			break;
		}
		
	}
	if (size != 0) {
		LOGPRIO(Logger::Priority::WARN) << "size of unlimited card data mismatched; " << size << " bytes were left\r\n";
	}

	
}

void AAUCardData::FromFileBuffer(char* buffer, DWORD size) {
	//find our png chunk
	BYTE* chunk = General::FindPngChunk((BYTE*)buffer, size, AAUCardData::PngChunkIdBigEndian);
	if (chunk != NULL) {
		FromBuffer((char*)chunk);
	}
}

/*
 * Will write the entire png chunk corresponding to this card. Format is like this:
 * DWORD chunkDataLength;
 * DWORD chunkId;
 * BYTE chunkData[chunkDataLength];
 * --> list of data entrys
 * DWORD checksum
 */
int AAUCardData::ToBuffer(char** buffer,int* size, bool resize) {
	LOGPRIO(Logger::Priority::SPAM) << "dumping card info...\r\n";
	int at = 0;
	bool ret = true;
	//a define to make this function look simpler.
	//dumps the given id, followed by the value, but only if its actually different from the default value
	#define DUMP_MEMBER(id,x) \
		if(x != g_defaultValues.x) { \
			DWORD varId = id; \
			ret &= WriteData(buffer,size,at,varId,resize); \
			ret &= WriteData(buffer,size,at,x,resize);\
		} else { \
			LOGPRIO(Logger::Priority::SPAM) << 	"... " #x " had default value and was not written\r\n";\
		}
	#define DUMP_MEMBER_CONTAINER(id,x) \
		if(!x.empty()) { \
			DWORD varId = id; \
			ret &= WriteData(buffer,size,at,varId,resize); \
			ret &= WriteData(buffer,size,at,x,resize);\
		} else { \
			LOGPRIO(Logger::Priority::SPAM) << 	"... " #x " had default value and was not written\r\n";\
		}

	//so, first we would need to write the size. we dont know that yet tho, so we will put in a placeholder
	ret &= General::BufferAppend(buffer, size, at, "Msgn", 4, resize);
	at += 4;
	//write chunk id (big endian)
	ret &= General::BufferAppend(buffer, size, at, (const char*)(&PngChunkIdBigEndian), 4, resize);
	at += 4;
	//now write all member

	//tan-slot
	DUMP_MEMBER('TanS',m_tanSlot);
	//overrides
	DUMP_MEMBER_CONTAINER('OvrT',m_meshOverrides);
	DUMP_MEMBER_CONTAINER('AOvT', m_archiveOverrides);
	DUMP_MEMBER_CONTAINER('ARdr', m_archiveRedirects);
	//eye textures
	DUMP_MEMBER('EtLN', m_eyeTextures[0].texFile);
	DUMP_MEMBER('EtRN', m_eyeTextures[1].texFile);
	DUMP_MEMBER_CONTAINER('EtLF', m_eyeTextures[0].texName);
	DUMP_MEMBER_CONTAINER('EtRF', m_eyeTextures[1].texName);
	//hair redirect
	DUMP_MEMBER('HrRd', m_hairRedirects.full);

	//now we know the size of the data. its where we are now (at) minus the start of the data (8) (big endian)
	int dataSize = at - 8;
	int dataSizeSwapped = _byteswap_ulong(at - 8);
	ret &= General::BufferAppend(buffer, size, 0, (const char*)(&dataSizeSwapped), 4, resize);

	//write checksum. stub for now
	DWORD checksum = 0;
	ret &= General::BufferAppend(buffer, size, at, (const char*)(&checksum), 4, resize);

	//undefine macros again
	#undef DUMP_MEMBER_CONTAINER
	#undef DUMP_NUMBER

	return !ret ? 0 : dataSize + 12;
	
}

bool AAUCardData::AddMeshOverride(const TCHAR* texture, const TCHAR* override) {
	if (m_meshOverrideMap.find(texture) != m_meshOverrideMap.end()) return false;
	TextureImage img(override);
	if (img.IsGood()) {
		std::wstring texStr(texture);
		m_meshOverrides.emplace_back(texStr, std::wstring(override));
		m_meshOverrideMap.emplace(std::move(texStr), std::move(img));
		return true;
	}
	return false;
}

bool AAUCardData::RemoveMeshOverride(int index) {
	if (m_meshOverrides.size() <= index) return false;
	auto vMatch = m_meshOverrides.begin() + index;
	auto mapMatch = m_meshOverrideMap.find(vMatch->first);
	m_meshOverrides.erase(vMatch);
	if(mapMatch != m_meshOverrideMap.end()) m_meshOverrideMap.erase(mapMatch);
	return true;
}

bool AAUCardData::AddArchiveOverride(const TCHAR* archive, const TCHAR* archivefile, const TCHAR* override) {
	if (m_archiveOverrideMap.find(std::pair<std::wstring,std::wstring>(archive, archivefile)) != m_archiveOverrideMap.end()) return false;
	OverrideFile img(override);
	if (img.IsGood()) {
		auto toOverride = std::pair<std::wstring, std::wstring>(archive, archivefile);
		m_archiveOverrides.emplace_back(toOverride, override);
		m_archiveOverrideMap.emplace(std::move(toOverride), std::move(img));
		return true;
	}
	return false;
}

bool AAUCardData::RemoveArchiveOverride(int index) {
	if (m_archiveOverrides.size() <= index) return false;
	auto vMatch = m_archiveOverrides.begin() + index;
	auto mapMatch = m_archiveOverrideMap.find(vMatch->first);
	m_archiveOverrides.erase(vMatch);
	if (mapMatch != m_archiveOverrideMap.end()) m_archiveOverrideMap.erase(mapMatch);
	return true;
}

bool AAUCardData::AddArchiveRedirect(const TCHAR* archive, const TCHAR* archivefile, const TCHAR* redirectarchive, const TCHAR* redirectfile) {
	//here i should check if the archive is valid, but meh
	auto left = std::pair<std::wstring, std::wstring>(archive, archivefile);
	auto right = std::pair<std::wstring, std::wstring>(redirectarchive, redirectfile);
	if (m_archiveRedirectMap.find(left) != m_archiveRedirectMap.end()) return false; //allready contains it
	m_archiveRedirects.emplace_back(left, right);
	m_archiveRedirectMap.insert(std::make_pair(left, right));
	return true;
}
bool AAUCardData::RemoveArchiveRedirect(int index) {
	if (m_archiveRedirects.size() <= index) return false;
	auto vMatch = m_archiveRedirects.begin() + index;
	auto mapMatch = m_archiveRedirectMap.find(vMatch->first);
	m_archiveRedirects.erase(vMatch);
	if (mapMatch != m_archiveRedirectMap.end()) m_archiveRedirectMap.erase(mapMatch);
	return true;
}

bool AAUCardData::SetEyeTexture(int leftright, const TCHAR* texName, bool save) {
	if (texName == NULL) {
		m_eyeTextures[leftright].texName = TEXT("");
		m_eyeTextures[leftright].texFile.clear();
		return true;
	}
	std::wstring fullPath = General::BuildEditPath(TEXT("data\\texture\\eye\\"), texName);
	HANDLE file = CreateFile(fullPath.c_str(), FILE_READ_ACCESS, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (file == INVALID_HANDLE_VALUE || file == NULL) {
		return false;
	}
	if (save) {
		DWORD lo, hi;
		lo = GetFileSize(file, &hi);
		m_eyeTextures[leftright].texFile.resize(lo);
		ReadFile(file, m_eyeTextures[leftright].texFile.data(), lo, &hi, NULL);
	}
	m_eyeTextures[leftright].texName = texName;
	CloseHandle(file);
	return true;
}