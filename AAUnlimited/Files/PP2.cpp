#include "StdAfx.h"
#include "zstd.h"
#include "opus.h"
#include <io.h>
#include <windows.h>
#include <Shlwapi.h>
#include "General/ModuleInfo.h"


#define DBG LOGPRIONC(Logger::Priority::SPAM) dec <<

using namespace std;

PP2 g_PP2;

const wstring & PP2File::getName(int idx) {
	for (auto &e : names) {
		if (e.second == idx)
			return e.first;
	}
	abort();
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

	char *p;
	headerInfo *h = (headerInfo *)metabuf;
	hdr = *h;
	LOGPRIONC(Logger::Priority::SPAM) wstring(fn) << dec
		<< " version " << int(h->version)
		<< ", " << int(h->nchunks) << " chunks, "
		<< int(h->nfiles) << " files, " << h->nnames << " names" << "\r\n";


	if (h->version == 0) {
		chunks = new chunkInfo[h->nchunks]();
		files = new fileInfo[h->nfiles]();
		uint32_t *offs = (uint32_t*)(metabuf + sizeof(headerInfo));
		// NOTE: last chunk is dummy one
		for (int i = 0; i < h->nchunks; i++) {
			chunks[i].offset = offs[i];
			chunks[i].usize = -1;
			chunks[i].firstfile = 0xdeadbabe;
			/*
			chunks[i].usize = 0;
			chunks[i].nfiles = 0;
			chunks[i].firstfile = 0;*/
		}
		V1fileEntry *v1f = (V1fileEntry*)(&offs[h->nchunks]);
		for (int i = 0; i < h->nfiles; i++) {
			files[i].chpos = v1f[i].off;
			files[i].chunk = v1f[i].chunk;
			files[i].flags = v1f[i].flags;
			files[i].osize = v1f[i].osize;

			if (v1f[i].off == 0)
				chunks[v1f[i].chunk].firstfile = i;
		}
		for (int i = 0; i < h->nchunks-1; i++) {
			assert(chunks[i].firstfile < h->nfiles);
		}
		p = (char*)(&v1f[h->nfiles]);
	}
	else if (h->version == 2) {
		chunks = decltype(chunks)(metabuf + sizeof(headerInfo));
		files = decltype(files)(metabuf + sizeof(headerInfo) + sizeof(chunkInfo) * h->nchunks);
		p = (char*)(&files[h->nfiles]);
	}
	else {
		LOGPRIONC(Logger::Priority::ERR) wstring(fn) << fn << " appears to be corrupt\r\n";
		return;
	}

//	md5s = new uint64_t[h->nfiles];
	size_t metalen = p - metabuf;

	for (uint32_t i = 0; i < h->nnames; i++) {
		uint32_t fidx;
		if (h->version == 0) {
			if (i < h->nfiles) {
				fidx = i;
				files[i].hash = *((uint64_t*)p);
			} 
			else
				fidx = *((uint32_t*)p);
			p += 8;
		}
		else {
			fidx = *((uint32_t*)p);
			p += 4;
		}
		uint16_t len = *((uint16_t*)p);

		len &= 0x7fff;
		p += 2;
		wstring tfn((wchar_t*)p, len / 2);
		wstring low(tfn);

		// the letter cases for pp names must remain canonical
		tfn.resize(tfn.find_first_of(L"/"));
		pp2->pplist.insert(tfn);

		// alternative: only if the path is a pp file, lowercase it
//		if (tfn.rfind(L".pp")+3 == tfn.size())

		// but the actual pathnmaes are lowercase
		transform(low.begin(), low.end(), low.begin(), ::tolower);
		names.emplace(low, fidx);

//		SharedInjections::WinAPI::RegisterPP(tfn.c_str());
		p += len;
	}


	cache = new atomic<cacheEntry *>[h->nfiles]();
	score = new uint32_t[h->nfiles]();

	// trimp the buffer w/o names
	if (h->version == 2)
		metabuf = (char*)realloc(metabuf, p - metabuf);
	else {
		free(metabuf);
		metabuf = NULL;
	}

	LOGPRIONC(Logger::Priority::SPAM) wstring(fn) << dec <<
		" metadata size " << int(metalen/1024) << "kb\r\n";
	this->h = HANDLE(_get_osfhandle(_fileno(sf)));
}


int PP2File::chunkSize(uint32_t chunk) {
	return chunks[chunk + 1].offset - chunks[chunk].offset;
}

PP2File::cacheEntry *PP2File::reallocCache(PP2File::cacheEntry *ce) {
	size_t old = HeapSize(pp2->HGet(), 0, (void*)ce) - sizeof(*ce);
	cacheEntry *nce = (cacheEntry*)HeapReAlloc(pp2->HGet(), 0, (void*)ce, ce->csize + sizeof(*ce));
	return nce;
}

uint32_t PP2File::rawfreeCache(cacheEntry *ce)
{
	uint32_t freed = ce->csize;
	pp2->cache_used -= freed;
	pp2->cache_count--;
	if (!HeapFree(pp2->HGet(), 0, ce)) {
		LOGPRIO(Logger::Priority::CRIT_ERR) << "Memleak " << ce->csize << " bytes! HeapFree failed for " << ce << "\n";
	}
	return freed;
}

uint32_t PP2File::freeCache(uint32_t idx)
{
	assert(idx < hdr.nfiles);
	cacheEntry *ce = cache[idx].exchange(0);
	if (!ce) {
		// The entry is already free
		return 0;
	}
	return rawfreeCache(ce);
}

char *PP2File::GameAlloc(size_t sz) {
	return (char*)Shared::IllusionMemAlloc(sz);
}

PP2File::cacheEntry *PP2File::rawallocCache(size_t size) {
	auto ce = (cacheEntry*)HeapAlloc(pp2->HGet(), 0, size + sizeof(cacheEntry));
	ce->csize = size;
	return ce;
}
PP2File::cacheEntry *PP2File::allocCache(uint32_t idx, size_t size) {
	auto ce = rawallocCache(size);
	pp2->cache_used += size;
	pp2->cache_count++;
	assert(!cache[idx].load());
	cache[idx].store(ce);
	return ce;
}

void PP2::compressWorker(workItem wi) {
	PP2File *arch = wi.archive;
	assert(wi.chunk < arch->hdr.nchunks);
	uint32_t fidx = arch->chunks[wi.chunk].firstfile;
	assert(fidx < arch->hdr.nfiles);

	// walk the files in a chunk
	for (uint32_t i = fidx; arch->files[i].chunk == wi.chunk; i++) {
		auto &tfe = arch->files[i];
		// this particular file is already cached
		if (arch->cache[i].load())
			continue;

		uint32_t worst = ZSTD_compressBound(tfe.osize);
		PP2File::cacheEntry *nce, *tce = arch->rawallocCache(worst);
		int got = ZSTD_compress(tce->data, worst, wi.buf + tfe.chpos, tfe.osize, 3);

		assert(got >= 0);
		tce->csize = got;

		nce = arch->reallocCache(tce);

		assert(!arch->cache[i].load());
		arch->cache[i].store(nce);

		cache_used += nce->csize;
		cache_count++;
	}
	bufmutex.lock();
	bufused -= arch->chunks[wi.chunk].usize;
	assert(arch->buffers[wi.chunk] == wi.buf);
	int ndel = arch->buffers.erase(wi.chunk);
	assert(ndel == 1);
	delete wi.buf;
	bufmutex.unlock();
}

void *PP2File::getCache(uint32_t idx, size_t *cachedsize) {
	bool buflocked = false;
	fileInfo &fe = files[idx];
	cacheEntry *ce = NULL;
	char *ret = NULL;

	if ((!cachedsize) && (fe.flags & FLAG_OPUS)) {
		ascore[idx]++;
		if (acache.find(idx) != acache.end()) {
			ret = GameAlloc(fe.osize);
			memcpy(ret, acache[idx], fe.osize);
			return ret;
		}
	}

	score[idx]++;

	ce = cache[idx].load();

	// if not in cache, put it in there
	while (!ce) {
		pp2->GC();

		chunkInfo &ch = chunks[fe.chunk];
		// if we have a work buffer, just use that, but don't
		// put it into cache just yet
		{
			unique_lock<mutex> lock(pp2->bufmutex);
			if (buffers.find(fe.chunk) != buffers.end()) {
				assert(!(fe.flags & FLAG_OPUS));
				ret = GameAlloc(fe.osize);
				char *buf = buffers[fe.chunk];
				memcpy(ret, buf + fe.chpos, fe.osize);
				if (cachedsize)
					*cachedsize = fe.osize;

				return ret;
			}
		}

		// compressed chunk size
		size_t sz = chunkSize(fe.chunk);
		char *zbuf;

		// if the file is alone in chunk, the compressed chunk itself
		// becomes a cache entry
		if (fe.flags & FLAG_ALONE) {
			ce = allocCache(idx, sz); // cache[idx] = ce now
			zbuf = ce->data;
		}
		else {
			// otherwise allocate special buffer
			zbuf = new char[sz];
		}

		// now read in the chunk
		OVERLAPPED over = { 0 };
		over.Offset = ch.offset;
		DWORD rgot = 0;
		bool read_ok = ReadFile(h, zbuf, sz, &rgot, &over);
		assert(read_ok);
		assert(rgot == sz);

		// if its unique snowflake (FLAG_ALONE), we're done here
		if (ce)
			break;

		assert(!(fe.flags & FLAG_ALONE));

		// otherwise decompress the chunk and pull
		// the file entry we need
		ret = GameAlloc(fe.osize);
		size_t tmplen = (ch.usize!=-1)?ch.usize:ZSTD_getDecompressedSize(zbuf, sz);
		ch.usize = tmplen;

		char *tmp = new char[tmplen];
		assert(fe.flags & FLAG_ZSTD);
		int got = ZSTD_decompress(tmp, tmplen, zbuf, sz);
		assert(got == tmplen);
		delete zbuf;

		// copy the data
		memcpy(ret, tmp + fe.chpos, fe.osize);
		if (cachedsize)
			*cachedsize = fe.osize;

		// now kick the worker thread to start recompressing
		// the entries again

		// If too much of buffers is in use, do not cache this,
		// we'll try again at some other time
		pp2->bufmutex.lock();
		if ((pp2->bufused/1024/1024) > (g_Config.PP2Buffers)) {
			delete tmp;
			LOGPRIO(Logger::Priority::WARN) << dec
				<< "Cache buffer trashing (used " << (pp2->bufused/1024) << "KiB) index " << idx << "\r\n";
			pp2->bufmutex.unlock();
			return ret;
		}
		buffers[fe.chunk] = tmp;
		pp2->bufused += tmplen;
		pp2->bufmutex.unlock();

		{
			unique_lock<mutex> lock(pp2->workmutex);
//			pp2->compressWorker({ this, fe.chunk, tmp });
			pp2->work.push({ this, fe.chunk, tmp });
		}
		pp2->work_condition.notify_one();
		return ret;
	}

	// Allocate result
	ret = GameAlloc(fe.osize);

	// Make sure we have something to fill it with
	assert(ce);

	if (fe.flags & FLAG_OPUS) {
		if (cachedsize) {
			ret = ce->data;
			*cachedsize = ce->csize;
			return ret;
		}
		int wshift = fe.flags & 3;
		int wavrate = ((fe.flags & 4) ? 12000 : 11025) << wshift;
		int opusrate = 12000 << wshift;
		OPUS_decompress(wavrate, opusrate, (fe.flags>>4)&0xf, ret, fe.osize, ce->data, ce->csize);
		if ((pp2->acache_used / 1024 / 1024) > g_Config.PP2AudioCache) {
			pp2->ACacheGC(pp2->acache_used / 4);
			MemAlloc::dumpheap();
		}

		void *aptr = HeapAlloc(pp2->HGet(), 0, fe.osize);
		if (!aptr) {
			pp2->OOM();
			aptr = HeapAlloc(pp2->HGet(), 0, fe.osize);
		}
		assert(!acache[idx]);
		acache[idx] = aptr;
		ascore[idx]++;
		memcpy(aptr, ret, fe.osize);
		pp2->acache_used += fe.osize;
		return ret;
	}

	// TODO: non-zstd files
	assert(fe.flags & FLAG_ZSTD);
//	pp2->gc_mutex.lock();
	int got = ZSTD_decompress(ret, fe.osize, ce->data, ce->csize);
//	pp2->gc_mutex.unlock();
	if (got != fe.osize) {
		LOGPRIO(Logger::Priority::CRIT_ERR) << dec
			<< "Decompressed size mismatch for "
			<< name << "/" << getName(idx)
			<< "chunk " << fe.chunk << " begins at " << chunks[fe.chunk].offset << ", chpos " << fe.chpos 
			<< " expected size " << fe.osize << "!=" << got << " from zstd " << ZSTD_getDecompressedSize(ce->data, ce->csize) << "\r\n";
	}

	if (cachedsize)
		*cachedsize = fe.osize;
	assert(got == fe.osize);
	return ret;
}

static bool is_pp_path(const wchar_t *path) {
	int pplen = wcslen(path);
	if (pplen < 5)
		return false;
	return !wcscmp(path + pplen - 4, L"*.pp");
}

static bool strip_data_path(wstring &path) {
	replace(path.begin(), path.end(), '\\', '/');

	size_t pos = path.rfind(L"/data/");
	if (pos == path.npos)
		return false;

	path = path.substr(pos + 6);
	return true;
}

bool PP2::FExists(const wchar_t *path) {
	if (!g_Config.bUsePP2)
		return false;

	wstring pa(path);
	transform(pa.begin(), pa.end(), pa.begin(), ::tolower);

	if (!strip_data_path(pa))
		return false;

	for (auto &pp : pfiles) {
		if (pp.names.find(pa) != pp.names.end())
			return true;
	}
	return false;
}


set<wstring> *PP2::FList(const wchar_t *path) {
	if (!g_Config.bUsePP2)
		return 0;
	if (is_pp_path(path))
		return &pplist;

	wstring mask(path);
	if (!strip_data_path(mask))
		return 0;

	static map<wstring,set<wstring> *> cache;

	// too wild
	if (mask[0] == '*')
		return 0;
	if (mask.size() < 4)
		return 0;
	if (mask.substr(0, 4) == L"save")
		return 0;
	if (cache[mask])
		return cache[mask];
	auto thelist = new set<wstring>();
	cache[mask] = thelist;

	// strip the search mask
	for (auto &pp : pfiles) {
		for (auto &e : pp.names) {
			if (PathMatchSpec(e.first.c_str(), mask.c_str())) {
				// respond only with last component
				wstring got(e.first.substr(e.first.find_last_of('/') + 1));
				thelist->insert(got);
			}
		}
	}
	if (thelist->size() == 0)
		return 0;

	LOGPRIONC(Logger::Priority::SPAM) dec
		<< "Produced dirlist of " << thelist->size() << " items\n";

	return thelist;
}

void PP2::AddPath(const wstring &path) {
	WIN32_FIND_DATA fd;
	HANDLE fh;
	if (!g_Config.bUsePP2) {
		LOGPRIO(Logger::Priority::ERR) << "Config not enabled\r\n";
		return;
	}
	LOGPRIONC(Logger::Priority::INFO) "PP2 adding search path " << path << "\\*.pp2\r\n";

	fh = FindFirstFile((path + L"\\*.pp2").c_str(), &fd);
	if (fh == INVALID_HANDLE_VALUE)
		return;
	do {	
		AddArchive((path + L"\\" + fd.cFileName).c_str());
	} while (FindNextFile(fh, &fd));

}


void PP2::AddArchive(const wchar_t *fn) {
	wchar_t buf[1024];
	GetFullPathName(fn, 1024, buf, NULL);
	wstring wfn(buf);

	LOGPRIONC(Logger::Priority::INFO) "Adding .pp2 archive " << wfn << "\r\n";
	for (auto &it : pfiles) {
		if (it.name == wfn) {
			LOGPRIONC(Logger::Priority::WARN) wfn << "is already loaded\r\n";
			return;
		}
	}
	pfiles.emplace_back(this, buf);
}

void PP2::bindLua() {
	LUA_SCOPE;
	auto _BINDING = g_Lua[LUA_BINDING_TABLE];
	_BINDING["PP2AddPath"] = GLua::Function([](auto &s) {
		const char *str = s.get(1);
		std::wstring fn = General::utf8.from_bytes(str);
		g_PP2.AddPath(fn);
		return 0;
	});
	_BINDING["PP2List"] = GLua::Function([](auto &s) {
		s.top(0);
		for (auto it : g_PP2.pfiles)
			s.push(General::to_utf8(it.name));	
		return s.top();
	});
	_BINDING["PP2GetFiles"] = GLua::Function([](auto &s) {
		int i = s.get(1);
		if (i >= g_PP2.pfiles.size()) return 0;

		auto &f = g_PP2.pfiles[i];
		auto t = s.newtable();
		// maps hash to { index, flags, osize, csize }
		const void *chk = lua_topointer(s.L(), 2);
		for (int i = 0; i < f.hdr.nfiles; i++) {
			LUA_SCOPE;
			auto &fe = f.files[i];
			auto tf = s.newtable();
			tf["index"] = i;
			tf["flags"] = fe.flags;
			tf["osize"] = fe.osize;
			tf["csize"] = (fe.flags&f.FLAG_OPUS) ? f.chunkSize(fe.chunk) : -1;
			auto hv = s.pushlstring((const char *)(&f.files[i].hash), 8).get();
			t[hv] = tf;
		}
		if (chk != lua_topointer(s.L(), 2))
			__debugbreak();
		return 1;
	});

	_BINDING["PP2ReadFile"] = GLua::Function([](auto &s) {
		int i = s.get(1);
		if (i >= g_PP2.pfiles.size()) return 0;
		auto &pf = g_PP2.pfiles[i];
		i = s.get(2);
		size_t osz = 0;
		void *buf = pf.getCache(i, &osz);
		if (!buf) return 0;
		s.pushlstring((const char*)buf, osz);
		Shared::IllusionMemFree(buf);
		return 1;
	});

	_BINDING["PP2GetNames"] = GLua::Function([](auto &s) {
		int i = s.get(1);
		if (i >= g_PP2.pfiles.size()) return 0;
		auto &f = g_PP2.pfiles[i];
		auto t = s.newtable();
		// maps name to index
		for (auto &it : f.names) {
			LUA_SCOPE;
			const char *nm = General::to_utf8(it.first);
			t[nm] = it.second;
		}
		return 1;
	});

}

bool PP2::LoadFile(wstring path, DWORD* readBytes, BYTE** outBuffer) {
	for (auto &&p : pfiles) {
		if (p.names.find(path) == p.names.end())
			continue;
		auto fidx = p.names[path];
		if (fidx == 0xffffffff) {
			*readBytes = 0;
			*outBuffer = (BYTE*)p.GameAlloc(0);
			return true;
		}
		*readBytes = p.files[fidx].osize;
		*outBuffer = (BYTE*)p.getCache(fidx, NULL);
		if (g_Config.PP2Profiling) {
			prof.write((char*)(&p.files[fidx].hash), 8);
			prof.write((char*)(&GameTick::tick), 4);
		}
#if 0
		wstring outf(L"out/");
		FILE *fo = _wfopen((outf + paramArchive + L"_" + paramFile).c_str(), L"wb");
		fwrite(*outBuffer, 1, *readBytes, fo);
		fclose(fo);
#endif
		return true;
	}
	return false;
}

bool PP2::ArchiveDecompress(const wchar_t* paramArchive, const wchar_t* paramFile, DWORD* readBytes, BYTE** outBuffer) {
	wstring path;
	int ppos = 0;

	if (paramArchive && paramArchive[0]) {
		//currname = paramFile;
		for (wchar_t *p = (wchar_t*)paramArchive; *p; p++)
			if (*p == L'\\' || *p == '/')
				paramArchive = p + 1;
		path = wstring(paramArchive) + L"/";
		ppos = path.length();
		path += paramFile;
	}
	else {
		path = paramFile;
		if (!strip_data_path(path))
			return false;
	}
	transform(path.begin(), path.end(), path.begin(), ::tolower);

	if (LoadFile(path, readBytes, outBuffer))
		return true;

	return false;
}

PP2::PP2() {};

PP2::~PP2() {
	if (!g_Config.bUsePP2)
		return;
	// WIN7 bug: work_condition.notify_all() will trash 'this' for reasons not yet clear.
#if 1
	// brute workaround
	ExitProcess(0);
#else
	{
		unique_lock<mutex> lock(workmutex);
		stopping = true;
	}
	work_condition.notify_all();
	for (auto &w : workers)
		w.join();
#endif
};

void PP2::Init() {
	if (!g_Config.bUsePP2)
		return;

	MemAlloc::dumpheap();
	int nthreads = thread::hardware_concurrency();
	if (!nthreads)
		nthreads = 4;
	for (int i = 0; i < nthreads; i++) {
		workers.emplace_back([this] {
			for (;;) {
				workItem wi;
				{
					unique_lock<mutex> lock(workmutex);
					work_condition.wait(lock, [this] {return this->stopping || !this->work.empty(); });
					if (this->stopping && this->work.empty())
						return;
					wi = work.front();
					work.pop();
				}
				compressWorker(wi);
			}
		});
	}
	bindLua();
}

void PP2::InitProfiling() {
	if (g_Config.bUsePP2 && g_Config.PP2Profiling) {
		std::string path(General::to_utf8(General::BuildAAUPath(L"pp2.prof")));
		prof.open(path, prof.ate | prof.out | prof.in | prof.binary);
		if (!prof.is_open()) {
			prof.open(path, prof.ate | prof.out | prof.in | prof.binary | prof.trunc);
		}
		// start writing on 12 byte boundary
		int pos = prof.tellp();
		prof.seekp(pos - (pos % 12));
		// start game marker
		LOGPRIONC(Logger::Priority::INFO) dec
			<< "PP2Prof: seeking to " << pos << "\r\n";

		prof.write("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12);
	}
}


HANDLE PP2::HGet() {
	return (HANDLE)_get_heap_handle();
}

void  PP2::GC() {
	bool ret = false;
	if (((cache_used / 1024 / 1024 + *Shared::IllusionMemUsed / 1024 / 1024 + bufused / 1024 / 1024)) > g_Config.PP2Cache) {
		CacheGC(cache_used / 4);
		ret = 1;
	}
	if ((acache_used / 1024 / 1024) > g_Config.PP2AudioCache) {
		ACacheGC(acache_used / 4);
		ret = 1;
	}
	if (ret || ((GameTick::now % 10000) == 1))
		MemAlloc::dumpheap();
}

void PP2::OOM() {
	LOGPRIONC(Logger::Priority::CRIT_ERR) dec
		<< "OOM: out of memory, trying emergency GC but this is bad\r\n";

	CacheGC(-1);
	ACacheGC(-1);
	g_Config.PP2Cache /= 2;
	g_Config.PP2AudioCache /= 2;
}


static void defrag_heap() {
	ULONG info[2] = { 1, 0 };
	HeapSetInformation(NULL, (HEAP_INFORMATION_CLASS)3, info, 8);
}

void PP2::CacheGC(size_t sz) {
//	unique_lock<shared_mutex> lock(gc_mutex);

	vector<uint64_t> array;
	int idx = 0;
	long tsize = 0;


	defrag_heap();
	LOGPRIONC(Logger::Priority::SPAM) dec
		<< "CacheGC: need to free " << sz/1024 << " KiB\r\n";

	for (auto &&p : pfiles) {
		for (int i = 0; i < p.hdr.nfiles; i++) {
			auto ce = p.cache[i].load();
			if (!ce)
				continue;
			tsize += ce->csize;
			uint64_t v = ((uint64_t)p.score[i] << 32) | ((uint64_t)i << 8) | idx;
			array.push_back(v);
		}
		// global pp2 index
		idx++;
	}
	sort(array.begin(), array.end());

	LOGPRIONC(Logger::Priority::SPAM) dec
		<< "CacheGC: usage " << array.size() << " entries, counted size is " << tsize / 1024 << "KiB" << ", accounted usage is " << cache_used / 1024 << "KiB in " << cache_count << " entries\r\n";

	size_t dropped = 0;
	int fhit, lhit;
	fhit = -1;
	int nent = 0;

	for (auto &a : array) {
		int pfile = a & 0xff;
		if (fhit < 0)
			fhit = a >> 32;
		lhit = a >> 32;
		dropped += pfiles[pfile].freeCache((a>>8) & 0xffffff);
		nent++;
		if (dropped > sz)
			break;
	}
	defrag_heap();
	LOGPRIONC(Logger::Priority::SPAM)
		"CacheGC: Freed " << nent << " compressed cache entries, " << dropped / 1024 << "KiB, illusion cache " << (*Shared::IllusionMemUsed/1024) << "KiB\r\n";
}

void PP2::ACacheGC(size_t sz) {
	//unique_lock<shared_mutex> lock(gc_mutex);

	vector<uint64_t> array;
	int idx = 0;
	long tsize = 0;

	defrag_heap();
	for (auto &&p : pfiles) {
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

	LOGPRIONC(Logger::Priority::SPAM) dec
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
	defrag_heap();
	LOGPRIONC(Logger::Priority::SPAM)
		"ACacheGC: Freed " << nent << " uncompressed audio cache entries, " << dropped / 1024 << "KiB\r\n";
}

