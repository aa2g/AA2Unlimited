#include <Windows.h>
#include <stdint.h>
#include <thread>
#include <mutex>
#include "opus.h"

class PP2;

class PP2File {
public:
#pragma pack(push, 1)
	struct fileEntry {
		uint32_t chunk;
		uint32_t off;
		uint32_t osize;
		uint16_t chpos;
		uint16_t flags;
	};
#pragma pack(pop)
	struct cacheEntry {
		size_t csize;
		char data[0];
	};
	HANDLE h;
	PP2 *pp2;
	struct fileEntry *files;
	uint32_t *chunks;
	std::map<std::wstring, uint32_t> names;
	char *metabuf;
	std::map<uint32_t, cacheEntry *> cache;
	std::map<uint32_t, uint32_t> score;

	std::map<uint32_t, void *> acache;
	std::map<uint32_t, uint32_t> ascore;


	PP2File(PP2 *pp2, const wchar_t *);
	void *getCache(uint32_t idx);
	uint32_t freeCache(uint32_t idx);
	cacheEntry *allocCache(uint32_t idx, size_t sz);
	PP2File::cacheEntry *PP2File::reallocCache(uint32_t, PP2File::cacheEntry *ce);
	bool OPUS_decompress(int, int, int nchan, char *dst, size_t dstlen, char *src, size_t srclen);
	uint32_t chunkSize(uint32_t chunk);
	std::wstring name;
	const std::wstring& getName(int idx);
};


class PP2 {
public:;
	std::vector<PP2File> pfiles;
	size_t cache_used;
	size_t cache_count;

	size_t acache_used;
	HANDLE PP2::HGet();

	PP2();
	void CacheGC(size_t);
	void ACacheGC(size_t);

	void AddArchive(const wchar_t *fn);
	void AddPath(const std::wstring &path);

	bool ArchiveDecompress(const wchar_t* paramArchive, const wchar_t* paramFile, DWORD* readBytes, BYTE** outBuffer);
};

extern PP2 g_PP2;