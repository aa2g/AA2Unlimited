
#include "StdAfx.h"
#include "Files\PersistentStorage.h"
#include "External/ExternalClasses/XXFile.h"

#if 1
namespace SharedInjections {
namespace ArchiveFile {


// This structure seems to be subclassed from std::wstring of MSVC 2008,
// along with atomic counter and custom allocator.
// We do only as much to please its own vtable, since the refcount
// is pinned high and buffer large, they never touch the memory.
struct IllusionString {
	IllusionString(const wchar_t *s) {
		ptr = buf;
		if (!s) s = L"";
		set(s);
		if (General::IsAAPlay)
			vtable = (void**)(General::GameBase + 0x392738);
		else
			vtable = (void**)(General::GameBase + 0x36f638);
		refcount = 999;
		alloc = 1023;
	}
	wchar_t *ptr;
	void **vtable;
	size_t size;
	size_t alloc;
	size_t refcount;
	wchar_t buf[1024];
	wchar_t *set(const wchar_t *s) {
		wcscpy(buf, s);
		size = wcslen(s);
		return buf;
	}
	/*	static IllusionString *get(wchar_t **s) {
	if (!*s)
	return 0;
	return (IllusionString*)(((BYTE*)(*s)) - 20);
	}*/
};

DWORD OpenFileAddress, OpenXXAddress;

ExtClass::XXFile* __stdcall CallOpenXX(void *_this, wchar_t **name, void *pploadclass, wchar_t **file, DWORD a)
{
	ExtClass::XXFile *retv;
	__asm {
		lea ecx, [_this]
		push dword ptr[ecx+16]
		push dword ptr[ecx+12]
		push dword ptr[ecx+8]
		push dword ptr[ecx+4]
		mov ecx, [ecx]
		call gonext
		jmp skip
gonext:
		push -1
		jmp[OpenXXAddress]
skip:
		add esp, 16
		mov retv, eax
	}
	return retv;
}

void bindLua() {
	LUA_SCOPE;
	auto binding = g_Lua["_BINDING"];
	binding["LoadXX"] = LUA_LAMBDA({
		IllusionString arch(General::utf8.from_bytes((const char*)s.get(2)).c_str());
		IllusionString file(General::utf8.from_bytes((const char*)s.get(4)).c_str());
		CallOpenXX((void*)DWORD(s.get(1)), &arch.ptr, (void*)DWORD(s.get(3)), &file.ptr, s.get(5));
	});
}

void *__stdcall OpenXXEvent(void *this_, wchar_t **archname, void *pploadclass, wchar_t **file, DWORD a) {

	if (g_Config.bLogPPAccess & 2) {
		LOGPRIONC(Logger::Priority::SPAM) "LoadXX(0x" << this_ << ",[[" << std::dec << std::wstring(*archname) << "]],0x" << std::hex << pploadclass << ",[[" << std::wstring(*file) << "]]," << a << ")\r\n";
	}

	return CallOpenXX(this_, archname, pploadclass, file, a);
}

void __declspec(naked) OpenXXWrapper() {
	__asm {
		push dword ptr [esp + 16]
		push dword ptr [esp + 16]
		push dword ptr [esp + 16]
		push dword ptr [esp + 16]
		push ecx
		call OpenXXEvent
		ret
	}
}


// Calls the original open function
BYTE *CallOpenFile(void *_this, wchar_t **fname, DWORD *outsize, wchar_t **archive) {
	BYTE *retv;
	__asm {
		mov ecx, _this
		mov eax, fname
		mov edx, outsize
		push archive
		push edx
		call gocall
		jmp skip
gocall:
		sub esp, 8
		push ebx
		mov ebx, [esp + 14h]
		jmp[OpenFileAddress]
skip:
		add esp, 8
		mov retv, eax
	}
	return retv;
}




wchar_t *get_basepath(wchar_t *s) {
	wchar_t *p = wcsrchr(s, '\\');
	if (!p) return s;
	s = p + 1;
	p = wcsrchr(s, '/');
	if (!p) return s;
	return p + 1;
}


BYTE * __stdcall OpenFileEvent(void *_this, wchar_t **paramFile, DWORD* readBytes, wchar_t** paramArchive) {
	/*
	IllusionString *archive = IllusionString::get(paramArchive);
	IllusionString *file = IllusionString::get(paramFile);

	if (file)
		LOGPRIONC(Logger::Priority::SPAM) std::hex << "vtable is " << file->vtable << "\r\n";*/

	if (readBytes)
		*readBytes = 0;
	wchar_t *orig_archive = *paramArchive;
	wchar_t *orig_file = *paramFile;

	if (orig_archive && orig_archive[0])
		orig_file = get_basepath(orig_file);
//	if (!wcscmp(orig_file, L"MP_STOR_09FLOOR.xx"))
//		__debugbreak();

	wchar_t *parchive = orig_archive;
	wchar_t *pfile = orig_file;

	const char *provider = NULL;
	const char *rewriter = "";
	const char *rewriter2 = "";
	BYTE *outBuffer = NULL;

	// The following can't cope with nil archive (yet?)
	if (!parchive)
		goto skip;


	if (Poser::OverrideFile(&parchive, &pfile, readBytes, &outBuffer)) {
		provider = "poseroverride";
		goto done;
	}

	if (Shared::ArchiveReplaceRules(&parchive, &pfile, readBytes, &outBuffer)) {
		rewriter = "replace";
	}

	if (Shared::TanOverride(&parchive, &pfile, readBytes, &outBuffer)) {
		provider = "tanoverride";
		goto done;
	}

	if (Shared::HairRedirect(&parchive, &pfile, readBytes, &outBuffer)) {
		rewriter2 = ",hairredirect";
	}

	if (Shared::ArchiveOverrideRules(parchive, pfile, readBytes, &outBuffer)) {
		provider = "archiveoverride";
		goto done;
	}

	if (g_Config.bUseShadowing) {
		if (Shared::OpenShadowedFile(parchive, pfile, readBytes, &outBuffer)) {
			provider = "shadowed";
			goto done;
		}
	}

	if (g_Config.bUsePPeX) {
		if (g_PPeX.ArchiveDecompress(parchive, pfile, readBytes, &outBuffer)) {
			provider = "ppex";
			goto done;
		}
	}

skip:;
	if (g_Config.bUsePP2) {
		if (g_PP2.ArchiveDecompress(parchive, pfile, readBytes, &outBuffer)) {
			provider = "pp2";
			goto done;
		}
	}

done:;

	// If no provider got us the file, call the games original
	if (!provider) {
		IllusionString archive(parchive);
		IllusionString file(pfile);

		outBuffer = CallOpenFile(_this, &file.ptr, readBytes, &archive.ptr);
	}

	if (g_Config.bLogPPAccess & 1) {
		LOGPRIONC(Logger::Priority::SPAM) "OpenFileEvent " <<
			"provider=" << (provider ? provider : "pp") << " " <<
			"archive=" << std::wstring(parchive?parchive:L"<none>") << " " <<
			"nfile=" << std::wstring(pfile);
		if ((parchive != orig_archive) || (orig_file != pfile)) {
			if (!orig_archive)
				orig_archive = L"<none>";
			LOGSPAM << " rewriter=" << rewriter << rewriter2 << " " <<
				" origarchive=" << std::wstring(orig_archive) << " origfile=" << std::wstring(orig_file);
		}
		LOGSPAM << " size=" << std::dec << *readBytes;
		LOGSPAM << "\r\n";
	}
	return outBuffer;
}

class padstr {
	int padding;
	std::wstring s;
};

class AudioClass {
public:;
	   virtual void dummy();
	   virtual void dummy2();
	   virtual void dummy3();
	   virtual int init_sound_buffer(int a2, BYTE *buf, DWORD, wchar_t **bn, int a6, int a7, int a8, int a9, int a10);

	   int load_audio(int a2, wchar_t **archive, void *ppcls, wchar_t **fname, int a6, int a7, int a8, int a9, int a10) {
		   int ret = 0x80004005;
		   DWORD outSize;
		   wchar_t *p = *fname;
		   BYTE *buf = OpenFileEvent(ppcls, fname, &outSize, archive);
		   if (buf) {
			   IllusionString fn(get_basepath(*fname));
			   ret = init_sound_buffer(a2, buf, outSize, &fn.ptr, a6, a7, a8, a9, a10);
			   Shared::IllusionMemFree(buf);
		   }
		   else if (p) {
			   p = p + wcslen(p) - 4;
			   if (wcscmp(p, L".wav")) return 0x80004005;
			   wcscpy(p, L".ogg");
//			   LOGPRIONC(Logger::Priority::SPAM) "audio open failed, retrying with" << p << "\n";
			   BYTE *buf = OpenFileEvent(ppcls, fname, &outSize, archive);
			   if (!buf) return 0x80004005;

			   IllusionString fn(get_basepath(*fname));
			   ret = init_sound_buffer(a2, buf, outSize, &fn.ptr, a6, a7, a8, a9, a10);
			   Shared::IllusionMemFree(buf);
		   }
		   return ret;
	   }
};

// Target of the injected jump, just converts the usercall convention
void __declspec(naked) RedirOpenTarget() {
	__asm {
		push [esp + 8]
		push [esp + 8]
		push eax
		push ecx
		call OpenFileEvent
		ret
	}
}


void OpenFileInject() {
	auto ah = &AudioClass::load_audio;
	void *h;
	memcpy(&h, &ah, 4);

	if (General::IsAAPlay) {
		OpenFileAddress = General::GameBase + 0x1D0B28;
		OpenXXAddress = General::GameBase + 0x2165D2;

		PatchIAT((void*)(General::GameBase + 0x335110), h);
		PatchIAT((void*)(General::GameBase + 0x3351B0), h);
	}
	else if (General::IsAAEdit) {
		OpenFileAddress = General::GameBase + 0x1B32C8;
		OpenXXAddress = General::GameBase + 0x1F8B52;

		PatchIAT((void*)(General::GameBase + 0x313A50), h);
		PatchIAT((void*)(General::GameBase + 0x313AF0), h);
	}

	Hook((BYTE*)OpenFileAddress - 8,
	{ 0x83, 0xec, 0x08, 0x53, 0x8b, 0x5c, 0x24, 0x14 },
	{ 0x90, 0x90, 0x90, 0xe9, HookControl::RELATIVE_DWORD, (DWORD)&RedirOpenTarget },
		NULL);

	Hook((BYTE*)OpenXXAddress - 2 - 5,
	{ 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x6a, 0xff },
	{ 0xe9, HookControl::RELATIVE_DWORD, (DWORD)&OpenXXWrapper, 0xeb, 0xf9 },
		NULL);

}

}
}


#else

#include "StdAfx.h"
#include "Files\PersistentStorage.h"
namespace SharedInjections {
namespace ArchiveFile {



/*
* If false is returned, the original function will be executed.
* else, the function is aborted and the results from this function are used.
*/
bool __stdcall OpenFileEvent(wchar_t** paramArchive, wchar_t** paramFile, DWORD* readBytes, BYTE** outBuffer) {
	const wchar_t *orig_paramArchive = *paramArchive;
	const wchar_t *orig_paramFile = *paramFile;
	const char *provider = NULL;
	const char *rewriter = "";
	const char *rewriter2 = "";

	if (Poser::OverrideFile(paramArchive, paramFile, readBytes, outBuffer)) {
		provider = "overridefile";
		goto done;
	}

	// NOTE: return value implies the name was rewritten
	if (Shared::ArchiveReplaceRules(paramArchive, paramFile, readBytes, outBuffer)) {
		rewriter = "replace";
	}

	if (Shared::TanOverride(paramArchive, paramFile, readBytes, outBuffer)) {
		provider = "tanoverride";
		goto done;
	}

	if (Shared::HairRedirect(paramArchive, paramFile, readBytes, outBuffer)) {
		rewriter2 = ",hairredirect";
	}

	if (Shared::ArchiveOverrideRules(*paramArchive, *paramFile, readBytes, outBuffer)) {
		provider = "archiveoverride";
		goto done;
	}


	if (g_Config.bUseShadowing) {
		if (Shared::OpenShadowedFile(*paramArchive, *paramFile, readBytes, outBuffer)) {
			provider = "shadowed";
			goto done;
		}
	}

	if (g_Config.bUsePPeX) {
		if (g_PPeX.ArchiveDecompress(*paramArchive, *paramFile, readBytes, outBuffer)) {
			provider = "ppex";
			goto done;
		}
	}

	if (g_Config.bUsePP2) {
		if (g_PP2.ArchiveDecompress(*paramArchive, *paramFile, readBytes, outBuffer)) {
			provider = "pp2";
			goto done;
		}
	}

done:;
	if (g_Config.bLogPPAccess) {
		LOGPRIONC(Logger::Priority::SPAM) "OpenFileEvent " <<
			"provider=" << (provider?provider:"pp") << " " <<
			"archive=" << std::wstring(*paramArchive) << " " <<
			"file=" << std::wstring(*paramFile);
		if ((*paramArchive != orig_paramArchive) || (orig_paramFile != *paramFile)) {
			LOGSPAM << " rewriter=" << rewriter << rewriter2 << " " <<
				" origarchive=" << std::wstring(orig_paramArchive) << " origfile=" << std::wstring(orig_paramFile);
		}
		if (provider)
			LOGSPAM << " size=" << std::dec << *readBytes;
		LOGSPAM << "\r\n";
	}

	return provider != NULL;
}

DWORD OpenFileNormalExit;
void __declspec(naked) OpenFileRedirect() {
	__asm {
		pushad
		push[esp + 0x20 + 0x10 + 0]
		push edi
		lea eax, [esp + 0x20 + 0xC + 8]
		push eax
		lea eax, [esp + 0x20 + 4 + 0xC]
		push eax
		call OpenFileEvent
		test al, al
		popad
		jz OpenFileRedirect_NormalExit
		mov al, 1
		ret
	OpenFileRedirect_NormalExit :
		push ebp
		mov ebp, esp
		and esp, -8
		jmp[OpenFileNormalExit]
	}
}



#define PPF_HANDLE ((HANDLE)-2)
std::set<std::wstring> PPFileList;
std::set<std::wstring>::iterator ppf_it;
HANDLE ppf_handle = INVALID_HANDLE_VALUE;

void RegisterPP(const wchar_t *name) {
	PPFileList.insert(name);
}

static BOOL WINAPI MyFC(HANDLE h) {
	if (h == ppf_handle) {
		ppf_handle = INVALID_HANDLE_VALUE;
		if (h == PPF_HANDLE)
			return TRUE;
	}
	return FindClose(h);
}

static BOOL WINAPI MyFN(HANDLE h, LPWIN32_FIND_DATAW data) {
	if (h == ppf_handle) {
		// We'll interject, but not just yet, wait for normal file list to finish
		if (h != PPF_HANDLE && ppf_it == PPFileList.begin() && FindNextFileW(h, data))
			return TRUE;
		if (ppf_it == PPFileList.end())
			return FALSE;
		wcscpy(data->cFileName, (*ppf_it).c_str());
		data->dwFileAttributes = FILE_ATTRIBUTE_ARCHIVE;
		ppf_it++;
		return TRUE;
	}
	return FindNextFileW(h, data);
}

static bool is_pp_path(const wchar_t *path) {
	int pplen = wcslen(path);
	if (pplen < 5)
		return false;
	return !wcscmp(path + pplen - 4, L"*.pp");
}

static HANDLE WINAPI MyFF(const wchar_t *path, LPWIN32_FIND_DATAW data) {
	HANDLE h = FindFirstFileW(path, data);
	if (!is_pp_path(path))
		return h;

	ppf_it = PPFileList.begin();

	if (h == INVALID_HANDLE_VALUE) {
		ppf_handle = h = PPF_HANDLE;
		if (!MyFN(h, data))
			return (ppf_handle = INVALID_HANDLE_VALUE);
	}

	ppf_handle = h;
	return h;
}

void DirScanInject()
{
	DWORD *ffaddr = (DWORD*)(General::GameBase + 0x2E31E0);
	if (General::IsAAEdit)
		ffaddr = (DWORD*)(General::GameBase + 0x2C41E0);

	Memrights rights(ffaddr, 12);

	ffaddr[0] = (DWORD)&MyFC;
	ffaddr[1] = (DWORD)&MyFF;
	ffaddr[2] = (DWORD)&MyFN;
}

void OpenFileInject() {
	if (General::IsAAEdit) {
		//bool someFunc(edi = DWORD* readBytes, wchar* archive, 
		//				someClass* globalClass, wchar* filename, BYTE** outBuffer) {
		/*AA2Edit.exe+1F89F0 - 55                    - push ebp
		AA2Edit.exe+1F89F1 - 8B EC                 - mov ebp,esp
		AA2Edit.exe+1F89F3 - 83 E4 F8              - and esp,-08 { 248 }
		AA2Edit.exe+1F89F6 - 83 EC 18              - sub esp,18 { 24 }
		AA2Edit.exe+1F89F9 - 33 C0                 - xor eax,eax
		AA2Edit.exe+1F89FB - 53                    - push ebx
		AA2Edit.exe+1F89FC - 8B 5D 14              - mov ebx,[ebp+14]
		AA2Edit.exe+1F89FF - 89 44 24 08           - mov [esp+08],eax
		AA2Edit.exe+1F8A03 - 89 44 24 0C           - mov [esp+0C],eax
		AA2Edit.exe+1F8A07 - 89 44 24 10           - mov [esp+10],eax
		AA2Edit.exe+1F8A0B - 89 44 24 14           - mov [esp+14],eax
		AA2Edit.exe+1F8A0F - 89 44 24 18           - mov [esp+18],eax*/
		DWORD address = General::GameBase + 0x1F89F0;
		DWORD redirectAddress = (DWORD)(&OpenFileRedirect);
		Hook((BYTE*)address,
			{ 0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8 },						//expected values
			{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress, 0x90 },	//redirect to our function
			NULL);
		OpenFileNormalExit = General::GameBase + 0x1F89F6;
	}
	else if (General::IsAAPlay) {
		//bool someFunc(edi = DWORD* readBytes, wchar* archive, 
		//				someClass* globalClass, wchar* filename, BYTE** outBuffer) {
		/*AA2Play v12 FP v1.4.0a.exe+216470 - 55                    - push ebp
		AA2Play v12 FP v1.4.0a.exe+216471 - 8B EC                 - mov ebp,esp
		AA2Play v12 FP v1.4.0a.exe+216473 - 83 E4 F8              - and esp,-08 { 248 }
		AA2Play v12 FP v1.4.0a.exe+216476 - 83 EC 18              - sub esp,18 { 24 }
		AA2Play v12 FP v1.4.0a.exe+216479 - 33 C0                 - xor eax,eax
		AA2Play v12 FP v1.4.0a.exe+21647B - 53                    - push ebx
		AA2Play v12 FP v1.4.0a.exe+21647C - 8B 5D 14              - mov ebx,[ebp+14]
		AA2Play v12 FP v1.4.0a.exe+21647F - 89 44 24 08           - mov [esp+08],eax
		AA2Play v12 FP v1.4.0a.exe+216483 - 89 44 24 0C           - mov [esp+0C],eax
		AA2Play v12 FP v1.4.0a.exe+216487 - 89 44 24 10           - mov [esp+10],eax
		AA2Play v12 FP v1.4.0a.exe+21648B - 89 44 24 14           - mov [esp+14],eax
		AA2Play v12 FP v1.4.0a.exe+21648F - 89 44 24 18           - mov [esp+18],eax
		*/
		DWORD address = General::GameBase + 0x216470;
		DWORD redirectAddress = (DWORD)(&OpenFileRedirect);
		Hook((BYTE*)address,
			{ 0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8 },						//expected values
			{ 0xE9, HookControl::RELATIVE_DWORD, redirectAddress, 0x90 },	//redirect to our function
			NULL);
		OpenFileNormalExit = General::GameBase + 0x216476;
	}
			
}

}
}

#endif