#include "StdAfx.h"
#include <Shlwapi.h>

#include "Files\PersistentStorage.h"

namespace SharedInjections {
namespace WinAPI {


static HANDLE WINAPI MyCF(
	_In_     LPCTSTR               lpFileName,
	_In_     DWORD                 dwDesiredAccess,
	_In_     DWORD                 dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_     DWORD                 dwCreationDisposition,
	_In_     DWORD                 dwFlagsAndAttributes,
	_In_opt_ HANDLE                hTemplateFile)
{
	HANDLE h = CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

	if (General::CastToString(lpFileName).find(".sav") != std::string::npos) {
		//extract the filename

		std::wstring filename = std::wstring(lpFileName);
		filename = filename.substr(filename.find(CLASS_SAVES_PATH) + std::wcslen(CLASS_SAVES_PATH));

		if (dwDesiredAccess == GENERIC_WRITE) {	//game saved
			auto filenameNoExt = filename.substr(0, filename.size() - 4);
			auto storage = PersistentStorage::ClassStorage::getStorage(filenameNoExt);
			storage.save();
		}
	}

	return h;
}

////////////////////////
#define PPF_HANDLE ((HANDLE)-2)
std::set<std::wstring> *FFList;
std::set<std::wstring>::iterator FFList_it;


HANDLE ppf_handle = INVALID_HANDLE_VALUE;

static BOOL WINAPI MyFC(HANDLE h) {
	if (h == ppf_handle) {
		ppf_handle = INVALID_HANDLE_VALUE;
		if (h == PPF_HANDLE) {
			FFList = NULL;
			return TRUE;
		}
	}
	return FindClose(h);
}

static BOOL WINAPI MyFN(HANDLE h, LPWIN32_FIND_DATAW data) {
	if (FFList && (h == ppf_handle)) {
		// We'll interject, but not just yet, wait for normal file list to finish
		if (h != PPF_HANDLE && FFList_it == FFList->begin() && FindNextFileW(h, data))
			return TRUE;
		if (FFList_it == FFList->end()) {
			FFList = NULL;
			SetLastError(ERROR_FILE_NOT_FOUND);
			return FALSE;
		}
		wcscpy(data->cFileName, (*FFList_it).c_str());
		data->dwFileAttributes = FILE_ATTRIBUTE_ARCHIVE;
		FFList_it++;
		return TRUE;
	}
	return FindNextFileW(h, data);
}

static HANDLE WINAPI MyFF(const wchar_t *path, LPWIN32_FIND_DATAW data) {
	//LOGPRIONC(Logger::Priority::SPAM) "FindFirstFile(" << std::wstring(path) << ")\n";

	HANDLE h = FindFirstFileW(path, data);
	DWORD err = GetLastError();

	// You can add more FindFirst hooks in here
	FFList = 0;
	if (!FFList)
		FFList = g_PP2.FList(path);
	if (!FFList) {
		SetLastError(err);
		return h;
	}

	FFList_it = FFList->begin();

	if (h == INVALID_HANDLE_VALUE) {
		ppf_handle = h = PPF_HANDLE;
		if (!MyFN(h, data)) {
			SetLastError(ERROR_FILE_NOT_FOUND);
			return (ppf_handle = INVALID_HANDLE_VALUE);
		}
	}

	SetLastError(0);
	ppf_handle = h;
	return h;
}

#define RVA2PTR(t,base,rva) ((t)(((PCHAR) base) + rva))
void *patch_iat(const char *func, void *to)
{
	HMODULE hostexe = (HMODULE)General::GameBase;
	PIMAGE_DOS_HEADER mz = (PIMAGE_DOS_HEADER)hostexe;
	PIMAGE_IMPORT_DESCRIPTOR imports;

	imports = RVA2PTR(PIMAGE_IMPORT_DESCRIPTOR, mz, RVA2PTR(PIMAGE_NT_HEADERS,mz, mz->e_lfanew)->
		OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	for (int i = 0; imports[i].Characteristics; i++) {
		PIMAGE_THUNK_DATA t1, t2;
		PIMAGE_IMPORT_BY_NAME import;

		t1 = RVA2PTR(PIMAGE_THUNK_DATA, mz, imports[i].FirstThunk);
		t2 = RVA2PTR(PIMAGE_THUNK_DATA, mz, imports[i].OriginalFirstThunk);

		for (; t2->u1.Function; t1++, t2++) {
			void *oldfn;
			DWORD oldp;
			MEMORY_BASIC_INFORMATION vmi;

			if (t2->u1.Ordinal & IMAGE_ORDINAL_FLAG)
				continue;

			import = RVA2PTR(PIMAGE_IMPORT_BY_NAME, mz, t2->u1.AddressOfData);
			if (strcmp(func, (char*)import->Name))
				continue;

			oldfn = (void*)t1->u1.Function;

			VirtualQuery(t1, &vmi, sizeof(vmi));
			if (!VirtualProtect(vmi.BaseAddress, vmi.RegionSize, PAGE_READWRITE, &oldp)) {
				return NULL;
			}
			t1->u1.Function = (ULONG_PTR)to;
			VirtualProtect(vmi.BaseAddress, vmi.RegionSize, oldp, &oldp);
			return oldfn;
		}
	}
	return NULL;
}

BOOL WINAPI MyExists(
	_In_ LPCTSTR path
) {
	LOGPRIONC(Logger::Priority::SPAM) "PathFileExistsW(" << std::wstring(path) << ")\n";

	return PathFileExistsW(path);
}


struct {
	const char *name;
	void *fn;
	void **old;
} patches[] = {
	{"FindFirstFileW", &MyFF, 0},
	{"FindNextFileW", &MyFN, 0},
	{"FindClose", &MyFC, 0 },
	{"CreateFileW", &MyCF, 0},
	{"PathFileExistsW", &MyExists, 0},
	{0}
};

void Inject() {

	for (int i = 0; patches[i].name; i++) {
		void *oldp = patch_iat(patches[i].name, patches[i].fn);
		if (patches[i].old)
			*patches[i].old = oldp;
	}

}

}
}
