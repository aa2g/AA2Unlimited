#include "AAUCardData.h"

#include <limits>
#include <algorithm>
#include <intrin.h>

#include "General\Buffer.h"
#include "Files\Logger.h"

const AAUCardData AAUCardData::g_defaultValues;

AAUCardData::AAUCardData()
{
	m_tanSlot = 0;
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
std::string AAUCardData::ReadData_sub(char*& buffer,int& size,std::string*) {
	DWORD length = ReadData<DWORD>(buffer,size);
	if (size < 0 || (DWORD)size < length) {
		char idName[5];
		*(DWORD*)(idName) = m_currReadMemberId;
		idName[4] = '\0';
		LOGPRIO(Logger::Priority::WARN) << "Not enough space left to parse member " << idName << "(" << m_currReadMemberId << "); expected " << length << ", but has " << size << "\r\n";
		return NULL;
	}
	std::string retVal(buffer,length);
	buffer += length,size -= length;
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
bool AAUCardData::WriteData_sub(char** buffer,int* size,int& at, const std::string& data,bool resize,std::string*) {
	bool ret = true;
	//write size first, then buffer
	DWORD ssize = data.size();
	ret &= General::BufferAppend(buffer,size,at,(const char*)(&ssize),4,resize);
	at += 4;
	ret &= General::BufferAppend(buffer,size,at,data.c_str(),data.size(),resize);
	at += data.size();
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
				" only " << m_meshOverrideMap.size() << " were valid\r\n";
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
				" only " << m_archiveOverrideMap.size() << " were valid\r\n";
			break; }
		}
	}
	if (size != 0) {
		LOGPRIO(Logger::Priority::WARN) << "size of unlimited card data mismatched; " << size << " bytes were left\r\n";
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

bool AAUCardData::AddMeshOverride(const char* texture, const char* override) {
	TextureImage img(override);
	if (img.IsGood()) {
		std::string texStr(texture);
		m_meshOverrides.emplace_back(texStr, std::string(override));
		m_meshOverrideMap.emplace(std::move(texStr), std::move(img));
		return true;
	}
	return false;
}

bool AAUCardData::RemoveMeshOverride(const char* texture, const char* override) {
	bool found = false;
	for (auto it = m_meshOverrides.begin(); it != m_meshOverrides.end(); it++) {
		if (it->first == texture && it->second == override) {
			m_meshOverrides.erase(it);
			found = true;
			break;
		}
	}
	auto mapMatch = m_meshOverrideMap.equal_range(texture);
	if (mapMatch.first != mapMatch.second) {
		for (auto it = mapMatch.first; it != mapMatch.second; it++) {
			if (it->second.GetFileName() == override) {
				m_meshOverrideMap.erase(it);
				break;
			}
		}
	}
	return found;
}

bool AAUCardData::RemoveMeshOverride(int index) {
	if (m_meshOverrides.size() <= index) return false;
	auto vMatch = m_meshOverrides.begin() + index;
	auto mapMatch = m_meshOverrideMap.equal_range(vMatch->first);
	if (mapMatch.first != mapMatch.second) {
		for (auto it = mapMatch.first; it != mapMatch.second; it++) {
			if (it->second.GetFileName() == vMatch->second) {
				m_meshOverrideMap.erase(it);
				break;
			}
		}
	}
	m_meshOverrides.erase(vMatch);
	return true;
}

bool AAUCardData::AddArchiveOverride(const char* archive, const char* archivefile, const char* override) {
	OverrideFile img(override);
	if (img.IsGood()) {
		auto toOverride = std::pair<std::string, std::string>(archive, archivefile);
		m_archiveOverrides.emplace_back(toOverride, override);
		m_archiveOverrideMap.emplace(std::move(toOverride), std::move(img));
		return true;
	}
	return false;
}

bool AAUCardData::RemoveArchiveOverride(const char* archive, const char* archivefile, const char* override) {
	bool found = false;
	auto leftPair = std::pair<std::string, std::string>(archive, archivefile);
	auto toFind = std::pair<std::pair<std::string, std::string>, std::string>(leftPair, override);
	auto match = std::find(m_archiveOverrides.begin(), m_archiveOverrides.end(), toFind);
	if (match != m_archiveOverrides.end()) {
		m_archiveOverrides.erase(match);
		found = true;
	}
	auto mapMatch = m_archiveOverrideMap.equal_range(leftPair);
	if (mapMatch.first != mapMatch.second) {
		for (auto it = mapMatch.first; it != mapMatch.second; it++) {
			if (it->second.GetFileName() == override) {
				m_archiveOverrideMap.erase(it);
				break;
			}
		}
	}
	return found;
}

bool AAUCardData::RemoveArchiveOverride(int index) {
	if (m_archiveOverrides.size() <= index) return false;
	auto vMatch = m_archiveOverrides.begin() + index;
	auto mapMatch = m_archiveOverrideMap.equal_range(vMatch->first);
	if (mapMatch.first != mapMatch.second) {
		for (auto it = mapMatch.first; it != mapMatch.second; it++) {
			if (it->second.GetFileName() == vMatch->second) {
				m_archiveOverrideMap.erase(it);
				break;
			}
		}
	}
	m_archiveOverrides.erase(vMatch);
	return true;
}