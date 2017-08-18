#pragma once

#include <Windows.h>
#include <stdint.h>
#include <thread>
#include <mutex>
#include <shared_mutex>

#include <atomic>
class PP2;
class PP2File {
public:
////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(push, 1)
	// the pp2 file format is as follows

	// Open the file, and do:
	//
	// seek(f.end(), -4)
	// headerlen = f.readu32()
	// seek(f.end(), -4-headerlen);
	// headerblob = f.read(headerlen)
	
	// You end up with header blob which is the following concatenated:
	struct headerInfo {
		uint32_t version;	// currently 2
		// sizes of the 3 tables which just follow after this structure
		uint32_t nchunks;	// number of chunks
		uint32_t nfiles;	// number of files
		uint32_t nnames;	// number of names (name can point to multiple files)
	};

	// first table, chunks[]
	struct chunkInfo {
		uint32_t offset;	// offset in pp2 file
		uint32_t usize;		// uncompressed chunk size, can be -1 to autodetect if ZSTD
		uint32_t firstfile;	// first file of a chunk
		uint32_t nfiles;	// number of files in a chunk
	};
	// Note - chunks don't store compressed size, to get it, do:
	// chunks[idx+1].offset - chunks[idx].offset
	// .pp2 includes a dummy chunk (which actually contains this whole header),
	// so that this always works. Chunks *MUST* be stored always in their offset
	// field order because of this, too.

	// second table, files[]
	struct fileInfo {
		uint32_t chunk;		// chunk this file belongs to
		uint32_t chpos;		// position of file in a chunk (ie offset)
		uint32_t osize;		// decompressed size
		uint32_t flags;		// flags (compression type)
		uint64_t hash;		// first 8 bytes of md5 hash
	};
	const uint32_t FLAG_OPUS = 0x400;
	const uint32_t FLAG_ZSTD = 0x800;
	const uint32_t FLAG_ALONE = 0x1000;
	const uint32_t FLAG_MASK_OPUS = 0x00ff;
	// low byte of flags, flags = flags&0xff, are decoder settings for opus:
	// opus decompressor srate:
	//	  12000 << (flags & 3)
	// output wav srate:
	//    ((flags & 4) ? 12000 : 11025) << (flags & 3)
	// number of channels:
	//    (flags >> 3)&3

	// third table, names[]
	struct nameInfo {
		uint32_t file;		// file this name refers to
		uint16_t namelen;	// byte size of name (no terminating \0)
		wchar_t name[0];	// name[namelen]
	};
	// names must include .pp, and case (the engine is case sensitive)

	///// Compression
	//
	// OPUS files have no header, just sequence of frames, each prepended
	// by uint16 frame length.
	// Configure srate etc by fileInfo.flags, for that particular file, then just:
	// while output.len < fileInfo.osize:
	//     frame = read_uint16(pos, framelen)
	//	   output += opus_decode(frame)
	//     pos += framelen + 2;
	// to get samples. All necessary information to make a proper wav header
	// can be computed from flags & osize field alone, too.
	//
	// OPUS files always reside alone in their own chunk, ie opus imples FLAG_ALONE.
	//
	// Though it is a file flag, ZSTD compression is always applied per chunk,
	// never per file. If you need to compress one file, make a chunk for it
	// (and set FLAG_ALONE). There are no settings for ZSTD, stock ZSTD_decompress is used.
////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct V1fileEntry {
		uint32_t chunk;
		uint32_t off;
		uint32_t osize;
		uint16_t chpos;
		uint16_t flags;
	};
#pragma pack(pop)
	struct chunkInfo *chunks;
	struct fileInfo *files;
	std::map<std::wstring, uint32_t> names;
	uint64_t *md5s;

	/////////////

	struct cacheEntry {
		size_t csize;
		char data[0];
	};

	struct headerInfo hdr;
	HANDLE h;
	PP2 *pp2;
	char *metabuf;

	std::atomic<cacheEntry *> *cache;
	uint32_t*score;

	std::map<uint32_t, void *> acache;
	std::map<uint32_t, uint32_t> ascore;

	char *GameAlloc(size_t sz);

	PP2File(PP2 *pp2, const wchar_t *);
	void *getCache(uint32_t idx);
	uint32_t freeCache(uint32_t idx);
	uint32_t rawfreeCache(PP2File::cacheEntry *ce);
	cacheEntry *rawallocCache(size_t sz);
	cacheEntry *allocCache(uint32_t idx, size_t sz);
	PP2File::cacheEntry *PP2File::reallocCache(PP2File::cacheEntry *ce);
	bool OPUS_decompress(int, int, int nchan, char *dst, size_t dstlen, char *src, size_t srclen);
	uint32_t chunkSize(uint32_t chunk);
	std::wstring name;
	const std::wstring& getName(int idx);

	std::map<uint32_t, char *> buffers;

};


class PP2 {
public:;
	struct workItem {
		PP2File *archive;
		uint32_t chunk;
		char *buf;
	};
	std::ofstream prof;
	std::mutex bufmutex;
	std::atomic<size_t> bufused;
	std::mutex workmutex;
	std::condition_variable work_condition;
	std::vector<std::thread> workers;
	std::queue<workItem> work;

	void compressWorker(workItem);
	bool stopping;
	std::vector<PP2File> pfiles;

	std::atomic<uint32_t> cache_used;
	std::atomic<uint32_t> cache_count;

	uint32_t acache_used;
	HANDLE PP2::HGet();

	PP2();
	~PP2();

	void OOM();
	void Init();
	void GC();
	void CacheGC(size_t);
	void ACacheGC(size_t);

	void AddArchive(const wchar_t *fn);
	void AddPath(const std::wstring &path);

	std::set<std::wstring> pplist;
	std::set<std::wstring> *FList(const wchar_t*);
	bool FExists(const wchar_t*);

	bool ArchiveDecompress(const wchar_t* paramArchive, const wchar_t* paramFile, DWORD* readBytes, BYTE** outBuffer);
	bool LoadFile(std::wstring path, DWORD* readBytes, BYTE** outBuffer);
};

extern PP2 g_PP2;

