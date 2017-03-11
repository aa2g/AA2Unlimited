#include "AAUCardData.h"

#include <limits>
#include <algorithm>
#include <intrin.h>
#include <sstream>

#include "config.h"
#include "Serialize.h"
#include "Functions\Shared\Slider.h"
#include "General\ModuleInfo.h"
#include "General\Buffer.h"
#include "General\Util.h"
#include "Files\Logger.h"
#include "Files\Config.h"
#include "Files\ModuleFile.h"
#include "Functions\Shared\TriggerEventDistributor.h"

const AAUCardData AAUCardData::g_defaultValues;

AAUCardData::AAUCardData()
{
	m_tanSlot = 0;

	m_currAAUSet = 0;
	m_aauSets.resize(1);
	
	for(int i = 0; i < sizeof(ret_files)/sizeof(ret_files[0]); i++) {
		ret_files[i].fileEnd = 0;
		ret_files[i].fileStart = 0;
	}
	m_version = 1;
}

AAUCardData::AAUDataSet::AAUDataSet() {
	m_bOutlineColor = false;
	m_bTanColor = false;
	m_name = TEXT("(default)");
}


AAUCardData::~AAUCardData()
{
	
}

void AAUCardData::Reset() {
	*this = g_defaultValues;
}

void AAUCardData::FromBuffer(char* buffer, int size) {
	using namespace Serialize;
	LOGPRIO(Logger::Priority::SPAM) << "reading card data...\r\n";
	//read members
	if (size < 4) return;

	{
		//in version 2 and onwards, the first identifier should be the version id. if it is not,
		//then the card is older (version 1)
		DWORD versionId = *(DWORD*)(buffer);
		m_currReadMemberId = versionId;
		if(versionId == 'Vers') {
			buffer += 4,size -= 4;
			m_version = ReadData<int>(buffer,size);
		}
		if(m_version == 1) {
			if (g_Config.GetKeyValue(Config::LEGACY_MODE).iVal == 0) {
				LOGPRIO(Logger::Priority::INFO) << "a aau card version " << m_version << " was read and ignored due to legacy mode settings.\r\n";
				this->Reset();
				return;
			}
			else if (g_Config.GetKeyValue(Config::LEGACY_MODE).iVal == 2) {
				//treat as version 2 card
				m_version = 2;
			}
		}
	}

	m_currAAUSet = 0;
	m_aauSets.clear();
	m_aauSets.resize(1);
	m_aauSets[0].m_name = TEXT("(default)");

	while(size > 4) {
		DWORD identifier = *(DWORD*)(buffer);
		m_currReadMemberId = identifier;
		buffer += 4,size -= 4;
		try {
			switch (identifier) {
			case 'Vers':
				m_version = ReadData<int>(buffer,size);
				LOGPRIO(Logger::Priority::WARN) << "...found Version value after first chunk, value " << m_version << "\r\n";
				break;
			case 'TanS':
				m_tanSlot = ReadData<BYTE>(buffer,size);
				LOGPRIO(Logger::Priority::SPAM) << "...found TanS, value " << m_tanSlot << "\r\n";
				break;
			case 'AUSS': {
				//aau data set start; name, but also indicates that the following chunks write to this set
				std::wstring name = ReadData<std::wstring>(buffer,size);
				if (name != TEXT("(default)")) {
					m_currAAUSet++;
					m_aauSets.resize(m_aauSets.size()+1);
					m_aauSets[m_currAAUSet].m_name = name;
				}
				LOGPRIO(Logger::Priority::SPAM) << "...found AUSS; starting new aau data set named " << name << "r\n";
				break; }
			case 'OvrT': {
				m_aauSets[m_currAAUSet].m_meshOverrides = ReadData<decltype(m_aauSets[m_currAAUSet].m_meshOverrides)>(buffer,size);
				LOGPRIO(Logger::Priority::SPAM) << "...found OvrT, loaded " << m_aauSets[m_currAAUSet].m_meshOverrides.size() << " elements; "
					<< m_aauSets[m_currAAUSet].m_meshOverrideMap.size() << " were valid\r\n";
				break; }
			case 'AOvT': {
				m_aauSets[m_currAAUSet].m_archiveOverrides = ReadData<decltype(m_aauSets[m_currAAUSet].m_archiveOverrides)>(buffer,size);
				LOGPRIO(Logger::Priority::SPAM) << "...found AOvT, loaded " << m_aauSets[m_currAAUSet].m_archiveOverrides.size() << " elements; "
					<< m_aauSets[m_currAAUSet].m_archiveOverrideMap.size() << " were valid\r\n";
				break; }
			case 'ARdr':
				m_aauSets[m_currAAUSet].m_archiveRedirects = ReadData<decltype(m_aauSets[m_currAAUSet].m_archiveRedirects)>(buffer,size);
				LOGPRIO(Logger::Priority::SPAM) << "...found ARdr, loaded " << m_aauSets[m_currAAUSet].m_archiveRedirects.size() << " elements\r\n";
				break;
			case 'OOvr':
				m_aauSets[m_currAAUSet].m_objectOverrides = ReadData<decltype(m_aauSets[m_currAAUSet].m_objectOverrides)>(buffer,size);
				LOGPRIO(Logger::Priority::SPAM) << "...found OOvr, loaded " << m_aauSets[m_currAAUSet].m_objectOverrides.size() << " elements;"
					<< m_aauSets[m_currAAUSet].m_objectOverrideMap.size() << " were valid\r\n";
				break;
			case 'EtLN':
				m_aauSets[m_currAAUSet].m_eyeTextures[0].texName = ReadData<std::wstring>(buffer,size);
				LOGPRIO(Logger::Priority::SPAM) << "...found EtLN: " << m_aauSets[m_currAAUSet].m_eyeTextures[0].texName << "\r\n";
				break;
			case 'EtRN':
				m_aauSets[m_currAAUSet].m_eyeTextures[1].texName = ReadData<std::wstring>(buffer,size);
				LOGPRIO(Logger::Priority::SPAM) << "...found EtRN: " << m_aauSets[m_currAAUSet].m_eyeTextures[1].texName << "\r\n";
				break;
			case 'EtLF':
				ret_files[0].fileStart = buffer-4; //before the chunk
				m_aauSets[m_currAAUSet].m_eyeTextures[0].texFile = ReadData<std::vector<BYTE>>(buffer,size);
				ret_files[0].fileEnd = buffer;
				LOGPRIO(Logger::Priority::SPAM) << "...found EtLF, size " << m_aauSets[m_currAAUSet].m_eyeTextures[0].texFile.size() << "\r\n";
				break;
			case 'EtRF':
				ret_files[1].fileStart = buffer-4; //before the chunk
				m_aauSets[m_currAAUSet].m_eyeTextures[1].texFile = ReadData<std::vector<BYTE>>(buffer,size);
				ret_files[1].fileEnd = buffer;
				LOGPRIO(Logger::Priority::SPAM) << "...found EtRF, size " << m_aauSets[m_currAAUSet].m_eyeTextures[1].texFile.size() << "\r\n";
				break;
			case 'EhXN':
				m_aauSets[m_currAAUSet].m_eyeHighlightName = ReadData<decltype(m_aauSets[m_currAAUSet].m_eyeHighlightName)>(buffer,size);
				LOGPRIO(Logger::Priority::SPAM) << "...found EtXN: " << m_aauSets[m_currAAUSet].m_eyeHighlightName << "\r\n";
				break;
			case 'EhXF':
				ret_files[2].fileStart = buffer-4; //before the chunk
				m_aauSets[m_currAAUSet].m_eyeHighlightFile = ReadData<decltype(m_aauSets[m_currAAUSet].m_eyeHighlightFile)>(buffer,size);
				ret_files[2].fileEnd = buffer;
				LOGPRIO(Logger::Priority::SPAM) << "...found EtXF, size " << m_aauSets[m_currAAUSet].m_eyeHighlightFile.size() << "\r\n";
				break;
			case 'HrRd':
				buffer += 4,size -= 4;
				LOGPRIO(Logger::Priority::SPAM) << "found HrRd, but hair redirects are not supported anymore.\r\n";
				break;
			case 'TnRd': {
				auto tanName = ReadData<std::wstring>(buffer,size);
				m_aauSets[m_currAAUSet].m_tanName = tanName;
				LOGPRIO(Logger::Priority::SPAM) << "found TnRd, value " << m_aauSets[m_currAAUSet].m_tanName << "\r\n";
				break; }
			case 'HrHl': {
				auto hairHighlightName = ReadData<decltype(m_aauSets[m_currAAUSet].m_hairHighlightName)>(buffer,size);
				m_aauSets[m_currAAUSet].m_hairHighlightName = hairHighlightName;
				break; }
			case 'OlCl':
				m_aauSets[m_currAAUSet].m_bOutlineColor = true;
				m_aauSets[m_currAAUSet].m_outlineColor = ReadData<DWORD>(buffer,size);
				LOGPRIO(Logger::Priority::SPAM) << "found OlCl, value " << m_aauSets[m_currAAUSet].m_outlineColor << "\r\n";
				break;
			case 'TnCl':
				m_aauSets[m_currAAUSet].m_bTanColor = true;
				m_aauSets[m_currAAUSet].m_tanColor = ReadData<DWORD>(buffer,size);
				LOGPRIO(Logger::Priority::SPAM) << "found TnCl, value " << m_aauSets[m_currAAUSet].m_tanColor << "\r\n";
				break;
			case 'BnTr':
				m_aauSets[m_currAAUSet].m_boneTransforms = ReadData<decltype(m_aauSets[m_currAAUSet].m_boneTransforms)>(buffer,size);
				for (const auto& it : m_aauSets[m_currAAUSet].m_boneTransforms) {
					if (m_aauSets[m_currAAUSet].m_boneTransformMap.find(it.first) == m_aauSets[m_currAAUSet].m_boneTransformMap.end()) {
						m_aauSets[m_currAAUSet].m_boneTransformMap.emplace(it.first,it.second);
					}
				}
				LOGPRIO(Logger::Priority::SPAM) << "...found BnTr, loaded " << m_aauSets[m_currAAUSet].m_boneTransformMap.size() << " elements.\r\n";
				break;
			case 'HrA0':
				m_aauSets[m_currAAUSet].m_hairs[0] = ReadData<std::vector<HairPart>>(buffer,size);
				LOGPRIO(Logger::Priority::SPAM) << "found HrA0, loaded " << m_aauSets[m_currAAUSet].m_hairs[0].size() << " elements\r\n";
				break;
			case 'HrA1':
				m_aauSets[m_currAAUSet].m_hairs[1] = ReadData<std::vector<HairPart>>(buffer,size);
				LOGPRIO(Logger::Priority::SPAM) << "found HrA1, loaded " << m_aauSets[m_currAAUSet].m_hairs[1].size() << " elements\r\n";
				break;
			case 'HrA2':
				m_aauSets[m_currAAUSet].m_hairs[2] = ReadData<std::vector<HairPart>>(buffer,size);
				LOGPRIO(Logger::Priority::SPAM) << "found HrA2, loaded " << m_aauSets[m_currAAUSet].m_hairs[2].size() << " elements\r\n";
				break;
			case 'HrA3':
				m_aauSets[m_currAAUSet].m_hairs[3] = ReadData<std::vector<HairPart>>(buffer,size);
				LOGPRIO(Logger::Priority::SPAM) << "found HrA3, loaded " << m_aauSets[m_currAAUSet].m_hairs[3].size() << " elements\r\n";
				break;
			case 'File':
				ret_files[3].fileStart = buffer-4; //before the 'File'
				m_savedFiles = ReadData<decltype(m_savedFiles)>(buffer,size);
				ret_files[3].fileEnd = buffer;
				LOGPRIO(Logger::Priority::SPAM) << "found File list, loaded " << m_savedFiles.size() << " elements.\r\n";
				break;
			case 'BnT2':
				m_boneRules = ReadData<decltype(m_boneRules)>(buffer,size);
				GenBoneRuleMap();
				LOGPRIO(Logger::Priority::SPAM) << "found BnT2, loaded " << m_boneRules.size() << " elements\r\n";
				break;
			case 'Slds':
				m_sliders = ReadData<decltype(m_sliders)>(buffer,size);
				GenSliderMap();
				LOGPRIO(Logger::Priority::SPAM) << "found Slds, loaded " << m_sliders.size() << " elements\r\n";
				break;
			case 'Trgs':
				m_triggers = ReadData<decltype(m_triggers)>(buffer,size);
				LOGPRIO(Logger::Priority::SPAM) << "found Trgs, loaded " << m_triggers.size() << " elements\r\n";
				break;
			case 'TrGv':
				m_globalVars = ReadData<decltype(m_globalVars)>(buffer,size);
				LOGPRIO(Logger::Priority::SPAM) << "found TrGv, loaded " << m_globalVars.size() << " elements\r\n";
				break;
			case 'TrMd':
				m_modules = ReadData<decltype(m_modules)>(buffer,size);
				LOGPRIO(Logger::Priority::SPAM) << "found TrMd, loaded " << m_modules.size() << " elements\r\n";
				break;
			case 'TrAt':
				m_cardStorage = ReadData<decltype(m_cardStorage)>(buffer,size);
				LOGPRIO(Logger::Priority::SPAM) << "found TrAt, loaded " << m_cardStorage.size() << " elements\r\n";
				break;
			}
		}
		catch(InsufficientBufferException e) {
			char idName[5];
			*(DWORD*)(idName) = m_currReadMemberId;
			idName[4] = '\0';
			LOGPRIO(Logger::Priority::WARN) << "Not enough space left to parse member " << idName << "(" << m_currReadMemberId << "); "
				"expected " << e.ExpectedSize() << ", but has " << e.AvailableSize() << "\r\n";
		}
	}

	m_currAAUSet = 0;

	if (size != 0) {
		LOGPRIO(Logger::Priority::WARN) << "size of unlimited card data mismatched; " << size << " bytes were left\r\n";
	}


	if (m_version == 1 && g_Config.GetKeyValue(Config::LEGACY_MODE).iVal == 3) {
		ConvertToNewVersion();
	}
	m_version = 2;
	
	GenAllFileMaps(); //we do this in the end in case conversion or something changed the file paths
	
}

bool AAUCardData::FromFileBuffer(char* buffer, DWORD size) {
	Reset();
	//try to find it at the end first
	if (size < 8) {
		m_version = AAUCardData::CurrentVersion;
		return false;
	}
	DWORD aauDataSize = *(DWORD*)(&buffer[size - 8]);
	if (aauDataSize < size - 8) {
		DWORD id = *(DWORD*)(&buffer[size - 8 - aauDataSize - 4]);
		if (id == PngChunkIdBigEndian) {
			FromBuffer((char*)(&buffer[size - 8 - aauDataSize]), aauDataSize);
			return true;
		}
	}
	//else, find our png chunk
	BYTE* chunk = General::FindPngChunk((BYTE*)buffer, size, AAUCardData::PngChunkIdBigEndian);
	if(chunk == NULL) chunk = General::FindPngChunk((BYTE*)buffer,size,*(DWORD*)"AAUD");
	if (chunk != NULL) {
		//first, read chunk size (big endian)
		ret_chunkSize = (char*)chunk;
		int size = _byteswap_ulong(*(DWORD*)(chunk)); chunk += 8;
		FromBuffer((char*)chunk, size);
		return true;
	}
	m_version = AAUCardData::CurrentVersion;
	return false;
}

/*
 * Will write the png file to a buffer. If pngChunks == true, the old format will be printed,
 * that includes a pngChunk structure like this:
 * DWORD chunkDataLength;
 * DWORD chunkId;
 * BYTE chunkData[chunkDataLength];
 * --> list of data entrys
 * DWORD checksum
 * Else, it will print a different structure:
 * DWORD chunkId;
 * BYTE chunkData[chunkDataLength]
 * DWORD chunkDataLength
 */
int AAUCardData::ToBuffer(char** buffer,int* size, bool resize, bool pngChunks) {
	using namespace Serialize;
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
	#define DUMP_MEMBER_AAUSET(id,x) \
		if(m_aauSets[i].x != g_defaultValues.m_aauSets[0]. x) { \
			DWORD varId = id; \
			ret &= WriteData(buffer,size,at,varId,resize); \
			ret &= WriteData(buffer,size,at,m_aauSets[i].x,resize);\
		} else { \
			LOGPRIO(Logger::Priority::SPAM) << 	"... " #x " had default value and was not written\r\n";\
		}
	#define DUMP_MEMBER_CONTAINER_AAUSET(id,x) \
		if(!m_aauSets[i].x.empty()) { \
			DWORD varId = id; \
			ret &= WriteData(buffer,size,at,varId,resize); \
			ret &= WriteData(buffer,size,at,m_aauSets[i].x,resize);\
		} else { \
			LOGPRIO(Logger::Priority::SPAM) << 	"... " #x " had default value and was not written\r\n";\
		}


	if (pngChunks) {
		//so, first we would need to write the size. we dont know that yet tho, so we will put in a placeholder
		ret &= General::BufferAppend(buffer, size, at, "Msgn", 4, resize);
		at += 4;
	}
	//write chunk id (big endian)
	ret &= General::BufferAppend(buffer, size, at, (const char*)(&PngChunkIdBigEndian), 4, resize);
	at += 4;
	//now write all members

	//version
	DUMP_MEMBER('Vers',m_version);
	//tan-slot
	DUMP_MEMBER('TanS',m_tanSlot);
	//dump aau sets
	for (int i = 0; i < m_aauSets.size(); i++) {
		DUMP_MEMBER_AAUSET('AUSS',m_name);
		//overrides
		DUMP_MEMBER_CONTAINER_AAUSET('OvrT',m_meshOverrides);
		DUMP_MEMBER_CONTAINER_AAUSET('AOvT',m_archiveOverrides);
		DUMP_MEMBER_CONTAINER_AAUSET('ARdr',m_archiveRedirects);
		DUMP_MEMBER_CONTAINER_AAUSET('OOvr',m_objectOverrides);
		//eye textures
		DUMP_MEMBER_AAUSET('EtLN',m_eyeTextures[0].texName);
		DUMP_MEMBER_AAUSET('EtRN',m_eyeTextures[1].texName);
		DUMP_MEMBER_CONTAINER_AAUSET('EtLF',m_eyeTextures[0].texFile);
		DUMP_MEMBER_CONTAINER_AAUSET('EtRF',m_eyeTextures[1].texFile);
		//highlight texture
		DUMP_MEMBER_AAUSET('EhXN',m_eyeHighlightName);
		DUMP_MEMBER_CONTAINER_AAUSET('EhXF',m_eyeHighlightFile);
		//tans
		DUMP_MEMBER_AAUSET('TnRd',m_tanName);
		//hair highlight
		DUMP_MEMBER_AAUSET('HrHl',m_hairHighlightName);
		//bone transforms
		DUMP_MEMBER_CONTAINER_AAUSET('BnTr',m_boneTransforms);
		if (m_aauSets[i].m_bOutlineColor) {
			DUMP_MEMBER_AAUSET('OlCl',m_outlineColor);
		}
		if (m_aauSets[i].m_bTanColor) {
			DUMP_MEMBER_AAUSET('TnCl',m_tanColor);
		}

		DUMP_MEMBER_CONTAINER_AAUSET('HrA0',m_hairs[0]);
		DUMP_MEMBER_CONTAINER_AAUSET('HrA1',m_hairs[1]);
		DUMP_MEMBER_CONTAINER_AAUSET('HrA2',m_hairs[2]);
		DUMP_MEMBER_CONTAINER_AAUSET('HrA3',m_hairs[3]);
	}
	
	DUMP_MEMBER_CONTAINER('TrGv',m_globalVars);
	DUMP_MEMBER_CONTAINER('Trgs',m_triggers);
	DUMP_MEMBER_CONTAINER('TrMd',m_modules);
	DUMP_MEMBER_CONTAINER('TrAt',m_cardStorage);

	DUMP_MEMBER_CONTAINER('File',m_savedFiles);
	DUMP_MEMBER_CONTAINER('BnT2',m_boneRules);
	DUMP_MEMBER_CONTAINER('Slds',m_sliders);

	//now we know the size of the data. its where we are now (at) minus the start of the data (8) (big endian)
	if (pngChunks) {
		int dataSize = at - 8;
		int dataSizeSwapped = _byteswap_ulong(at - 8);
		ret &= General::BufferAppend(buffer, size, 0, (const char*)(&dataSizeSwapped), 4, resize);

		//write checksum
		DWORD checksum = General::Crc32((BYTE*)(*buffer)+4, dataSize+4);
		ret &= General::BufferAppend(buffer, size, at, (const char*)(&checksum), 4, resize);
		return !ret ? 0 : dataSize + 12;
	}
	else {
		int dataSize = at - 4;
		ret &= General::BufferAppend(buffer, size, at, (const char*)(&dataSize), 4, resize);
		return !ret ? 0 : dataSize + 8;
	}
	

	//undefine macros again
	#undef DUMP_MEMBER_CONTAINER
	#undef DUMP_MEMBER
	#undef DUMP_MEMBER_CONTAINER_AAUSET
	#undef DUMP_MEMBER_AAUSET

	
	
}

/*****************************/
/* Elementwise add functions */
/*****************************/

bool AAUCardData::AddMeshOverride(const TCHAR* texture, const TCHAR* override) {
	if (m_aauSets[m_currAAUSet].m_meshOverrideMap.find(texture) != m_aauSets[m_currAAUSet].m_meshOverrideMap.end()) return false;
	TextureImage img(override, TextureImage::OVERRIDE);
	if (img.IsGood()) {
		std::wstring texStr(texture);
		m_aauSets[m_currAAUSet].m_meshOverrides.emplace_back(texStr, std::wstring(override));
		m_aauSets[m_currAAUSet].m_meshOverrideMap.emplace(std::move(texStr), std::move(img));
		return true;
	}
	return false;
}

bool AAUCardData::RemoveMeshOverride(int index) {
	if (index < 0 || (size_t)index >= m_aauSets[m_currAAUSet].m_meshOverrides.size()) return false;
	auto vMatch = m_aauSets[m_currAAUSet].m_meshOverrides.begin() + index;
	auto mapMatch = m_aauSets[m_currAAUSet].m_meshOverrideMap.find(vMatch->first);
	m_aauSets[m_currAAUSet].m_meshOverrides.erase(vMatch);
	if(mapMatch != m_aauSets[m_currAAUSet].m_meshOverrideMap.end()) m_aauSets[m_currAAUSet].m_meshOverrideMap.erase(mapMatch);
	return true;
}

bool AAUCardData::AddArchiveOverride(const TCHAR* archive, const TCHAR* archivefile, const TCHAR* override) {
	if (m_aauSets[m_currAAUSet].m_archiveOverrideMap.find(std::pair<std::wstring,std::wstring>(archive, archivefile)) != m_aauSets[m_currAAUSet].m_archiveOverrideMap.end()) return false;
	OverrideFile img(override, OverrideFile::OVERRIDE);
	if (img.IsGood()) {
		auto toOverride = std::pair<std::wstring, std::wstring>(archive, archivefile);
		m_aauSets[m_currAAUSet].m_archiveOverrides.emplace_back(toOverride, override);
		m_aauSets[m_currAAUSet].m_archiveOverrideMap.emplace(std::move(toOverride), std::move(img));
		return true;
	}
	return false;
}

bool AAUCardData::RemoveArchiveOverride(int index) {
	if (index < 0 || (size_t)index >= m_aauSets[m_currAAUSet].m_archiveOverrides.size()) return false;
	auto vMatch = m_aauSets[m_currAAUSet].m_archiveOverrides.begin() + index;
	auto mapMatch = m_aauSets[m_currAAUSet].m_archiveOverrideMap.find(vMatch->first);
	m_aauSets[m_currAAUSet].m_archiveOverrides.erase(vMatch);
	if (mapMatch != m_aauSets[m_currAAUSet].m_archiveOverrideMap.end()) m_aauSets[m_currAAUSet].m_archiveOverrideMap.erase(mapMatch);
	return true;
}

bool AAUCardData::AddArchiveRedirect(const TCHAR* archive, const TCHAR* archivefile, const TCHAR* redirectarchive, const TCHAR* redirectfile) {
	//here i should check if the archive is valid, but meh
	auto left = std::pair<std::wstring, std::wstring>(archive, archivefile);
	auto right = std::pair<std::wstring, std::wstring>(redirectarchive, redirectfile);
	if (m_aauSets[m_currAAUSet].m_archiveRedirectMap.find(left) != m_aauSets[m_currAAUSet].m_archiveRedirectMap.end()) return false; //allready contains it
	m_aauSets[m_currAAUSet].m_archiveRedirects.emplace_back(left, right);
	m_aauSets[m_currAAUSet].m_archiveRedirectMap.insert(std::make_pair(left, right));
	return true;
}
bool AAUCardData::RemoveArchiveRedirect(int index) {
	if (index < 0 || (size_t)index >= m_aauSets[m_currAAUSet].m_archiveRedirects.size()) return false;
	auto vMatch = m_aauSets[m_currAAUSet].m_archiveRedirects.begin() + index;
	auto mapMatch = m_aauSets[m_currAAUSet].m_archiveRedirectMap.find(vMatch->first);
	m_aauSets[m_currAAUSet].m_archiveRedirects.erase(vMatch);
	if (mapMatch != m_aauSets[m_currAAUSet].m_archiveRedirectMap.end()) m_aauSets[m_currAAUSet].m_archiveRedirectMap.erase(mapMatch);
	return true;
}

bool AAUCardData::AddObjectOverride(const TCHAR * object,const TCHAR * file) {
	char buff[256];
	size_t n;
	wcstombs_s(&n,buff,object,256);
	std::string strObject = buff;
	wcstombs_s(&n,buff,file,256);
	std::string strFile = buff;
	if (m_aauSets[m_currAAUSet].m_objectOverrideMap.find(strObject) != m_aauSets[m_currAAUSet].m_objectOverrideMap.end()) return false; //allready contains it
	XXObjectFile ofile(file, XXObjectFile::OVERRIDE);
	if(ofile.IsGood()) {
		m_aauSets[m_currAAUSet].m_objectOverrides.emplace_back(object,file);
		m_aauSets[m_currAAUSet].m_objectOverrideMap.insert(std::make_pair(strObject,std::move(ofile)));
	}
	return true;
}

bool AAUCardData::RemoveObjectOverride(int index) {
	if (index < 0 || (size_t)index >= m_aauSets[m_currAAUSet].m_objectOverrides.size()) return false;
	auto vMatch = m_aauSets[m_currAAUSet].m_objectOverrides.begin() + index;
	char buff[256];
	size_t n;
	wcstombs_s(&n,buff,vMatch->first.c_str(),256);
	auto mapMatch = m_aauSets[m_currAAUSet].m_objectOverrideMap.find(buff);
	m_aauSets[m_currAAUSet].m_objectOverrides.erase(vMatch);
	if (mapMatch != m_aauSets[m_currAAUSet].m_objectOverrideMap.end()) m_aauSets[m_currAAUSet].m_objectOverrideMap.erase(mapMatch);
	return true;
}


bool AAUCardData::AddBoneTransformation(const TCHAR* boneName,D3DMATRIX transform) {
	if (m_aauSets[m_currAAUSet].m_boneTransformMap.find(boneName) != m_aauSets[m_currAAUSet].m_boneTransformMap.end()) return false; //allready contains it
	m_aauSets[m_currAAUSet].m_boneTransforms.emplace_back(boneName,transform);
	m_aauSets[m_currAAUSet].m_boneTransformMap.insert(std::make_pair(boneName,transform));
	return true;
}
bool AAUCardData::RemoveBoneTransformation(int index) {
	if (index < 0 || (size_t)index >= m_aauSets[m_currAAUSet].m_boneTransforms.size()) return false;
	auto vMatch = m_aauSets[m_currAAUSet].m_boneTransforms.begin() + index;
	auto mapMatch = m_aauSets[m_currAAUSet].m_boneTransformMap.find(vMatch->first);
	m_aauSets[m_currAAUSet].m_boneTransforms.erase(vMatch);
	if (mapMatch != m_aauSets[m_currAAUSet].m_boneTransformMap.end()) m_aauSets[m_currAAUSet].m_boneTransformMap.erase(mapMatch);
	return true;
}

bool AAUCardData::AddAAUDataSet(const TCHAR* name) {
	for (auto& elem : m_aauSets) {
		if (elem.m_name == name) return false;
	}
	m_aauSets.resize(m_aauSets.size() + 1);
	m_aauSets[m_aauSets.size() - 1].m_name = name;
	return true;
}

bool AAUCardData::CopyAAUDataSet(const TCHAR * name)
{
	for (auto& elem : m_aauSets) {
		if (elem.m_name == name) return false;
	}
	m_aauSets.resize(m_aauSets.size() + 1);
	m_aauSets[m_aauSets.size() - 1] = m_aauSets[GetCurrAAUSet()];
	m_aauSets[m_aauSets.size() - 1].m_name = name;
	return true;
}
bool AAUCardData::RemoveAAUDataSet(int index) {
	if (index >= m_aauSets.size()) return false;
	if (index == 0) return false;
	m_aauSets.erase(m_aauSets.begin() + index);
	if (index == m_currAAUSet) m_currAAUSet = 0;
	return true;
}
void AAUCardData::SwitchActiveAAUDataSet(int newSet) {
	if (newSet >= m_aauSets.size()) return;
	m_currAAUSet = newSet;
}

bool AAUCardData::AddBoneRule(MeshModFlag flags, const TCHAR* xxFileName,const TCHAR* boneName,AAUCardData::BoneMod mod) {
	m_boneRules.push_back(std::make_pair(std::make_pair(flags, std::pair<std::wstring,std::wstring>(xxFileName,boneName)),mod));
	if(flags & MODIFY_BONE) {
		auto mapIt = m_boneRuleMap.find(xxFileName);
		if (mapIt != m_boneRuleMap.end()) {
			auto map2it = mapIt->second.find(boneName);
			if (map2it != mapIt->second.end()) {
				//add mod
				map2it->second.push_back(mod);
			}
			else {
				//new map with mod
				std::vector<BoneMod> vec;
				vec.push_back(mod);
				mapIt->second.emplace(boneName,std::move(vec));
			}
		}
		else {
			std::map<std::wstring,std::vector<BoneMod>> map;
			std::vector<BoneMod> vec;
			vec.push_back(mod);
			map.emplace(boneName,vec);
			m_boneRuleMap.emplace(xxFileName,std::move(map));
		}
	}
	if(flags & MODIFY_FRAME) {
		auto mapIt = m_frameRuleMap.find(xxFileName);
		if (mapIt != m_frameRuleMap.end()) {
			auto map2it = mapIt->second.find(boneName);
			if (map2it != mapIt->second.end()) {
				//add mod
				map2it->second.push_back(mod);
			}
			else {
				//new map with mod
				std::vector<BoneMod> vec;
				vec.push_back(mod);
				mapIt->second.emplace(boneName,std::move(vec));
			}
		}
		else {
			std::map<std::wstring,std::vector<BoneMod>> map;
			std::vector<BoneMod> vec;
			vec.push_back(mod);
			map.emplace(boneName,vec);
			m_frameRuleMap.emplace(xxFileName,std::move(map));
		}
	}
	
	return true;
}

bool AAUCardData::RemoveBoneRule(int index) {
	if (index < 0 || (size_t)index >= m_boneRules.size()) return false;
	auto vIt = m_boneRules.begin() + index;
	int flags = vIt->first.first;
	if(flags & MODIFY_BONE) {
		auto mapIt = m_boneRuleMap.find(vIt->first.second.first);
		std::map<std::wstring,std::vector<BoneMod>>& map = mapIt->second;
		auto map2It = map.find(vIt->first.second.second);
		auto& modVec = map2It->second;
		//remove this mod from the mod vector
		for (auto it = modVec.begin(); it != modVec.end(); it++) {
			if (*it == vIt->second) {
				modVec.erase(it);
				break;
			}
		}
		//if vector is empty, remove it from the second map
		if (modVec.size() == 0) {
			map.erase(map2It);
		}
		//if this map is now empty, remove it from first map
		if (map.size() == 0) {
			m_boneRuleMap.erase(mapIt);
		}
	}
	if(flags & MODIFY_FRAME) {
		auto mapIt = m_frameRuleMap.find(vIt->first.second.first);
		std::map<std::wstring,std::vector<BoneMod>>& map = mapIt->second;
		auto map2It = map.find(vIt->first.second.second);
		auto& modVec = map2It->second;
		//remove this mod from the mod vector
		for (auto it = modVec.begin(); it != modVec.end(); it++) {
			if (*it == vIt->second) {
				modVec.erase(it);
				break;
			}
		}
		//if vector is empty, remove it from the second map
		if (modVec.size() == 0) {
			map.erase(map2It);
		}
		//if this map is now empty, remove it from first map
		if (map.size() == 0) {
			m_frameRuleMap.erase(mapIt);
		}
	}
	m_boneRules.erase(vIt);
	return true;
}

void AAUCardData::SetSliderValue(int sliderTarget,int sliderIndex,float value) {
	//save guard
	if (sliderTarget < 0 || sliderTarget >= ExtClass::CharacterStruct::N_MODELS || Shared::g_sliders[sliderTarget].size() <= sliderIndex) {
		LOGPRIO(Logger::Priority::WARN) << "invalid slider id (" << sliderTarget << "|" << sliderIndex << ") set\r\n";
		return;
	}
	//add the value to the slider vector, or remove if value is 0 and its contained
	size_t i;
	for(i = 0; i < m_sliders.size(); i++) {
		if(m_sliders[i].first.first == sliderTarget && m_sliders[i].first.second == sliderIndex) {
			//found it
			if(value == Shared::g_sliders[sliderTarget][sliderIndex].GetNeutralValue()) {
				m_sliders.erase(m_sliders.begin() + i);
			}
			else {
				m_sliders[i].second = value;
			}
			break;
		}
	}
	if(i == m_sliders.size() && value != Shared::g_sliders[sliderTarget][sliderIndex].GetNeutralValue()) {
		//didnt find, so we need to add it
		m_sliders.push_back(std::make_pair(std::make_pair(sliderTarget,sliderIndex),value));
	}
	GenSliderMap();
}

/****************************/
/* Map Generation functions */
/****************************/

void AAUCardData::GenMeshOverrideMap() {
	for(int i = 0; i < m_aauSets.size(); i++) {
		m_aauSets[i].m_meshOverrideMap.clear();
		for (const auto& it : m_aauSets[i].m_meshOverrides) {
			std::wstring path;
			TextureImage::PathStart start;
			switch (m_version) {
			case 1:
				//version 1 mean relative to the old texture path
				path = VER1_OVERRIDE_IMAGE_PATH + it.second;
				start = TextureImage::AAEDIT;
				break;
			case 2:
				//relative to override path
				path = it.second;
				start = TextureImage::OVERRIDE;
			default:
				break;
			}
			TextureImage img(path.c_str(),start);
			if (img.IsGood()) {
				m_aauSets[i].m_meshOverrideMap.emplace(it.first,std::move(img));
			}
		}
	}
}
void AAUCardData::GenArchiveOverrideMap() {
	for (int i = 0; i < m_aauSets.size(); i++) {
		m_aauSets[i].m_archiveOverrideMap.clear();
		for (const auto& it : m_aauSets[i].m_archiveOverrides) {
			std::wstring path;
			OverrideFile::PathStart start;
			switch (m_version) {
			case 1:
				//version 1 mean relative to data root, either play or edit
				path = VER1_OVERRIDE_ARCHIVE_PATH + it.second;
				start = (OverrideFile::PathStart) (OverrideFile::AAEDIT | OverrideFile::AAPLAY);
				break;
			case 2:
				//relative to override path
				path = it.second;
				start = OverrideFile::OVERRIDE;
			default:
				break;
			}
			OverrideFile img(path.c_str(), start);
			if (img.IsGood()) {
				m_aauSets[i].m_archiveOverrideMap.emplace(it.first,std::move(img));
			}
		}
	}
}
void AAUCardData::GenArchiveRedirectMap() {
	for (int i = 0; i < m_aauSets.size(); i++) {
		m_aauSets[i].m_archiveRedirectMap.clear();
		for (const auto& it : m_aauSets[i].m_archiveRedirects) {
			if (m_aauSets[i].m_archiveRedirectMap.find(it.first) == m_aauSets[i].m_archiveRedirectMap.end()) {
				m_aauSets[i].m_archiveRedirectMap.emplace(it.first,it.second);
			}
		}
	}
}
void AAUCardData::GenObjectOverrideMap() {
	for (int i = 0; i < m_aauSets.size(); i++) {
		m_aauSets[i].m_objectOverrideMap.clear();
		for (const auto& it : m_aauSets[i].m_objectOverrides) {
			char buff[256];
			size_t n;
			wcstombs_s(&n,buff,it.first.c_str(),256);
			std::string strObject = buff;
		
			std::wstring path;
			XXObjectFile::PathStart start;
			switch (m_version) {
			case 1:
				//version 1 mean relative to data root, either play or edit
				path = VER1_OVERRIDE_ARCHIVE_PATH + it.second;
				start = (OverrideFile::PathStart) (OverrideFile::AAEDIT | OverrideFile::AAPLAY);
				break;
			case 2:
				//relative to override path
				path = it.second;
				start = OverrideFile::OVERRIDE;
			default:
				break;
			}

			XXObjectFile ofile(path.c_str(), start);
			if(ofile.IsGood()) {
				m_aauSets[i].m_objectOverrideMap.insert(std::make_pair(strObject,std::move(ofile)));
			}
		}
	}
}
void AAUCardData::GenBoneRuleMap() {
	m_boneRuleMap.clear();
	for (auto& elem : m_boneRules) {
		if(elem.first.first & MODIFY_BONE) {
			auto m = m_boneRuleMap.find(elem.first.second.first);
			if (m != m_boneRuleMap.end()) {
				std::vector<BoneMod> vec;
				vec.push_back(elem.second);
				m->second.emplace(elem.first.second.second,std::move(vec));
			}
			else {
				std::map<std::wstring,std::vector<BoneMod>> tmp;
				std::vector<BoneMod> vec;
				vec.push_back(elem.second);
				tmp.emplace(elem.first.second.second,std::move(vec));

				m_boneRuleMap.emplace(elem.first.second.first,std::move(tmp));
			}
		}
		if (elem.first.first & MODIFY_FRAME) {
			auto m = m_frameRuleMap.find(elem.first.second.first);
			if (m != m_frameRuleMap.end()) {
				std::vector<BoneMod> vec;
				vec.push_back(elem.second);
				m->second.emplace(elem.first.second.second,std::move(vec));
			}
			else {
				std::map<std::wstring,std::vector<BoneMod>> tmp;
				std::vector<BoneMod> vec;
				vec.push_back(elem.second);
				tmp.emplace(elem.first.second.second,std::move(vec));

				m_frameRuleMap.emplace(elem.first.second.first,std::move(tmp));
			}
		}
	}
}
void AAUCardData::GenSliderMap() {
	for(int i = 0; i < ARRAYSIZE(m_boneSliderMap); i++) {
		m_boneSliderMap[i].clear();
	}
	for (int i = 0; i < ARRAYSIZE(m_frameSliderMap); i++) {
		m_frameSliderMap[i].clear();
	}

	for(auto elem : m_sliders) {
		int target = elem.first.first;
		if(target < 0 || target >= ExtClass::CharacterStruct::N_MODELS || Shared::g_sliders[target].size() <= elem.first.second) {
			LOGPRIO(Logger::Priority::WARN) << "invalid slider id (" << target << "|" << elem.first.second << ") read; the slider was skipped\r\n";
			continue;
		}
		const Shared::Slider& slider = Shared::g_sliders[target][elem.first.second];
		if(slider.flags & MODIFY_BONE) {
			auto& map = m_boneSliderMap[target];
			auto it = map.find(slider.boneName);
			if (it != map.end()) {
				BoneMod mod = slider.GenerateModifier(elem.second);
				it->second.push_back(std::make_pair(&slider,mod));
			}
			else {
				std::vector<std::pair<const Shared::Slider*,BoneMod>> vec;
				BoneMod mod = slider.GenerateModifier(elem.second);
				vec.push_back(std::make_pair(&slider,mod));
				map.emplace(slider.boneName,vec);
			}
		}
		if (slider.flags & MODIFY_FRAME) {
			auto& map = m_frameSliderMap[target];
			auto it = map.find(slider.boneName);
			if (it != map.end()) {
				BoneMod mod = slider.GenerateModifier(elem.second);
				it->second.push_back(std::make_pair(&slider,mod));
			}
			else {
				std::vector<std::pair<const Shared::Slider*,BoneMod>> vec;
				BoneMod mod = slider.GenerateModifier(elem.second);
				vec.push_back(std::make_pair(&slider,mod));
				map.emplace(slider.boneName,vec);
			}
		}
	}
}

void AAUCardData::GenAllFileMaps() {
	GenArchiveRedirectMap();
	GenMeshOverrideMap();
	GenArchiveOverrideMap();
	GenObjectOverrideMap();

	int tmp = m_currAAUSet;
	for (int i = 0; i < m_aauSets.size(); i++) {
		m_currAAUSet = i;
		auto temp = m_aauSets[m_currAAUSet].m_tanName; //not sure if this is stricly neccessary, but i do it out of safety
		SetTan(temp.c_str());
		temp = m_aauSets[m_currAAUSet].m_hairHighlightName;
		SetHairHighlight(temp.c_str());
	}
	m_currAAUSet = tmp;
}

/***************************/
/* Setting loose variables */
/***************************/

bool AAUCardData::SetEyeTexture(int leftright, const TCHAR* texName, bool save) {
	int other = leftright == 0 ? 1 : 0;
	if (texName == NULL) {
		m_aauSets[m_currAAUSet].m_eyeTextures[leftright].texName = TEXT("");
		m_aauSets[m_currAAUSet].m_eyeTextures[leftright].texFile.clear();
		return true;
	}
	std::wstring fullPath = General::BuildEditPath(TEXT("data\\texture\\eye\\"), texName);
	HANDLE file = CreateFile(fullPath.c_str(), FILE_READ_ACCESS, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (file == INVALID_HANDLE_VALUE || file == NULL) {
		return false;
	}
	if (save && m_aauSets[m_currAAUSet].m_eyeTextures[other].texName != m_aauSets[m_currAAUSet].m_eyeTextures[leftright].texName) {
		DWORD lo, hi;
		lo = GetFileSize(file, &hi);
		m_aauSets[m_currAAUSet].m_eyeTextures[leftright].texFile.resize(lo);
		ReadFile(file,m_aauSets[m_currAAUSet].m_eyeTextures[leftright].texFile.data(), lo, &hi, NULL);
	}
	m_aauSets[m_currAAUSet].m_eyeTextures[leftright].texName = texName;
	CloseHandle(file);
	return true;
}

bool AAUCardData::SetEyeHighlight(const TCHAR* texName) {
	if (texName == NULL) {
		m_aauSets[m_currAAUSet].m_eyeHighlightName = TEXT("");
		m_aauSets[m_currAAUSet].m_eyeHighlightFile.clear();
		return true;
	}
	if(m_aauSets[m_currAAUSet].m_eyeHighlightFile.size() > 0) {
		m_aauSets[m_currAAUSet].m_eyeHighlightFile.clear();
	}
	std::wstring fullPath = General::BuildEditPath(TEXT("data\\texture\\hilight\\"),texName);
	HANDLE file = CreateFile(fullPath.c_str(),FILE_READ_ACCESS,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
	if (file == INVALID_HANDLE_VALUE || file == NULL) {
		return false;
	}
	m_aauSets[m_currAAUSet].m_eyeHighlightName = texName;
	
	DWORD lo,hi;
	lo = GetFileSize(file,&hi);
	m_aauSets[m_currAAUSet].m_eyeHighlightFile.resize(lo);
	ReadFile(file,m_aauSets[m_currAAUSet].m_eyeHighlightFile.data(),lo,&hi,NULL);

	CloseHandle(file);
	return true;
}

bool AAUCardData::SetHairHighlight(const TCHAR* name) {
	std::wstring path; 
	TextureImage::PathStart start;
	switch(m_version) {
	case 1:
		path = VER1_HAIR_HIGHLIGHT_PATH;
		start = TextureImage::AAEDIT;
		break;
	case 2:
		path = HAIR_HIGHLIGHT_PATH;
		start = TextureImage::OVERRIDE;
	default:
		break;
	}
	path += name;
	m_aauSets[m_currAAUSet].m_hairHighlightImage = TextureImage(path.c_str(), start);
	if (m_aauSets[m_currAAUSet].m_hairHighlightImage.IsGood()) {
		m_aauSets[m_currAAUSet].m_hairHighlightName = name;
		return true;
	}
	return false;
}

bool AAUCardData::SetTan(const TCHAR* name) {
	std::wstring path;
	TextureImage::PathStart start;
	switch(m_version) {
	case 1:
		path = VER1_TAN_PATH;
		start = TextureImage::AAEDIT;
		break;
	case 2:
	default:
		path = TAN_PATH;
		start = TextureImage::OVERRIDE;
		break;
	}
	path += name;
	path += TEXT("\\");
	bool anyGood = false;
	for (int i = 0; i < 5; i++) {
		wchar_t iChar = L'0' + i;
		std::wstring file = TEXT("0");
		file += iChar;
		file += TEXT(".bmp");
		m_aauSets[m_currAAUSet].m_tanImages[i] = TextureImage((path + file).c_str(), start);
		anyGood = anyGood || m_aauSets[m_currAAUSet].m_tanImages[i].IsGood();
	}
	if (anyGood) m_aauSets[m_currAAUSet].m_tanName = name;
	return anyGood;
}

bool AAUCardData::AddHair(BYTE kind,BYTE slot,BYTE adjustment,bool flip) {
	m_aauSets[m_currAAUSet].m_hairs[kind].push_back({ kind,slot,flip,adjustment });
	return true;
}

//index is extended index for all 4 hair kinds, front, side, back, ext
bool AAUCardData::RemoveHair(int index) {
	int kind;
	for(kind = 0; kind < 4; kind++) {
		if(index < 0 || (size_t)index < m_aauSets[m_currAAUSet].m_hairs[kind].size()) {
			break;
		}
		index -= m_aauSets[m_currAAUSet].m_hairs[kind].size();
	}
	if (kind >= 4) return false;

	auto vMatch = m_aauSets[m_currAAUSet].m_hairs[kind].begin() + index;
	m_aauSets[m_currAAUSet].m_hairs[kind].erase(vMatch);
	return true; 
}

bool AAUCardData::AddModule(const TCHAR* moduleName) {
	ModuleFile modFile(moduleName);
	if(modFile.IsGood()) {
		AddModule(modFile.mod);
		return true;
	}
	return false;
}

bool AAUCardData::AddModule(const Shared::Triggers::Module& mod) {
	bool globalConflict = false;
	//check for globals in this module
	for(auto& global : mod.globals) {
		//search in our globals if we allready have this one
		Shared::Triggers::GlobalVariable* var = NULL;
		for(auto& ourGlobal : m_globalVars) {
			if(ourGlobal.name == global.name) {
				var = &ourGlobal;
				break;
			}
		}
		if(var == NULL) {
			//new var, add

		}
		else {
			//allready have this var
			globalConflict = true;
		}
		
	}

	if(!globalConflict) {
		m_modules.push_back(mod);
		m_globalVars.insert(m_globalVars.end(), mod.globals.begin(), mod.globals.end());
	}
	
	return true;
}

bool AAUCardData::RemoveModule(int index) {
	if(index < 0 || (size_t)index >= m_modules.size()) {
		return false;
	}
	m_modules.erase(m_modules.begin() + index);
	return true;
}

/********************************/
/* Save and Dump file functions */
/********************************/

void AAUCardData::SaveOverrideFiles() {
	m_savedFiles.clear();

	//general overrides first:
	//mesh overrides:
	for(const auto& mrule : m_aauSets[m_currAAUSet].m_meshOverrideMap) {
		std::vector<BYTE> buffer(mrule.second.GetFileSize());
		mrule.second.WriteToBuffer(buffer.data());
		if(buffer.size() > 0) {
			auto path = mrule.second.GetRelPath();
			m_savedFiles.emplace_back(std::make_pair(2,path),buffer);
		}
	}

	//archive overrides
	for(const auto& arule : m_aauSets[m_currAAUSet].m_archiveOverrideMap) {
		std::vector<BYTE> buffer(arule.second.GetFileSize());
		arule.second.WriteToBuffer(buffer.data());
		if (buffer.size() > 0) {
			int location;
			auto path = arule.second.GetRelPath();
			//if (General::StartsWith(arule.second.GetFilePath().c_str(),General::AAPlayPath.c_str())) {
			//	path = path;
			//	location = 0;
			//}
			//else {
				path = path;
				location = 2;
			//}
			m_savedFiles.emplace_back(std::make_pair(location,path),buffer);
		}
	}

	//object overrides
	for(const auto& orule : m_aauSets[m_currAAUSet].m_objectOverrideMap) {
		std::vector<BYTE> buffer(orule.second.GetFileSize());
		orule.second.WriteToBuffer(buffer.data());
		if (buffer.size() > 0) {
			auto path = orule.second.GetRelPath();
			m_savedFiles.emplace_back(std::make_pair(2,path),buffer);
		}
	}

	//hair highlight
	if(m_aauSets[m_currAAUSet].m_hairHighlightImage.IsGood()) {
		std::vector<BYTE> buffer(m_aauSets[m_currAAUSet].m_hairHighlightImage.GetFileSize());
		m_aauSets[m_currAAUSet].m_hairHighlightImage.WriteToBuffer(buffer.data());
		auto path = m_aauSets[m_currAAUSet].m_hairHighlightImage.GetRelPath();
		m_savedFiles.emplace_back(std::make_pair(2,path),buffer);
	}

	//tan
	for (int i = 0; i < 5; i++) {
		if (m_aauSets[m_currAAUSet].m_tanImages[i].IsGood()) {
			std::vector<BYTE> buffer(m_aauSets[m_currAAUSet].m_tanImages[i].GetFileSize());
			m_aauSets[m_currAAUSet].m_tanImages[i].WriteToBuffer(buffer.data());
			auto path = m_aauSets[m_currAAUSet].m_tanImages[i].GetRelPath();
			m_savedFiles.emplace_back(std::make_pair(2,path),buffer);
		}
	}
}

bool AAUCardData::DumpSavedOverrideFiles() {
	if (m_savedFiles.size() == 0) return false;
	int mode = g_Config.GetKeyValue(Config::SAVED_FILE_USAGE).iVal;
	if (mode == 3) return false; //if 3, do not extract
	
	// look for which files to extract
	//override files
	std::vector<std::pair<int,std::wstring>> toExtract;
	for (unsigned int i = 0; i < m_savedFiles.size(); i++) {
		auto& file = m_savedFiles[i];
		int basePath = file.first.first;
		std::wstring fullPath;
		if      (basePath == 0) fullPath = General::BuildPlayPath(file.first.second.c_str());
		else if (basePath == 1) fullPath = General::BuildEditPath(file.first.second.c_str());
		else					fullPath = General::BuildOverridePath(file.first.second.c_str());
		//check if path has any backdirections (two dots or more)
		bool suspicious = false;
		for(unsigned int i = 0; i < fullPath.size()-1; i++) {
			if(fullPath[i] == '.') {
				if(fullPath[i+1] == '.') {
					suspicious = true;
					break;
				}
				else {
					i++; //skip the second dot as well
				}
			}
		}
		if(suspicious) {
			std::wstringstream warningMessage;
			warningMessage << TEXT("The card contains a file with a suspicious file path:\r\n");
			warningMessage << fullPath << TEXT("This cards files will not be extracted. Blame the guy who made the card");
			MessageBox(NULL,warningMessage.str().c_str(),TEXT("Warning"),MB_ICONWARNING);
			return false;
		}
		if (!General::FileExists(fullPath.c_str()))
			toExtract.emplace_back(i, std::move(fullPath));
	}
	//eye textures/highlights
	std::pair<std::wstring,std::vector<BYTE>*> eyeStuff[3];
	for (int i = 0; i < 2; i++) {
		if (m_aauSets[m_currAAUSet].m_eyeTextures[i].texName.size() > 0 && m_aauSets[m_currAAUSet].m_eyeTextures[i].texFile.size() > 0) {
			//make sure texture has no folders in it first
			bool validFileName = true;
			for (wchar_t c : m_aauSets[m_currAAUSet].m_eyeTextures[i].texName) {
				if (c == L'\\') {
					validFileName = false;
				}
			}
			if (!validFileName) {
				std::wstringstream warningMessage;
				warningMessage << TEXT("The card contains a file with a suspicious file path:\r\n");
				warningMessage << m_aauSets[m_currAAUSet].m_eyeTextures[i].texName << TEXT("This cards files will not be extracted. Blame the guy who made the card");
				MessageBox(NULL,warningMessage.str().c_str(),TEXT("Warning"),MB_ICONWARNING);
				return false;
			}
			std::wstring fullPath = General::BuildEditPath(TEXT("data\\texture\\eye\\"),m_aauSets[m_currAAUSet].m_eyeTextures[i].texName.c_str());
			if (!General::FileExists(fullPath.c_str())) {
				eyeStuff[i] = make_pair(fullPath, &m_aauSets[m_currAAUSet].m_eyeTextures[i].texFile);
			}
		}
	}
	//eye highlight
	if (m_aauSets[m_currAAUSet].m_eyeHighlightName.size() > 0 && m_aauSets[m_currAAUSet].m_eyeHighlightFile.size() > 0) {
		//make sure texture has no folders in it first
		bool validFileName = true;
		for (wchar_t c : m_aauSets[m_currAAUSet].m_eyeHighlightName) {
			if (c == L'\\') {
				validFileName = false;
			}
		}
		if (!validFileName) {
			LOGPRIO(Logger::Priority::WARN) << "saved eye file " << m_aauSets[m_currAAUSet].m_eyeHighlightName << " contains paths in "
				"file name and was not extracted for safety purposes.\r\n";
		}
		std::wstring fullPath = General::BuildEditPath(TEXT("data\\texture\\hilight\\"),m_aauSets[m_currAAUSet].m_eyeHighlightName.c_str());
		if (!General::FileExists(fullPath.c_str())) {
			eyeStuff[2] = make_pair(fullPath,&m_aauSets[m_currAAUSet].m_eyeHighlightFile);
		}
	}

	if(toExtract.size() == 0 && eyeStuff[0].second == NULL && eyeStuff[1].second == NULL &&eyeStuff[2].second == NULL) {
		return false;
	}

	//if mode is 1, build a popup asking for extraction
	if(mode == 1) {
		std::wstringstream text(TEXT("This card contains files that were not found in your installation:\r\n"));
		for(auto& elem : toExtract) {
			int i = elem.first;
			if(m_savedFiles[i].first.first == 0) text << TEXT("AAPlay\\");
			else if (m_savedFiles[i].first.first == 1) text << TEXT("AAEdit\\");
			else text << OVERRIDE_PATH;
			text << m_savedFiles[i].first.second << TEXT("\r\n");
		}
		text << TEXT("These files are probably required for the card to work properly.\r\n"
					 "Do you want to extract these files now?");
		int res = MessageBox(NULL,text.str().c_str(),TEXT("Info"),MB_YESNO);
		if(res != IDYES) {
			//doesnt want to extract, abort
			return false;
		}
	}

	//create files
	//overrides
	bool success = true;
	for (auto& elem : toExtract) {
		General::CreatePathForFile(elem.second.c_str());
		HANDLE file = CreateFile(elem.second.c_str(),GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_NEW,0,NULL);
		if(file == INVALID_HANDLE_VALUE || file == NULL) {
			int err = GetLastError();
			LOGPRIO(Logger::Priority::WARN) << "could not create file " << elem.second.c_str() << ": error " << err << "\r\n";
			success = false;
			continue;
		}
		DWORD written = 0;
		auto& vec = m_savedFiles[elem.first].second;
		WriteFile(file,vec.data(),vec.size(),&written,NULL);
		CloseHandle(file);

		if(written != vec.size()) {
			success = false;
		}
	}
	//eye stuff
	for (int i = 0; i < 3; i++) {
		auto& elem = eyeStuff[i];
		if (elem.second == NULL) continue;
		HANDLE file = CreateFile(elem.first.c_str(),GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_NEW,0,NULL);
		if (file == INVALID_HANDLE_VALUE || file == NULL) {
			int err = GetLastError();
			LOGPRIO(Logger::Priority::WARN) << "could not create file " << elem.first.c_str() << ": error " << err << "\r\n";
			success = false;
			continue;
		}
		DWORD written = 0;
		auto& vec = *elem.second;
		WriteFile(file,vec.data(),vec.size(),&written,NULL);
		CloseHandle(file);

		if (written != vec.size()) {
			success = false;
		}
	}
	
	if(!success) {
		LOGPRIO(Logger::Priority::WARN) << "failed to extract some files from aau card\r\n";
	}

	return success;
}

void AAUCardData::ConvertToNewVersion() {
	if(m_version == 1) {
		std::wstringstream message;
		bool success = true;

		std::vector<std::pair<std::wstring,std::wstring>> filesToMove;

		/////////////////////////
		// change all the paths that were relative to some path to a path relative to the override folder

		//if a path allready starts with the override path, truncate it to be relative to that instead
		//else, move file path into override folder
		//if inconsitent (both rules applied), conversion failed
		auto changeFilePaths = [&](TCHAR* start, std::wstring& relPath) {
			std::wstring path = start + relPath;
			if (General::StartsWith(path,OVERRIDE_PATH)) {
				//path starts in override folder; truncate path to fit
				std::wstring& toChange = relPath;
				std::wstring temp = toChange;
				toChange = path.substr(wcslen(OVERRIDE_PATH));
				LOGPRIO(Logger::Priority::INFO) << "truncating path " << temp << " to " << toChange << "\r\n";
			}
			else {
				//path does not match with override folder; move files in there.
				OverrideFile::PathStart start = (OverrideFile::PathStart) (OverrideFile::AAEDIT | OverrideFile::AAPLAY);
				OverrideFile tmp(path.c_str(),start);

				std::wstring target = General::BuildOverridePath(relPath.c_str());
				std::wstring fileTarget = tmp.GetFilePath();
				if (tmp.IsGood() && target != fileTarget && !General::FileExists(target.c_str())) {
					filesToMove.push_back(std::make_pair(tmp.GetFilePath(),target));
				}
			}
		};

		//archive overrides; based in VER1_OVERRIDE_ARCHIVE_PATH
		for(auto& elem : m_aauSets[m_currAAUSet].m_archiveOverrides) {
			changeFilePaths(VER1_OVERRIDE_ARCHIVE_PATH, elem.second);
		}
		//mesh texture overrides; based in VER1_OVERRIDE_IMAGE_PATH
		for (auto& elem : m_aauSets[m_currAAUSet].m_meshOverrides) {
			changeFilePaths(VER1_OVERRIDE_IMAGE_PATH,elem.second);
		}
		//object overrides; based in VER1_OVERRIDE_ARCHIVE_PATH as well
		for (auto& elem : m_aauSets[m_currAAUSet].m_objectOverrides) {
			changeFilePaths(VER1_OVERRIDE_ARCHIVE_PATH,elem.second);
		}

		if(!success) {
			message << TEXT("Conversion failed.");
			MessageBox(NULL,message.str().c_str(),TEXT("Warning"),0);
			return;
		}
		
		if(filesToMove.size() > 0) {
			
			message << filesToMove.size() << TEXT(" files can be moved:\r\n");
			for (int i = 0; i < min(filesToMove.size(),5); i++) {
				message << filesToMove[i].first << TEXT(" -> ") << filesToMove[i].second << TEXT("\r\n");
			}
			if (filesToMove.size() > 5) message << TEXT("...\r\n");
			message << TEXT("\r\nDo you want to copy them now?");
			int res = MessageBox(NULL,message.str().c_str(),TEXT("Conversion - Copy Files"),MB_YESNO);
			if(res == IDYES) {
				for(auto& elem : filesToMove) {
					General::CreatePathForFile(elem.second.c_str());
					CopyFile(elem.first.c_str(),elem.second.c_str(),TRUE);
				}
			}
		}

		m_version = 2;
	}
}

void AAUCardData::ClearOverrides(){
	m_savedFiles.clear();
}