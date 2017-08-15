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

	}
}
