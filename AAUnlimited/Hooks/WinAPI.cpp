#include "StdAfx.h"
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

		void CreateFileInject()
		{
			DWORD *cfaddr = (DWORD*)(General::GameBase + 0x2E31B0);
			if (General::IsAAEdit) return;

			Memrights rights(cfaddr, 4);

			cfaddr[0] = (DWORD)&MyCF;
		}

////////////////////////
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

	}
}
