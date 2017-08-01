#include "StdAfx.h"
#include "zstd.h"
#include "opus.h"

#define DBG LOGPRIONC(Logger::Priority::SPAM) std::dec <<

using namespace std;
//const wchar_t *currname;


#define FLAG_OPUS 0x400
#define FLAG_ZSTD 0x800
#define FLAG_ALONE 0x1000
#define INIT_SCORE 0
#define SRATE 44100

PP2 g_PP2;

const std::wstring & PP2File::getName(int idx) {
	for (auto &e : names) {
		if (e.second == idx)
			return e.first;
	}
}
bool PP2File::OPUS_decompress(int srate, int opusrate, int nchan, char *dst, size_t dstlen, char *src, size_t srclen)
{
	OpusDecoder *dec;
	int err;
	dec = opus_decoder_create(opusrate, nchan, &err);

	strcpy(dst, "RIFF");
#define W(x,y) *((uint32_t*)(dst+x)) = y;
	*(uint32_t*)(dst + 4) = dstlen - 8;
	strcpy(dst + 8, "WAVEfmt ");
	W(16, 16);
	W(20, 1);
	W(22, nchan);
	W(24, srate);
	W(28, nchan * 2 * srate);
	W(32, nchan * 2);
	W(34, 16);
	strcpy(dst + 36, "data");
	W(40, dstlen - 44);
#undef W
	size_t pos = 44;
	dstlen -= 44;

	char samples[5760 * 4];
	while (dstlen) {
		uint32_t frmlen = *((uint16_t*)src);
		src += 2;
		int gotsam = opus_decode(dec, (unsigned char*)src, frmlen, (opus_int16*)samples, 5760, 0);
		src += frmlen;
		if (gotsam < 0) {
			return false;
		}
		gotsam *= nchan * 2;
		if (gotsam > dstlen)
			gotsam = dstlen;
		memcpy(dst + pos, samples, gotsam);
		pos += gotsam;
		dstlen -= gotsam;
	}
	return true;
}


PP2File::PP2File(PP2 *_pp2, const wchar_t *fn) : pp2(_pp2) {
	name = wstring(fn);
	FILE *sf = _wfopen(fn, L"rb");
	if (!sf) {
		LOGPRIONC(Logger::Priority::ERR) wstring(fn) << fn << " failed to open: " << strerror(errno) << "\r\n";

	}
	ifstream f(sf);

	f.seekg(-4, f.end);
	int32_t delta;
	f.read((char*)&delta, 4);

	char *buf = new char[delta];
	f.seekg(-4 - delta, f.end);
	f.read(buf, delta);
	int usize = ZSTD_getDecompressedSize(buf, delta);
	metabuf = (char*)malloc(usize);
	int got = ZSTD_decompress(metabuf, usize, buf, delta);
	delete buf;

#define INFOLEN 16
	uint32_t *info = (uint32_t*)metabuf;
	uint32_t version = info[0];
	if (got < 0 || version != 0) {
		LOGPRIONC(Logger::Priority::ERR) wstring(fn) << fn << " appears to be corrupt\r\n";
		return;
	}
	uint32_t chcount = info[1];
	uint32_t fcount = info[2];
	uint32_t ncount = info[3];


	LOGPRIONC(Logger::Priority::SPAM) wstring(fn) << std::dec
		<< " version " << int(version)
		<< ", " << int(chcount) << " chunks, "
		<< int(fcount) << " files, " << ncount << " names" << "\r\n";

	chunks = (uint32_t*)(metabuf + INFOLEN); 
	files = (fileEntry*)(metabuf + INFOLEN + chcount * sizeof(chunks[0]));

	char *p = (char*)&files[fcount];
	size_t metalen = p - metabuf;

	for (uint32_t i = 0; i < ncount; i++) {
		uint32_t *linkto = (uint32_t*)p;
		uint32_t idx = i;
		p += 8;
		uint16_t len = *((uint16_t*)p);
		if (len & 0x8000)
			idx = *linkto;
		len &= 0x7fff;
		p += 2;
		names.emplace(wstring((wchar_t*)p, len / 2), idx);
		p += len;
	}

	// trimp the buffer w/o names
	metabuf = (char*)realloc(metabuf, metalen);
	chunks = (uint32_t*)(metabuf + INFOLEN);
	assert(chunks[0] == 0);
	files = (fileEntry*)(metabuf + INFOLEN + chcount * sizeof(chunks[0]));

	LOGPRIONC(Logger::Priority::SPAM) wstring(fn) << std::dec <<
		" metadata trimmed to " << int(metalen/1024) << "kb\r\n";
	h = HANDLE(_get_osfhandle(_fileno(sf)));
}


uint32_t PP2File::chunkSize(uint32_t chunk) {
	return chunks[chunk + 1] - chunks[chunk];
}

PP2File::cacheEntry *PP2File::reallocCache(uint32_t idx, PP2File::cacheEntry *ce) {
	size_t old = HeapSize(pp2->HGet(), 0, (void*)ce) - sizeof(*ce);
	cacheEntry *nce = (cacheEntry*)HeapReAlloc(pp2->HGet(), 0, (void*)ce, ce->csize + sizeof(*ce));
	pp2->cache_used -= old;
	pp2->cache_used += ce->csize;
	cache[idx] = nce;
	return nce;
}

uint32_t PP2File::freeCache(uint32_t idx)
{
	//DBG "Freecache " << idx << "\r\n";
	assert(cache.find(idx) != cache.end());
	uint32_t sz = cache[idx]->csize;
	HeapFree(pp2->HGet(), 0, cache[idx]);
	pp2->cache_used -= sz;
	pp2->cache_count--;
	cache.erase(idx);
	return sz;
}

static char *GameAlloc(size_t sz) {
	return (char*)Shared::IllusionMemAlloc(sz);
}


PP2File::cacheEntry *PP2File::allocCache(uint32_t idx, size_t size) {
	if ((pp2->cache_used / 1024 / 1024) > g_Config.PP2Cache)
		pp2->CacheGC(pp2->cache_used / 4);

	cacheEntry *ce = (cacheEntry*)HeapAlloc(pp2->HGet(), 0, size + sizeof(*ce));
	if (!ce) {
		pp2->OOM();
		ce = (cacheEntry*)HeapAlloc(pp2->HGet(), 0, size + sizeof(*ce));
	}
	pp2->cache_used += size;
	pp2->cache_count++;
	cache[idx] = ce;
	score[idx]++;
	ce->csize = size;
	return ce;
}

void *PP2File::getCache(uint32_t idx) {
	fileEntry &fe = files[idx];
	cacheEntry *ce = NULL;
	char *ret = NULL;

	if (fe.flags & FLAG_OPUS) {
		ascore[idx]++;
		if (acache.find(idx) != acache.end()) {
			ret = GameAlloc(fe.osize);
			memcpy(ret, acache[idx], fe.osize);
			return ret;
		}
	}

	// if not in cache, put it in there
	if (cache.find(idx) == cache.end()) {
		//DBG "Not found in cache " << idx << "\r\n";
		// trim the cache if needed

		size_t sz = chunkSize(fe.chunk);
		char *zbuf;
		if (fe.flags & FLAG_ALONE) {
			ce = allocCache(idx, sz);
			zbuf = ce->data;
		}
		else {
			zbuf = new char[sz];
		}

		OVERLAPPED over = { 0 };
		over.Offset = chunks[fe.chunk];
		DWORD got = 0;
		bool read_ok = ReadFile(h, zbuf, sz, &got, &over);
		assert(read_ok);
		assert(got == sz);

		if (!ce) {
			assert(!(fe.flags & FLAG_ALONE));
			ret = GameAlloc(fe.osize);
			size_t tmplen = ZSTD_getDecompressedSize(zbuf, sz);
			char *tmp = new char[tmplen];
			assert(fe.flags & FLAG_ZSTD);
			int got = ZSTD_decompress(tmp, tmplen, zbuf, sz);
			assert(got == tmplen);
			delete zbuf;
			uint32_t i = idx - fe.chpos;
			assert(files[i].off == 0);
			uint32_t ichunk = files[i].chunk;
			do {
				if (cache.find(i) == cache.end()) {
					auto &tfe = files[i];
					assert(tfe.chunk == ichunk);
					/*DBG "Recomp " << std::dec
						<< tfe.chunk << ","
						<< tfe.chpos << ","
						<< i << ","
						<< files[i].off << ","
						<< files[i].osize << ","
						<< tmplen << "\r\n";*/

					uint32_t worst = ZSTD_compressBound(tfe.osize);
					auto tce = allocCache(i, worst);
					if (i == idx) {
						memcpy(ret, tmp + tfe.off, tfe.osize);
					}
					assert(tce != NULL);
					assert(tfe.off + tfe.osize <= tmplen);
					tce->csize = ZSTD_compress(tce->data, worst, (void*)(tmp + tfe.off), tfe.osize, 1);
					//DBG i << " compressed to " << tce->csize << " from " << tfe.osize << "\r\n";
					tce = reallocCache(i, tce);
				}
			} while ((files[i].off + files[i++].osize) < tmplen);
			i--;
			assert(files[i].off + files[i].osize == tmplen);
			delete tmp;
		}
	} {
		//DBG "Found in cache " << std::dec << idx << "\r\n";
	}

	// decompressed during load
	if (ret)
		return ret;

	// decompress from cache
	ret = GameAlloc(fe.osize);
	
	if (!ce)
		ce = cache[idx];
	assert(ce);
	score[idx]++;

	if (fe.flags & FLAG_OPUS) {
		int wshift = fe.flags & 3;
		int wavrate = ((fe.flags & 4) ? 12000 : 11025) << wshift;
		int opusrate = 12000 << wshift;
		OPUS_decompress(wavrate, opusrate, (fe.flags>>4)&0xf, ret, fe.osize, ce->data, ce->csize);
		if ((pp2->acache_used / 1024 / 1024) > g_Config.PP2AudioCache)
			pp2->ACacheGC(pp2->acache_used / 4);

		void *aptr = HeapAlloc(pp2->HGet(), 0, fe.osize);
		if (!aptr) {
			pp2->OOM();
			aptr = HeapAlloc(pp2->HGet(), 0, fe.osize);
		}
		acache[idx] = aptr;
		ascore[idx]++;
		memcpy(aptr, ret, fe.osize);
		pp2->acache_used += fe.osize;
		return ret;
	}

	assert(fe.flags & FLAG_ZSTD);
	int got = ZSTD_decompress(ret, fe.osize, ce->data, ce->csize);
	if (got != fe.osize) {
		LOGPRIO(Logger::Priority::CRIT_ERR) << std::dec
			<< "Decompressed size mismatch for "
			<< name << "/" << getName(idx)
			<< "chunk " << fe.chunk << " begins at " << chunks[fe.chunk] << ", chpos " << fe.chpos << " choff " << fe.off
			<< " expected size " << fe.osize << "!=" << got << " from zstd " << ZSTD_getDecompressedSize(ce->data, ce->csize) << "\r\n";
	}

	assert(got == fe.osize);
	return ret;
}


void PP2::AddPath(const wstring &path) {
	WIN32_FIND_DATA fd;
	HANDLE fh;
	if (!g_Config.bUsePP2) {
		LOGPRIO(Logger::Priority::ERR) << "Config not enabled\r\n";
		return;
	}
	LOGPRIONC(Logger::Priority::INFO) "Adding path " << path << "\r\n";

	fh = FindFirstFile((path + L"\\*.pp2").c_str(), &fd);
	if (fh == INVALID_HANDLE_VALUE)
		return;
	do {	
		AddArchive((path + L"\\" + fd.cFileName).c_str());
	} while (FindNextFile(fh, &fd));

}

void PP2::AddArchive(const wchar_t *fn) {
	LOGPRIONC(Logger::Priority::INFO) "Adding .pp2 archive " << wstring(fn) << "\r\n";
	pfiles.emplace_back(this, fn);
}

bool PP2::ArchiveDecompress(const wchar_t* paramArchive, const wchar_t* paramFile, DWORD* readBytes, BYTE** outBuffer) {
	//currname = paramFile;
	for (wchar_t *p = (wchar_t*)paramArchive; *p; p++)
		if (*p == L'\\' || *p == '/')
			paramArchive = p + 1;
	auto path = (wstring(paramArchive) + L"/" + paramFile);
	//DBG "@@@loading " << path << "\r\n";

	transform(path.begin(), path.end(), path.begin(), ::tolower);

	for (auto &p : pfiles) {
		if (p.names.find(path) == p.names.end())
			continue;
		auto fidx = p.names[path];
		if (fidx == 0xffffffff) {
			*readBytes = 0;
			*outBuffer = (BYTE*)GameAlloc(0);
			return true;
		}
		*readBytes = p.files[fidx].osize;
		*outBuffer = (BYTE*)p.getCache(fidx);
		
		return true;
	}

	return false;
}

PP2::PP2() {};
HANDLE PP2::HGet() {
	if (!*Shared::IllusionMemAllocHeap)
		*Shared::IllusionMemAllocHeap = HeapCreate(0, 0, 0);
	return *Shared::IllusionMemAllocHeap;
}

void PP2::OOM() {
	LOGPRIONC(Logger::Priority::CRIT_ERR) std::dec
		<< "OOM: out of memory, trying emergency GC but this is bad\r\n";

	CacheGC(-1);
	ACacheGC(-1);
	g_Config.PP2Cache /= 2;
	g_Config.PP2AudioCache /= 2;
}

void PP2::CacheGC(size_t sz) {
	vector<uint64_t> array;
	int idx = 0;
	long tsize = 0;

	LOGPRIONC(Logger::Priority::SPAM) std::dec
		<< "CacheGC: need to free " << sz/1024 << " KiB\r\n";

	for (auto &p : pfiles) {
		for (auto const& e : p.score) {
			if (p.cache.find(e.first) == p.cache.end())
				continue;
			tsize += p.cache[e.first]->csize;
			uint64_t v = ((uint64_t)e.second << 32) | ((uint64_t)e.first << 8) | idx;
			array.push_back(v);
		}
		idx++;
	}
	sort(array.begin(), array.end());

	LOGPRIONC(Logger::Priority::SPAM) std::dec
		<< "CacheGC: usage " << array.size() << " entries, counted size is " << tsize / 1024 << "KiB" << ", accounted usage is " << cache_used / 1024 << "KiB in " << cache_count << " entries\r\n";

	size_t dropped = 0;
	int fhit, lhit;
	fhit = -1;
	int nent = 0;
	for (auto &a : array) {
		if (fhit < 0)
			fhit = a >> 32;
		lhit = a >> 32;
		dropped += pfiles[a&0xff].freeCache((a>>8) & 0xffffff);
		nent++;
		if (dropped > sz)
			break;
	}
	LOGPRIONC(Logger::Priority::SPAM)
		"CacheGC: Freed " << nent << " compressed cache entries, " << dropped / 1024 << "KiB, hits" <<fhit<<","<<lhit<<"\r\n";
}

void PP2::ACacheGC(size_t sz) {
	vector<uint64_t> array;
	int idx = 0;
	long tsize = 0;

	for (auto &p : pfiles) {
		for (auto const& e : p.ascore) {
			if (p.acache.find(e.first) == p.acache.end())
				continue;
			tsize += p.files[e.first].osize;
			uint64_t v = ((uint64_t)e.second << 32) | ((uint64_t)e.first << 8) | idx;
			array.push_back(v);
		}
		idx++;
	}
	sort(array.begin(), array.end());

	LOGPRIONC(Logger::Priority::SPAM) std::dec
		<< "ACacheGC: usage " << array.size() << " entries, counted size is " << tsize / 1024 << "KiB" << ", accounted usage is " << acache_used / 1024 << "KIB\r\n";

	int dropped = 0;
	int nent = 0;
	for (auto &a : array) {
		uint32_t entry = (a >> 8) & 0xffffff;
		auto &ac = pfiles[a & 0xff].acache;
		assert(ac[entry] != NULL);
		HeapFree(HGet(), 0, ac[entry]);
		ac.erase(entry);
		int esz = pfiles[a & 0xff].files[entry].osize;
		dropped += esz;
		nent++;
		acache_used -= esz;
		if (dropped > sz)
			break;
	}
	LOGPRIONC(Logger::Priority::SPAM)
		"ACacheGC: Freed " << nent << " uncompressed audio cache entries, " << dropped / 1024 << "KiB\r\n";
}
