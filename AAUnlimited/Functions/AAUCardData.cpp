#include "AAUCardData.h"


#include "defs.h"
#include "Serialize.h"
#include "Functions\Shared\Slider.h"
#include "General\ModuleInfo.h"
#include "General\Buffer.h"
#include "General\Util.h"
#include "Files\Logger.h"
#include "Files\Config.h"
#include "Files\ModuleFile.h"
#include "Functions\Shared\TriggerEventDistributor.h"
#include "MemMods/Shared/Events/ArchiveFileOpen.h"

const AAUCardData AAUCardData::g_defaultValues;

AAUCardData::AAUCardData()
{
	m_tanSlot = 0;

	m_currCardStyle = 0;
	m_styles.resize(1);
	m_version = 1;
}

AAUCardData::CardStyle::CardStyle() {
	m_bOutlineColor = false;
	m_bTanColor = false;
	wcscpy_s(m_name, TEXT("(default)"));
}


AAUCardData::~AAUCardData()
{
	BlobReset();
}

void AAUCardData::Reset() {
	BlobReset();
	*this = g_defaultValues;
	LOGPRIO(Logger::Priority::SPAM) << "resetting card data\n";
}

bool AAUCardData::FromPNGBuffer(char* buffer, DWORD size) {
	Reset();
	if (size < 8) {
		m_version = AAUCardData::CurrentVersion;
		return false;
	}

	BYTE* chunk = General::FindPngChunk((BYTE*)buffer, size, AAUCardData::PngChunkIdBigEndian);
	if (chunk) {
		int size = _byteswap_ulong(*(DWORD*)(chunk)); chunk += 8;
		FromBuffer((char*)chunk, size);
		GenAllFileMaps();
		return true;
	}
	m_version = AAUCardData::CurrentVersion;
	return false;
}


void AAUCardData::FromBuffer(char* buffer, int size) {
	using namespace Serialize;
	LOGPRIO(Logger::Priority::SPAM) << std::dec << "reading card data " << size << " bytes...\r\n";
	//read members
	if (size < 4) return;

	{
		//in version 2 and onwards, the first identifier should be the version id. if it is not,
		//then the card is older (version 1)
		DWORD versionId = *(DWORD*)(buffer);
		m_currReadMemberId = versionId;
		if (versionId == 'Vers') {
			buffer += 4, size -= 4;
			m_version = ReadData<int>(buffer, size);
		}
		else {
			LOGPRIO(Logger::Priority::WARN) << "Version 1 cards no longer supported.\r\n";
			return;
		}
	}

	m_currCardStyle = 0;
	m_styles.clear();
	m_styles.resize(1);
	wcscpy_s(m_styles[0].m_name, TEXT("(default)"));

	while (size > 4) {
		DWORD identifier = *(DWORD*)(buffer);
		m_currReadMemberId = identifier;
		buffer += 4, size -= 4;
		try {
			switch (identifier) {
			case 'Vers':
				m_version = ReadData<int>(buffer, size);
				LOGPRIO(Logger::Priority::WARN) << "...found Version value after first chunk, value " << m_version << "\r\n";
				break;
			case 'TanS':
				m_tanSlot = ReadData<BYTE>(buffer, size);
				LOGPRIO(Logger::Priority::SPAM) << "...found TanS, value " << m_tanSlot << "\r\n";
				break;
			case 'AUSS': {
				//aau data set start; name, but also indicates that the following chunks write to this set
				wchar_t wcharName[32];	//TODO: remove magic number
				for (int letter = 0; letter < 32; letter++) {	//read the buffer
					wcharName[letter] = (wchar_t)*buffer;
					buffer += sizeof(wchar_t);
					size -= sizeof(wchar_t);
				}
				if (wcscmp(wcharName, L"(default)")) {	//if it's an actual Style name
					m_currCardStyle++;
					m_styles.resize(m_styles.size() + 1);
					wcscpy_s(m_styles[m_currCardStyle].m_name, wcharName);
				}
				LOGPRIO(Logger::Priority::SPAM) << "...found AUSS; starting new aau data set named " << std::wstring(wcharName) << "\r\n";
				break; }
			case 'AUDS': {
				//aau card data set
				m_styles[m_currCardStyle].m_cardStyleData = ReadData<decltype(m_styles[m_currCardStyle].m_cardStyleData)>(buffer, size);
				LOGPRIO(Logger::Priority::SPAM) << "...found AUDS, size: " << sizeof(m_styles[m_currCardStyle].m_cardStyleData) << "\r\n";
				break;
			}
			case 'OvrT': {
				m_styles[m_currCardStyle].m_meshOverrides = ReadData<decltype(m_styles[m_currCardStyle].m_meshOverrides)>(buffer, size);
				LOGPRIO(Logger::Priority::SPAM) << "...found OvrT, loaded " << m_styles[m_currCardStyle].m_meshOverrides.size() << " elements; "
					<< m_styles[m_currCardStyle].m_meshOverrideMap.size() << " were valid\r\n";
				break; }
			case 'AOvT': {
				m_styles[m_currCardStyle].m_archiveOverrides = ReadData<decltype(m_styles[m_currCardStyle].m_archiveOverrides)>(buffer, size);
				LOGPRIO(Logger::Priority::SPAM) << "...found AOvT, loaded " << m_styles[m_currCardStyle].m_archiveOverrides.size() << " elements; "
					<< m_styles[m_currCardStyle].m_archiveOverrideMap.size() << " were valid\r\n";
				break; }
			case 'ARdr':
				m_styles[m_currCardStyle].m_archiveRedirects = ReadData<decltype(m_styles[m_currCardStyle].m_archiveRedirects)>(buffer, size);
				LOGPRIO(Logger::Priority::SPAM) << "...found ARdr, loaded " << m_styles[m_currCardStyle].m_archiveRedirects.size() << " elements\r\n";
				break;
			case 'OOvr':
				m_styles[m_currCardStyle].m_objectOverrides = ReadData<decltype(m_styles[m_currCardStyle].m_objectOverrides)>(buffer, size);
				LOGPRIO(Logger::Priority::SPAM) << "...found OOvr, loaded " << m_styles[m_currCardStyle].m_objectOverrides.size() << " elements;"
					<< m_styles[m_currCardStyle].m_objectOverrideMap.size() << " were valid\r\n";
				break;
			case 'EtLN':
				m_styles[m_currCardStyle].m_eyeTextures[0].texName = ReadData<std::wstring>(buffer, size);
				LOGPRIO(Logger::Priority::SPAM) << "...found EtLN: " << m_styles[m_currCardStyle].m_eyeTextures[0].texName << "\r\n";
				break;
			case 'EtRN':
				m_styles[m_currCardStyle].m_eyeTextures[1].texName = ReadData<std::wstring>(buffer, size);
				LOGPRIO(Logger::Priority::SPAM) << "...found EtRN: " << m_styles[m_currCardStyle].m_eyeTextures[1].texName << "\r\n";
				break;
			case 'EtLF':
				//ret_files[0].fileStart = buffer - 4; //before the chunk
				m_styles[m_currCardStyle].m_eyeTextures[0].texFile = ReadData<std::vector<BYTE>>(buffer, size);
				//ret_files[0].fileEnd = buffer;
				LOGPRIO(Logger::Priority::SPAM) << "...found EtLF, size " << m_styles[m_currCardStyle].m_eyeTextures[0].texFile.size() << "\r\n";
				break;
			case 'EtRF':
				//ret_files[1].fileStart = buffer - 4; //before the chunk
				m_styles[m_currCardStyle].m_eyeTextures[1].texFile = ReadData<std::vector<BYTE>>(buffer, size);
				//ret_files[1].fileEnd = buffer;
				LOGPRIO(Logger::Priority::SPAM) << "...found EtRF, size " << m_styles[m_currCardStyle].m_eyeTextures[1].texFile.size() << "\r\n";
				break;
			case 'EhXN':
				m_styles[m_currCardStyle].m_eyeHighlightName = ReadData<decltype(m_styles[m_currCardStyle].m_eyeHighlightName)>(buffer, size);
				LOGPRIO(Logger::Priority::SPAM) << "...found EtXN: " << m_styles[m_currCardStyle].m_eyeHighlightName << "\r\n";
				break;
			case 'EhXF':
				//ret_files[2].fileStart = buffer - 4; //before the chunk
				m_styles[m_currCardStyle].m_eyeHighlightFile = ReadData<decltype(m_styles[m_currCardStyle].m_eyeHighlightFile)>(buffer, size);
				//ret_files[2].fileEnd = buffer;
				LOGPRIO(Logger::Priority::SPAM) << "...found EtXF, size " << m_styles[m_currCardStyle].m_eyeHighlightFile.size() << "\r\n";
				break;
			case 'HrRd':
				buffer += 4, size -= 4;
				LOGPRIO(Logger::Priority::SPAM) << "found HrRd, but hair redirects are not supported anymore.\r\n";
				break;
			case 'TnRd': {
				auto tanName = ReadData<std::wstring>(buffer, size);
				m_styles[m_currCardStyle].m_tanName = tanName;
				LOGPRIO(Logger::Priority::SPAM) << "found TnRd, value " << m_styles[m_currCardStyle].m_tanName << "\r\n";
				break; }
			case 'HrHl': {
				auto hairHighlightName = ReadData<decltype(m_styles[m_currCardStyle].m_hairHighlightName)>(buffer, size);
				m_styles[m_currCardStyle].m_hairHighlightName = hairHighlightName;
				break; }
			case 'OlCl':
				m_styles[m_currCardStyle].m_bOutlineColor = true;
				m_styles[m_currCardStyle].m_outlineColor = ReadData<DWORD>(buffer, size);
				LOGPRIO(Logger::Priority::SPAM) << "found OlCl, value " << m_styles[m_currCardStyle].m_outlineColor << "\r\n";
				break;
			case 'TnCl':
				m_styles[m_currCardStyle].m_bTanColor = true;
				m_styles[m_currCardStyle].m_tanColor = ReadData<DWORD>(buffer, size);
				LOGPRIO(Logger::Priority::SPAM) << "found TnCl, value " << m_styles[m_currCardStyle].m_tanColor << "\r\n";
				break;
			case 'BnTr':
				m_styles[m_currCardStyle].m_boneTransforms = ReadData<decltype(m_styles[m_currCardStyle].m_boneTransforms)>(buffer, size);
				for (const auto& it : m_styles[m_currCardStyle].m_boneTransforms) {
					if (m_styles[m_currCardStyle].m_boneTransformMap.find(it.first) == m_styles[m_currCardStyle].m_boneTransformMap.end()) {
						m_styles[m_currCardStyle].m_boneTransformMap.emplace(it.first, it.second);
					}
				}
				LOGPRIO(Logger::Priority::SPAM) << "...found BnTr, loaded " << m_styles[m_currCardStyle].m_boneTransformMap.size() << " elements.\r\n";
				break;
			case 'HrA0':
				m_styles[m_currCardStyle].m_hairs[0] = ReadData<std::vector<HairPart>>(buffer, size);
				LOGPRIO(Logger::Priority::SPAM) << "found HrA0, loaded " << m_styles[m_currCardStyle].m_hairs[0].size() << " elements\r\n";
				break;
			case 'HrA1':
				m_styles[m_currCardStyle].m_hairs[1] = ReadData<std::vector<HairPart>>(buffer, size);
				LOGPRIO(Logger::Priority::SPAM) << "found HrA1, loaded " << m_styles[m_currCardStyle].m_hairs[1].size() << " elements\r\n";
				break;
			case 'HrA2':
				m_styles[m_currCardStyle].m_hairs[2] = ReadData<std::vector<HairPart>>(buffer, size);
				LOGPRIO(Logger::Priority::SPAM) << "found HrA2, loaded " << m_styles[m_currCardStyle].m_hairs[2].size() << " elements\r\n";
				break;
			case 'HrA3':
				m_styles[m_currCardStyle].m_hairs[3] = ReadData<std::vector<HairPart>>(buffer, size);
				LOGPRIO(Logger::Priority::SPAM) << "found HrA3, loaded " << m_styles[m_currCardStyle].m_hairs[3].size() << " elements\r\n";
				break;
			case 'BnT2':
				m_styles[m_currCardStyle].m_boneRules = ReadData<decltype(m_styles[m_currCardStyle].m_boneRules)>(buffer, size);
				GenBoneRuleMap();
				LOGPRIO(Logger::Priority::SPAM) << "found BnT2, loaded " << m_styles[m_currCardStyle].m_boneRules.size() << " elements\r\n";
				break;
			case 'Slds':
				m_styles[m_currCardStyle].m_sliders = ReadData<decltype(m_styles[m_currCardStyle].m_sliders)>(buffer, size);
				GenSliderMap();
				LOGPRIO(Logger::Priority::SPAM) << "found Slds, loaded " << m_styles[m_currCardStyle].m_sliders.size() << " elements\r\n";
				break;
			case 'File':
				//ret_files[3].fileStart = buffer - 4; //before the 'File'
				m_savedFiles = ReadData<decltype(m_savedFiles)>(buffer, size);
				//ret_files[3].fileEnd = buffer;
				LOGPRIO(Logger::Priority::SPAM) << "found File list, loaded " << m_savedFiles.size() << " elements.\r\n";
				break;
			case 'Blob':
				m_blobInfo = ReadData<decltype(m_blobInfo)>(buffer, size);
				LOGPRIO(Logger::Priority::SPAM) << "found Blob list, loaded " << m_blobInfo.size() << " elements.\r\n";
				break;
			case 'Trgs':
				m_triggers = ReadData<decltype(m_triggers)>(buffer, size);
				LOGPRIO(Logger::Priority::SPAM) << "found Trgs, loaded " << m_triggers.size() << " elements\r\n";
				break;
			case 'TrGv':
				m_globalVars = ReadData<decltype(m_globalVars)>(buffer, size);
				LOGPRIO(Logger::Priority::SPAM) << "found TrGv, loaded " << m_globalVars.size() << " elements\r\n";
				break;
			case 'TrMd':
				m_modules = ReadData<decltype(m_modules)>(buffer, size);
				LOGPRIO(Logger::Priority::SPAM) << "found TrMd, loaded " << m_modules.size() << " elements\r\n";
				break;
			case 'TrAt':
				m_cardStorage = ReadData<decltype(m_cardStorage)>(buffer, size);
				LOGPRIO(Logger::Priority::SPAM) << "found TrAt, loaded " << m_cardStorage.size() << " elements\r\n";
				break;
			}
		}
		catch (InsufficientBufferException e) {
			char idName[5];
			*(DWORD*)(idName) = m_currReadMemberId;
			idName[4] = '\0';
			LOGPRIO(Logger::Priority::WARN) << "Not enough space left to parse member " << idName << "(" << m_currReadMemberId << "); "
				"expected " << e.ExpectedSize() << ", but has " << e.AvailableSize() << "\r\n";
		}
	}

	m_currCardStyle = 0;

	if (size != 0) {
		LOGPRIO(Logger::Priority::WARN) << "size of unlimited card data mismatched; " << size << " bytes were left\r\n";
	}

}


/*
 * Will write the png file to a buffer.
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
		if(m_styles[i].x != g_defaultValues.m_styles[0]. x) { \
			DWORD varId = id; \
			ret &= WriteData(buffer,size,at,varId,resize); \
			ret &= WriteData(buffer,size,at,m_styles[i].x,resize);\
		} else { \
			LOGPRIO(Logger::Priority::SPAM) << 	"... " #x " had default value and was not written\r\n";\
		}
#define DUMP_MEMBER_CONTAINER_AAUSET(id,x) \
		if(!m_styles[i].x.empty()) { \
			DWORD varId = id; \
			ret &= WriteData(buffer,size,at,varId,resize); \
			ret &= WriteData(buffer,size,at,m_styles[i].x,resize);\
		} else { \
			LOGPRIO(Logger::Priority::SPAM) << 	"... " #x " had default value and was not written\r\n";\
		}

int AAUCardData::ToBuffer(char** buffer) {
	int at = 0;
	int ssize = 0;
	int *size = &ssize;
	using namespace Serialize;
	LOGPRIO(Logger::Priority::SPAM) << "dumping card info...\r\n";
	bool ret = true;
	bool resize = true;

	//dumps the given id, followed by the value, but only if its actually different from the default value
	//so, first we would need to write the size. we dont know that yet tho, so we will put in a placeholder
	ret &= General::BufferAppend(buffer, size, at, "Msgn", 4, resize);
	at += 4;
	//write chunk id (big endian)
	ret &= General::BufferAppend(buffer, size, at, (const char*)(&AAUCardData::PngChunkIdBigEndian), 4, resize);
	at += 4;
	//now write all members

	//version
	DUMP_MEMBER('Vers', m_version);
	//tan-slot
	//DUMP_MEMBER('TanS', m_tanSlot);	//deprecated
	//embedded files, deprecated by blobs
	//DUMP_MEMBER_CONTAINER('File', m_savedFiles);
	if (m_blobInfo.size())
		DUMP_MEMBER_CONTAINER('Blob', m_blobInfo);
	//dump aau sets
	for (int i = 0; i < m_styles.size(); i++) {
		DUMP_MEMBER_AAUSET('AUSS', m_name);
		//overrides
		DUMP_MEMBER_CONTAINER_AAUSET('OvrT', m_meshOverrides);
		DUMP_MEMBER_CONTAINER_AAUSET('AOvT', m_archiveOverrides);
		DUMP_MEMBER_CONTAINER_AAUSET('ARdr', m_archiveRedirects);
		DUMP_MEMBER_CONTAINER_AAUSET('OOvr', m_objectOverrides);
		//eye textures
		DUMP_MEMBER_AAUSET('EtLN', m_eyeTextures[0].texName);
		DUMP_MEMBER_AAUSET('EtRN', m_eyeTextures[1].texName);
		//deprecated by blobs
		//DUMP_MEMBER_CONTAINER_AAUSET('EtLF', m_eyeTextures[0].texFile);
		//DUMP_MEMBER_CONTAINER_AAUSET('EtRF', m_eyeTextures[1].texFile);
		//highlight texture
		DUMP_MEMBER_AAUSET('EhXN', m_eyeHighlightName);
		// deprecated by blobs
		//DUMP_MEMBER_CONTAINER_AAUSET('EhXF', m_eyeHighlightFile);
		//tans
		DUMP_MEMBER_AAUSET('TnRd', m_tanName);
		//hair highlight
		DUMP_MEMBER_AAUSET('HrHl', m_hairHighlightName);
		//bone transforms
		DUMP_MEMBER_CONTAINER_AAUSET('BnTr', m_boneTransforms);
		if (m_styles[i].m_bOutlineColor) {
			DUMP_MEMBER_AAUSET('OlCl', m_outlineColor);
		}
		if (m_styles[i].m_bTanColor) {
			DUMP_MEMBER_AAUSET('TnCl', m_tanColor);
		}

		DUMP_MEMBER_CONTAINER_AAUSET('HrA0', m_hairs[0]);
		DUMP_MEMBER_CONTAINER_AAUSET('HrA1', m_hairs[1]);
		DUMP_MEMBER_CONTAINER_AAUSET('HrA2', m_hairs[2]);
		DUMP_MEMBER_CONTAINER_AAUSET('HrA3', m_hairs[3]);

		DUMP_MEMBER_CONTAINER_AAUSET('BnT2', m_boneRules);
		DUMP_MEMBER_CONTAINER_AAUSET('Slds', m_sliders);
		//Card Data Set
		DUMP_MEMBER_CONTAINER_AAUSET('AUDS', m_cardStyleData);
	}

	DUMP_MEMBER_CONTAINER('TrGv', m_globalVars);
	DUMP_MEMBER_CONTAINER('Trgs', m_triggers);
	DUMP_MEMBER_CONTAINER('TrMd', m_modules);
	DUMP_MEMBER_CONTAINER('TrAt', m_cardStorage);


	//now we know the size of the data. its where we are now (at) minus the start of the data (8) (big endian)
	int dataSize = at - 8;
	int dataSizeSwapped = _byteswap_ulong(at - 8);
	ret &= General::BufferAppend(buffer, size, 0, (const char*)(&dataSizeSwapped), 4, resize);

	//write checksum
	DWORD checksum = General::Crc32((BYTE*)(*buffer) + 4, dataSize + 4);
	ret &= General::BufferAppend(buffer, size, at, (const char*)(&checksum), 4, resize);

	return !ret ? 0 : dataSize + 12;
}
//undefine macros again
#undef DUMP_MEMBER_CONTAINER
#undef DUMP_MEMBER
#undef DUMP_MEMBER_CONTAINER_AAUSET
#undef DUMP_MEMBER_AAUSET

/*****************************/
/* Elementwise add functions */
/*****************************/

bool AAUCardData::AddMeshOverride(const TCHAR* texture, const TCHAR* override) {
	if (m_styles[m_currCardStyle].m_meshOverrideMap.find(texture) != m_styles[m_currCardStyle].m_meshOverrideMap.end()) return false;
	TextureImage img(override, TextureImage::OVERRIDE);
	if (img.IsGood()) {
		std::wstring texStr(texture);
		m_styles[m_currCardStyle].m_meshOverrides.emplace_back(texStr, std::wstring(override));
		m_styles[m_currCardStyle].m_meshOverrideMap.emplace(std::move(texStr), std::move(img));
		return true;
	}
	return false;
}

bool AAUCardData::RemoveMeshOverride(int index) {
	if (index < 0 || (size_t)index >= m_styles[m_currCardStyle].m_meshOverrides.size()) return false;
	auto vMatch = m_styles[m_currCardStyle].m_meshOverrides.begin() + index;
	auto mapMatch = m_styles[m_currCardStyle].m_meshOverrideMap.find(vMatch->first);
	m_styles[m_currCardStyle].m_meshOverrides.erase(vMatch);
	if (mapMatch != m_styles[m_currCardStyle].m_meshOverrideMap.end()) m_styles[m_currCardStyle].m_meshOverrideMap.erase(mapMatch);
	return true;
}

bool AAUCardData::AddArchiveOverride(const TCHAR* archive, const TCHAR* archivefile, const TCHAR* override) {
	if (m_styles[m_currCardStyle].m_archiveOverrideMap.find(std::pair<std::wstring, std::wstring>(archive, archivefile)) != m_styles[m_currCardStyle].m_archiveOverrideMap.end()) return false;
	OverrideFile img(override, OverrideFile::OVERRIDE);
	if (img.IsGood()) {
		auto toOverride = std::pair<std::wstring, std::wstring>(archive, archivefile);
		m_styles[m_currCardStyle].m_archiveOverrides.emplace_back(toOverride, override);
		m_styles[m_currCardStyle].m_archiveOverrideMap.emplace(std::move(toOverride), std::move(img));
		return true;
	}
	return false;
}

bool AAUCardData::RemoveArchiveOverride(int index) {
	if (index < 0 || (size_t)index >= m_styles[m_currCardStyle].m_archiveOverrides.size()) return false;
	auto vMatch = m_styles[m_currCardStyle].m_archiveOverrides.begin() + index;
	auto mapMatch = m_styles[m_currCardStyle].m_archiveOverrideMap.find(vMatch->first);
	m_styles[m_currCardStyle].m_archiveOverrides.erase(vMatch);
	if (mapMatch != m_styles[m_currCardStyle].m_archiveOverrideMap.end()) m_styles[m_currCardStyle].m_archiveOverrideMap.erase(mapMatch);
	return true;
}

bool AAUCardData::AddArchiveRedirect(const TCHAR* archive, const TCHAR* archivefile, const TCHAR* redirectarchive, const TCHAR* redirectfile) {
	//here i should check if the archive is valid, but meh
	auto left = std::pair<std::wstring, std::wstring>(archive, archivefile);
	auto right = std::pair<std::wstring, std::wstring>(redirectarchive, redirectfile);
	if (m_styles[m_currCardStyle].m_archiveRedirectMap.find(left) != m_styles[m_currCardStyle].m_archiveRedirectMap.end()) return false; //allready contains it
	m_styles[m_currCardStyle].m_archiveRedirects.emplace_back(left, right);
	m_styles[m_currCardStyle].m_archiveRedirectMap.insert(std::make_pair(left, right));
	return true;
}
bool AAUCardData::RemoveArchiveRedirect(int index) {
	if (index < 0 || (size_t)index >= m_styles[m_currCardStyle].m_archiveRedirects.size()) return false;
	auto vMatch = m_styles[m_currCardStyle].m_archiveRedirects.begin() + index;
	auto mapMatch = m_styles[m_currCardStyle].m_archiveRedirectMap.find(vMatch->first);
	m_styles[m_currCardStyle].m_archiveRedirects.erase(vMatch);
	if (mapMatch != m_styles[m_currCardStyle].m_archiveRedirectMap.end()) m_styles[m_currCardStyle].m_archiveRedirectMap.erase(mapMatch);
	return true;
}

bool AAUCardData::AddObjectOverride(const TCHAR * object, const TCHAR * file) {
	char buff[256];
	size_t n;
	wcstombs_s(&n, buff, object, 256);
	std::string strObject = buff;
	wcstombs_s(&n, buff, file, 256);
	std::string strFile = buff;
	if (m_styles[m_currCardStyle].m_objectOverrideMap.find(strObject) != m_styles[m_currCardStyle].m_objectOverrideMap.end()) return false; //allready contains it
	XXObjectFile ofile(file, XXObjectFile::OVERRIDE);
	if (ofile.IsGood()) {
		m_styles[m_currCardStyle].m_objectOverrides.emplace_back(object, file);
		m_styles[m_currCardStyle].m_objectOverrideMap.insert(std::make_pair(strObject, std::move(ofile)));
	}
	return true;
}

bool AAUCardData::RemoveObjectOverride(int index) {
	if (index < 0 || (size_t)index >= m_styles[m_currCardStyle].m_objectOverrides.size()) return false;
	auto vMatch = m_styles[m_currCardStyle].m_objectOverrides.begin() + index;
	char buff[256];
	size_t n;
	wcstombs_s(&n, buff, vMatch->first.c_str(), 256);
	auto mapMatch = m_styles[m_currCardStyle].m_objectOverrideMap.find(buff);
	m_styles[m_currCardStyle].m_objectOverrides.erase(vMatch);
	if (mapMatch != m_styles[m_currCardStyle].m_objectOverrideMap.end()) m_styles[m_currCardStyle].m_objectOverrideMap.erase(mapMatch);
	return true;
}


bool AAUCardData::AddBoneTransformation(const TCHAR* boneName, D3DMATRIX transform) {
	if (m_styles[m_currCardStyle].m_boneTransformMap.find(boneName) != m_styles[m_currCardStyle].m_boneTransformMap.end()) return false; //allready contains it
	m_styles[m_currCardStyle].m_boneTransforms.emplace_back(boneName, transform);
	m_styles[m_currCardStyle].m_boneTransformMap.insert(std::make_pair(boneName, transform));
	return true;
}
bool AAUCardData::RemoveBoneTransformation(int index) {
	if (index < 0 || (size_t)index >= m_styles[m_currCardStyle].m_boneTransforms.size()) return false;
	auto vMatch = m_styles[m_currCardStyle].m_boneTransforms.begin() + index;
	auto mapMatch = m_styles[m_currCardStyle].m_boneTransformMap.find(vMatch->first);
	m_styles[m_currCardStyle].m_boneTransforms.erase(vMatch);
	if (mapMatch != m_styles[m_currCardStyle].m_boneTransformMap.end()) m_styles[m_currCardStyle].m_boneTransformMap.erase(mapMatch);
	return true;
}

bool AAUCardData::UpdateCardStyle(int set, ExtClass::CharacterData* charData) {
	m_styles[set].m_cardStyleData.CopyCharacterData(charData);
	return true;
}

bool AAUCardData::CopyCardStyle(const TCHAR * name, ExtClass::CharacterData* charData)
{
	for (auto& elem : m_styles) {
		if (elem.m_name == name) return false;
	}
	m_styles.resize(m_styles.size() + 1);
	m_styles[m_styles.size() - 1] = m_styles[GetCurrAAUSet()];
	wcscpy_s(m_styles[m_styles.size() - 1].m_name, name);
	m_styles[m_styles.size() - 1].m_cardStyleData.CopyCharacterData(charData);
	return true;
}
bool AAUCardData::RemoveCardStyle(int index) {
	if (index >= m_styles.size()) return false;
	if (index == 0) return false;
	if (index == m_currCardStyle) { m_currCardStyle = 0; }
	m_styles.erase(m_styles.begin() + index);
	return true;
}
void AAUCardData::SwitchActiveCardStyle(int newSet, ExtClass::CharacterData* charData) {
	if (newSet >= m_styles.size()) return;
	m_currCardStyle = newSet;
	charData->CopyCharacterSetData(&m_styles[m_currCardStyle].m_cardStyleData);
}

int AAUCardData::FindStyleIdxByName(std::wstring* name) {
	for (int i = 0; i < m_styles.size(); i++) {
		if (!name->compare(m_styles[i].m_name)) return i;
	}

	return 0;
}


bool AAUCardData::AddBoneRule(MeshModFlag flags, const TCHAR* xxFileName, const TCHAR* boneName, AAUCardData::BoneMod mod) {
	m_styles[m_currCardStyle].m_boneRules.push_back(std::make_pair(std::make_pair(flags, std::pair<std::wstring, std::wstring>(xxFileName, boneName)), mod));
	if (flags & MODIFY_BONE) {
		auto mapIt = m_styles[m_currCardStyle].m_boneRuleMap.find(xxFileName);
		if (mapIt != m_styles[m_currCardStyle].m_boneRuleMap.end()) {
			auto map2it = mapIt->second.find(boneName);
			if (map2it != mapIt->second.end()) {
				//add mod
				map2it->second.push_back(mod);
			}
			else {
				//new map with mod
				std::vector<BoneMod> vec;
				vec.push_back(mod);
				mapIt->second.emplace(boneName, std::move(vec));
			}
		}
		else {
			std::map<std::wstring, std::vector<BoneMod>> map;
			std::vector<BoneMod> vec;
			vec.push_back(mod);
			map.emplace(boneName, vec);
			m_styles[m_currCardStyle].m_boneRuleMap.emplace(xxFileName, std::move(map));
		}
	}
	if (flags & MODIFY_FRAME) {
		auto mapIt = m_styles[m_currCardStyle].m_frameRuleMap.find(xxFileName);
		if (mapIt != m_styles[m_currCardStyle].m_frameRuleMap.end()) {
			auto map2it = mapIt->second.find(boneName);
			if (map2it != mapIt->second.end()) {
				//add mod
				map2it->second.push_back(mod);
			}
			else {
				//new map with mod
				std::vector<BoneMod> vec;
				vec.push_back(mod);
				mapIt->second.emplace(boneName, std::move(vec));
			}
		}
		else {
			std::map<std::wstring, std::vector<BoneMod>> map;
			std::vector<BoneMod> vec;
			vec.push_back(mod);
			map.emplace(boneName, vec);
			m_styles[m_currCardStyle].m_frameRuleMap.emplace(xxFileName, std::move(map));
		}
	}

	return true;
}

bool AAUCardData::RemoveBoneRule(int index) {
	if (index < 0 || (size_t)index >= m_styles[m_currCardStyle].m_boneRules.size()) return false;
	auto vIt = m_styles[m_currCardStyle].m_boneRules.begin() + index;
	int flags = vIt->first.first;
	if (flags & MODIFY_BONE) {
		auto mapIt = m_styles[m_currCardStyle].m_boneRuleMap.find(vIt->first.second.first);
		std::map<std::wstring, std::vector<BoneMod>>& map = mapIt->second;
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
			m_styles[m_currCardStyle].m_boneRuleMap.erase(mapIt);
		}
	}
	if (flags & MODIFY_FRAME) {
		auto mapIt = m_styles[m_currCardStyle].m_frameRuleMap.find(vIt->first.second.first);
		std::map<std::wstring, std::vector<BoneMod>>& map = mapIt->second;
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
			m_styles[m_currCardStyle].m_frameRuleMap.erase(mapIt);
		}
	}
	m_styles[m_currCardStyle].m_boneRules.erase(vIt);
	return true;
}

void AAUCardData::SetSliderValue(int sliderTarget, int sliderIndex, float value) {
	//save guard
	if (sliderTarget < 0 || sliderTarget >= ExtClass::CharacterStruct::N_MODELS || Shared::g_sliders[sliderTarget].size() <= sliderIndex) {
		LOGPRIO(Logger::Priority::WARN) << "invalid slider id (" << sliderTarget << "|" << sliderIndex << ") set\r\n";
		return;
	}
	//add the value to the slider vector, or remove if value is 0 and its contained
	size_t i;
	for (i = 0; i < m_styles[m_currCardStyle].m_sliders.size(); i++) {
		if (m_styles[m_currCardStyle].m_sliders[i].first.first == sliderTarget && m_styles[m_currCardStyle].m_sliders[i].first.second == sliderIndex) {
			//found it
			if (value == Shared::g_sliders[sliderTarget][sliderIndex].GetNeutralValue()) {
				m_styles[m_currCardStyle].m_sliders.erase(m_styles[m_currCardStyle].m_sliders.begin() + i);
			}
			else {
				m_styles[m_currCardStyle].m_sliders[i].second = value;
			}
			break;
		}
	}
	if (i == m_styles[m_currCardStyle].m_sliders.size() && value != Shared::g_sliders[sliderTarget][sliderIndex].GetNeutralValue()) {
		//didnt find, so we need to add it
		m_styles[m_currCardStyle].m_sliders.push_back(std::make_pair(std::make_pair(sliderTarget, sliderIndex), value));
	}
	GenSliderMap();
}

void AAUCardData::GenAllFileMaps() {
	for (auto &st : m_styles) {
		st.m_meshOverrideMap.clear();
		for (auto &list : st.m_meshOverrides) {
			TextureImage tex(list.second.c_str(), TextureImage::OVERRIDE);
			if (tex.IsGood()) st.m_meshOverrideMap.emplace(list.first, tex);
		}

		st.m_archiveOverrideMap.clear();
		for (auto &list : st.m_archiveOverrides) {
			OverrideFile of(list.second.c_str(), OverrideFile::OVERRIDE);
			if (of.IsGood()) st.m_archiveOverrideMap.emplace(list.first, of);
		}

		st.m_objectOverrideMap.clear();
		for (auto &list : st.m_objectOverrides) {
			XXObjectFile xx(list.second.c_str(), OverrideFile::OVERRIDE);
			if (xx.IsGood()) st.m_objectOverrideMap.emplace(General::utf8.to_bytes(list.first), xx);
		}

		st.m_archiveRedirectMap.clear();
		for (auto &list : st.m_archiveRedirects)
			st.m_archiveRedirectMap.emplace(list.first, list.second);
	}
	for (int i = 0; i < m_styles.size(); i++) {
		SetTan(m_styles[i].m_tanName.c_str(), i);
		SetHairHighlight(m_styles[i].m_hairHighlightName.c_str(), i);
	}
}

void AAUCardData::GenBoneRuleMap() {
	m_styles[m_currCardStyle].m_boneRuleMap.clear();
	for (auto& elem : m_styles[m_currCardStyle].m_boneRules) {
		if (elem.first.first & MODIFY_BONE) {
			auto m = m_styles[m_currCardStyle].m_boneRuleMap.find(elem.first.second.first);
			if (m != m_styles[m_currCardStyle].m_boneRuleMap.end()) {
				std::vector<BoneMod> vec;
				vec.push_back(elem.second);
				m->second.emplace(elem.first.second.second, std::move(vec));
			}
			else {
				std::map<std::wstring, std::vector<BoneMod>> tmp;
				std::vector<BoneMod> vec;
				vec.push_back(elem.second);
				tmp.emplace(elem.first.second.second, std::move(vec));

				m_styles[m_currCardStyle].m_boneRuleMap.emplace(elem.first.second.first, std::move(tmp));
			}
		}
		if (elem.first.first & MODIFY_FRAME) {
			auto m = m_styles[m_currCardStyle].m_frameRuleMap.find(elem.first.second.first);
			if (m != m_styles[m_currCardStyle].m_frameRuleMap.end()) {
				std::vector<BoneMod> vec;
				vec.push_back(elem.second);
				m->second.emplace(elem.first.second.second, std::move(vec));
			}
			else {
				std::map<std::wstring, std::vector<BoneMod>> tmp;
				std::vector<BoneMod> vec;
				vec.push_back(elem.second);
				tmp.emplace(elem.first.second.second, std::move(vec));

				m_styles[m_currCardStyle].m_frameRuleMap.emplace(elem.first.second.first, std::move(tmp));
			}
		}
	}
}
void AAUCardData::GenSliderMap() {
	for (int i = 0; i < ARRAYSIZE(m_styles[m_currCardStyle].m_boneSliderMap); i++) {
		m_styles[m_currCardStyle].m_boneSliderMap[i].clear();
	}
	for (int i = 0; i < ARRAYSIZE(m_styles[m_currCardStyle].m_frameSliderMap); i++) {
		m_styles[m_currCardStyle].m_frameSliderMap[i].clear();
	}

	for (auto elem : m_styles[m_currCardStyle].m_sliders) {
		int target = elem.first.first;
		if (target < 0 || target >= ExtClass::CharacterStruct::N_MODELS || Shared::g_sliders[target].size() <= elem.first.second) {
			LOGPRIO(Logger::Priority::WARN) << "invalid slider id (" << target << "|" << elem.first.second << ") read; the slider was skipped\r\n";
			continue;
		}
		const Shared::Slider& slider = Shared::g_sliders[target][elem.first.second];
		if (slider.flags & MODIFY_BONE) {
			auto& map = m_styles[m_currCardStyle].m_boneSliderMap[target];
			auto it = map.find(slider.boneName);
			if (it != map.end()) {
				BoneMod mod = slider.GenerateModifier(elem.second);
				it->second.push_back(std::make_pair(&slider, mod));
			}
			else {
				std::vector<std::pair<const Shared::Slider*, BoneMod>> vec;
				BoneMod mod = slider.GenerateModifier(elem.second);
				vec.push_back(std::make_pair(&slider, mod));
				map.emplace(slider.boneName, vec);
			}
		}
		if (slider.flags & MODIFY_FRAME) {
			auto& map = m_styles[m_currCardStyle].m_frameSliderMap[target];
			auto it = map.find(slider.boneName);
			if (it != map.end()) {
				BoneMod mod = slider.GenerateModifier(elem.second);
				it->second.push_back(std::make_pair(&slider, mod));
			}
			else {
				std::vector<std::pair<const Shared::Slider*, BoneMod>> vec;
				BoneMod mod = slider.GenerateModifier(elem.second);
				vec.push_back(std::make_pair(&slider, mod));
				map.emplace(slider.boneName, vec);
			}
		}
	}
}


/***************************/
/* Setting loose variables */
/***************************/

bool AAUCardData::SetEyeTexture(int leftright, const TCHAR* texName, bool save) {
	int other = leftright == 0 ? 1 : 0;
	if (texName == NULL) {
		m_styles[m_currCardStyle].m_eyeTextures[leftright].texName = TEXT("");
		return true;
	}
	m_styles[m_currCardStyle].m_eyeTextures[leftright].texName = texName;
	return true;
}

bool AAUCardData::SetEyeHighlight(const TCHAR* texName) {
	if (texName == NULL) {
		m_styles[m_currCardStyle].m_eyeHighlightName = TEXT("");
		return true;
	}
	m_styles[m_currCardStyle].m_eyeHighlightName = texName;
	return true;
}

bool AAUCardData::SetHairHighlight(const TCHAR* name, int style) {
	if (style < 0) style = m_currCardStyle;
	std::wstring path;
	TextureImage::PathStart start;
	path = HAIR_HIGHLIGHT_PATH;
	start = TextureImage::OVERRIDE;
	path += name;
	m_styles[style].m_hairHighlightImage = TextureImage(path.c_str(), start);
	if (m_styles[style].m_hairHighlightImage.IsGood()) {
		m_styles[style].m_hairHighlightName = name;
		return true;
	}
	return false;
}

bool AAUCardData::SetTan(const TCHAR* name, int style) {
	if (style < 0) style = m_currCardStyle;
	//if empty tan name we unset and invalidate the current style tan files
	if (!wcsnlen_s(name, 255)) {
		for (int i = 0; i < 5; i++) {
			m_styles[style].m_tanImages[i] = TextureImage();
		}
		m_styles[style].m_tanName = L"";
		return false;
	}
	std::wstring path;
	TextureImage::PathStart start;
	path = TAN_PATH;
	start = TextureImage::OVERRIDE;
	path += name;
	path += TEXT("\\");
	bool anyGood = false;
	for (int i = 0; i < 5; i++) {
		wchar_t iChar = L'0' + i;
		std::wstring file = TEXT("0");
		file += iChar;
		file += TEXT(".bmp");
		m_styles[style].m_tanImages[i] = TextureImage((path + file).c_str(), start);
		anyGood = anyGood || m_styles[style].m_tanImages[i].IsGood();
	}
	if (anyGood) m_styles[style].m_tanName = name;
	return anyGood;
}

bool AAUCardData::AddHair(BYTE kind, BYTE slot, BYTE adjustment, bool flip) {
	m_styles[m_currCardStyle].m_hairs[kind].push_back({ kind,slot,flip,adjustment });
	return true;
}

//index is extended index for all 4 hair kinds, front, side, back, ext
bool AAUCardData::RemoveHair(int index) {
	int kind;
	for (kind = 0; kind < 4; kind++) {
		if (index < 0 || (size_t)index < m_styles[m_currCardStyle].m_hairs[kind].size()) {
			break;
		}
		index -= m_styles[m_currCardStyle].m_hairs[kind].size();
	}
	if (kind >= 4) return false;

	auto vMatch = m_styles[m_currCardStyle].m_hairs[kind].begin() + index;
	m_styles[m_currCardStyle].m_hairs[kind].erase(vMatch);
	return true;
}

bool AAUCardData::AddModule(const TCHAR* moduleName) {
	ModuleFile modFile(moduleName);
	if (modFile.IsGood()) {
		AddModule(modFile.mod);
		return true;
	}
	return false;
}

bool AAUCardData::AddModule(const Shared::Triggers::Module& mod) {
	bool alwaysAdd = true;	//TODO: remove once action prompt is in place
	bool globalConflict = false;
	//check for globals in this module
	for (auto& global : mod.globals) {
		//search in our globals if we allready have this one
		Shared::Triggers::GlobalVariable* var = NULL;
		for (auto& ourGlobal : m_globalVars) {
			if (ourGlobal.name == global.name) {
				var = &ourGlobal;
				break;
			}
		}
		if (var == NULL) {
			//new var, add
			if (alwaysAdd) {
				m_globalVars.push_back(global);
			}
		}
		else {
			//allready have this var
			globalConflict = true;
		}

	}

	if (!globalConflict && !alwaysAdd) {	//no conflict, add everything
		m_modules.push_back(mod);
		m_globalVars.insert(m_globalVars.end(), mod.globals.begin(), mod.globals.end());
	}
	else {	//some conflicts, ask for aciton
		//not implemented yet
		if (alwaysAdd) m_modules.push_back(mod);
	}

	return true;
}

bool AAUCardData::RemoveModule(int index) {
	if (index < 0 || (size_t)index >= m_modules.size()) {
		return false;
	}
	m_modules.erase(m_modules.begin() + index);
	return true;
}

/********************************/
/* Save and Dump file functions */
/********************************/
void AAUCardData::PrepareSaveBlob() {
	BlobReset();
	int nstyles = m_styles.size();
	for (int i = 0; i < nstyles; i++)
		PrepareSaveBlob(i);
}

void AAUCardData::PrepareSaveBlob(int style) {
	//general overrides first:

	//mesh overrides:
	for (const auto& mrule : m_styles[style].m_meshOverrideMap) {
		std::vector<BYTE> buffer(mrule.second.GetFileSize());
		mrule.second.WriteToBuffer(buffer.data());
		if (buffer.size() > 0) {
			auto path = mrule.second.GetRelPath();
			BlobAppendEntry(path, BLOB_OVERRIDE, buffer.data(), buffer.size());
		}
	}

	//archive overrides
	for (const auto& arule : m_styles[style].m_archiveOverrideMap) {
		std::vector<BYTE> buffer(arule.second.GetFileSize());
		arule.second.WriteToBuffer(buffer.data());
		if (buffer.size() > 0) {
			int location;
			auto path = arule.second.GetRelPath();
			BlobAppendEntry(path, BLOB_OVERRIDE, buffer.data(), buffer.size());
		}
	}

	//object overrides
	for (const auto& orule : m_styles[style].m_objectOverrideMap) {
		std::vector<BYTE> buffer(orule.second.GetFileSize());
		orule.second.WriteToBuffer(buffer.data());
		if (buffer.size() > 0) {
			auto path = orule.second.GetRelPath();
			BlobAppendEntry(path, BLOB_OVERRIDE, buffer.data(), buffer.size());
		}
	}

	//hair highlight
	if (m_styles[style].m_hairHighlightImage.IsGood()) {
		std::vector<BYTE> buffer(m_styles[style].m_hairHighlightImage.GetFileSize());
		m_styles[style].m_hairHighlightImage.WriteToBuffer(buffer.data());
		auto path = m_styles[style].m_hairHighlightImage.GetRelPath();
		BlobAppendEntry(path, BLOB_OVERRIDE, buffer.data(), buffer.size());
	}

	//tan
	for (int i = 0; i < 5; i++) {
		if (m_styles[style].m_tanImages[i].IsGood()) {
			std::vector<BYTE> buffer(m_styles[style].m_tanImages[i].GetFileSize());
			m_styles[style].m_tanImages[i].WriteToBuffer(buffer.data());
			auto path = m_styles[style].m_tanImages[i].GetRelPath();
			BlobAppendEntry(path, BLOB_OVERRIDE, buffer.data(), buffer.size());
		}
	}

	// eye tex
	for (int i = 0; i < 2; i++) {
		std::wstring &texName = m_styles[style].m_eyeTextures[i].texName;
		if (texName.size())
			BlobAppendFile(texName, BLOB_EYE, General::BuildEditPath(TEXT("data\\texture\\eye\\"), texName.c_str()));
	}

	// hilite tex
	std::wstring &texName = m_styles[style].m_eyeHighlightName;
	if (texName.size())
		BlobAppendFile(texName, BLOB_HI, General::BuildEditPath(TEXT("data\\texture\\hilight\\"), texName.c_str()));

}

static bool CheckPath(std::wstring &fullPath) {
	bool suspicious = false;
	for (unsigned int i = 0; i < fullPath.size() - 1; i++) {
		if (fullPath[i] == '.') {
			if (fullPath[i + 1] == '.') {
				suspicious = true;
				break;
			}
			else {
				i++; //skip the second dot as well
			}
		}
	}
	if (suspicious) {
		std::wstringstream warningMessage;
		warningMessage << TEXT("The card contains a file with a suspicious file path:\r\n");
		warningMessage << fullPath << TEXT("This cards files will not be extracted. Blame the guy who made the card");
		MessageBox(NULL, warningMessage.str().c_str(), TEXT("Warning"), MB_ICONWARNING | MB_TASKMODAL);
		return false;
	}
	return true;
}

bool AAUCardData::BlobAppendBuffer(BYTE *buf, size_t len) {
	bool ret = General::BufferAppend(&Blob, &BlobSize, BlobAt, (const char*)buf, len, true);
	BlobAt += len;
	return ret;
}
bool AAUCardData::BlobAppendEntry(std::wstring &name, int typ, BYTE *buf, size_t len) {
	LOGPRIO(Logger::Priority::SPAM) << std::dec << "appending " << name << " of " << len << " bytes, type = " << typ << "\n";
	auto ninfo = std::make_pair(typ, name);
	// already have this entry
	for (auto &b : m_blobInfo) {
		if (b.first == ninfo)
			return true;
	}
	m_blobInfo.emplace_back(std::make_pair(ninfo, len));
	return BlobAppendBuffer(buf, len);
}

// Note that the appended file can come from an archive, too
bool AAUCardData::BlobAppendFile(std::wstring &name, int typ, std::wstring fullPath) {
	DWORD sz;
	BYTE *buf = SharedInjections::ArchiveFile::ReadBuf(fullPath.c_str(), &sz);
	if (buf) BlobAppendEntry(name, typ, buf, sz);
	Shared::IllusionMemFree(buf);
	return buf != NULL;
}

void AAUCardData::BlobReset() {
	m_blobInfo.clear();
	BlobAt = 0;
	BlobSize = 0;
	if (Blob) {
		delete Blob;
		Blob = nullptr;
	}
}


bool AAUCardData::ConvertFilesToBlob() {
	LOGPRIO(Logger::Priority::SPAM) << "converting File to Blob\r\n";

	// override files
	std::vector<std::pair<int, std::wstring>> toExtract;
	for (unsigned int i = 0; i < m_savedFiles.size(); i++) {
		auto& file = m_savedFiles[i];
		int typ = file.first.first;
		if (typ < 2) {
			LOGPRIO(Logger::Priority::WARN) << "could not convert file " << file.first.second.c_str() << ": v1 card embedded files no longer supported\r\n";
			continue;
		}
		BlobAppendEntry(file.first.second, typ, file.second.data(), file.second.size());
		file.second.resize(0);
	}

	//eye textures
	std::pair<std::wstring, std::vector<BYTE>*> eyeStuff[3];
	for (int i = 0; i < 2; i++) {
		if (m_styles[m_currCardStyle].m_eyeTextures[i].texName.size() > 0 && m_styles[m_currCardStyle].m_eyeTextures[i].texFile.size() > 0) {
			std::wstring &texName = m_styles[m_currCardStyle].m_eyeTextures[i].texName;
			auto &texFile = m_styles[m_currCardStyle].m_eyeTextures[i].texFile;

			if (texName.find('\\') != texName.npos)
				return false;

			//texName = L"data\\texture\\eye\\" + texName;

			BlobAppendEntry(texName, BLOB_EYE, texFile.data(), texFile.size());
			texFile.resize(0);
		}
	}

	//eye highlight
	if (m_styles[m_currCardStyle].m_eyeHighlightName.size() > 0 && m_styles[m_currCardStyle].m_eyeHighlightFile.size() > 0) {
		//make sure texture has no folders in it first
		std::wstring &texName = m_styles[m_currCardStyle].m_eyeHighlightName;
		auto &texFile = m_styles[m_currCardStyle].m_eyeHighlightFile;

		if (texName.find('\\') != texName.npos)
			return false;

		//texName = L"data\\texture\\hilight\\" + texName;
		BlobAppendEntry(texName, BLOB_HI, texFile.data(), texFile.size());
		texFile.resize(0);
	}
	LOGPRIO(Logger::Priority::SPAM) << std::dec << "got " << m_blobInfo.size() << " blob entries\r\n";

	return true;
}

bool AAUCardData::PrepareDumpBlob() {
	int need_extract = 0;
	int mode = g_Config.savedFileUsage;
	if (mode == 2) return false; //if 2, do not extract

	// no blob to extract
	if (m_blobInfo.size() == 0) return false;

	// now rewrite the relative paths in the blob to absolute according to file type
	int total = 0;
	for (auto& elem : m_blobInfo) {
		int typ = elem.first.first;
		std::wstring path = L"";

		switch (typ) {
		case BLOB_EYE:
			path = L"data\\texture\\eye\\";
			break;
		case BLOB_HI:
			path = L"data\\texture\\hilight\\";
			break;
		case BLOB_OVERRIDE:
			path = OVERRIDE_PATH;
			break;
		}
		if (path.size()) {
			auto &ep = elem.first.second;
			LOGPRIO(Logger::Priority::SPAM) << std::dec << "dumping " << ep << " of " << elem.second << " bytes, typ=" << typ << "\n";

			// eyes cant have slashes in em
			if ((typ < BLOB_OVERRIDE) && ep.find_first_of(L"\\/") != ep.npos)
				return false;

			path = General::BuildEditPath(path.c_str(), ep.c_str());

			// avoid nasty paths
			if (!CheckPath(path))
				return false;

			// is the file missing?
			if (!General::FileExists(path.c_str())) {
				need_extract++;
				elem.first.second = path;
			}
			// it exists, clear the filename so that it is skipped during extraction
			else {
				elem.first.second = L"";
			}
		}
		total += elem.second;
	}

	// Only for File imports, we don't know blob size yet otherwise
	if ((BlobAt) && (total != BlobAt)) {
		LOGPRIO(Logger::Priority::ERR) << std::dec << "sum of blob entries mismatch blob size, " << total << "!=" << BlobAt << ", aborting extraction.\r\n";
		return false;
	}

	// nothing to do
	if (!need_extract) return false;

	//if mode is 0, build a popup asking for extraction
	if (mode == 0) {
		std::wstringstream text;
		text << "A card wants to extract " << need_extract << " files:\n";
		for (auto& elem : m_blobInfo) {
			auto &fn = elem.first.second;
			if (!fn.size()) continue;
			text << fn << "\n";
		}
		text << "These files are probably required for the card to work properly.\n"
			"Do you want to extract these files now?";
		int res = MessageBox(NULL, text.str().c_str(), TEXT("Info"), MB_YESNO | MB_TASKMODAL);
		if (res != IDYES) {
			//doesnt want to extract, abort
			return false;
		}
	}
	return true;
}


bool AAUCardData::DumpBlob() {
	//create files
	bool success = true;
	int off = 0;
	for (auto& elem : m_blobInfo) {
		auto &fn = elem.first.second;
		int sz = elem.second;
		// only files marked for extraction have names
		if (fn.size()) {
			General::CreatePathForFile(fn.c_str());
			HANDLE file = CreateFile(fn.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, 0, NULL);
			if (file == INVALID_HANDLE_VALUE || file == NULL) {
				int err = GetLastError();
				LOGPRIO(Logger::Priority::WARN) << "could not create file " << fn << ": error " << err << "\r\n";
				success = false;
			}
			else {
				DWORD written = 0;
				WriteFile(file, Blob + off, sz, &written, NULL);
				CloseHandle(file);
				if (written != sz)
					success = false;
			}
		}
		off += sz;
	}

	if (!success) {
		LOGPRIO(Logger::Priority::WARN) << "failed to extract some files from aau card\r\n";
	}

	return success;
}

