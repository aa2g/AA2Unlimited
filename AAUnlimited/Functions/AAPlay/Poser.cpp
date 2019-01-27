#include "Poser.h"

#include <string>
#include "Files\Logger.h"

#include "StdAfx.h"
#include "PoserController.h"

namespace Poser {

	PoserController g_PoserController;

	void LoadCharacter(ExtClass::CharacterStruct* charStruct) {
		g_PoserController.LoadCharacter(charStruct);
		g_PoserController.SwapTransientSliders(false);
	}

	void UpdateCharacter(ExtClass::CharacterStruct* charStruct) {
		g_PoserController.UpdateCharacter(charStruct);
		g_PoserController.SwapTransientSliders(true);
	}

	void RemoveCharacter(ExtClass::CharacterStruct* charStruct) {
		g_PoserController.RemoveCharacter(charStruct);
	}

	bool OverrideFile(wchar_t** paramArchive, wchar_t** paramFile, DWORD* readBytes, BYTE** outBuffer) {
		std::wstring& override = g_PoserController.GetOverride(std::wstring(*paramFile));
		if (override.empty()) return false;
		HANDLE hFile = CreateFile(override.c_str(), FILE_GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) {
			return false;
		}
		DWORD lo, hi;
		lo = GetFileSize(hFile, &hi);
		void* fileBuffer = Shared::IllusionMemAlloc(lo);
		ReadFile(hFile, fileBuffer, lo, &hi, NULL);
		CloseHandle(hFile);
		*outBuffer = (BYTE*)fileBuffer;
		*readBytes = hi;
		return true;
	}

	void FrameModEvent(ExtClass::XXFile* xxFile) {
		if (xxFile) {
			g_PoserController.FrameModEvent(xxFile);
		}
	}

	void bindLua() {
		auto b = g_Lua[LUA_BINDING_TABLE].get();
		b["GetPoserCharacter"] = LUA_LAMBDA({
			ExtClass::CharacterStruct* charStruct = (ExtClass::CharacterStruct*)(s.get(1));
			s.push(g_PoserController.GetPoserCharacter(charStruct));
		});
		b["GetPoserProp"] = LUA_LAMBDA({
			PoserController::PoserProp* prop = nullptr;
			ExtClass::XXFile* xxStruct = (ExtClass::XXFile*)(s.get(1));
			if (xxStruct) {
				prop = new PoserController::PoserProp(xxStruct);
				g_PoserController.FrameModProp(prop);
			}
			s.push(prop);
		});
		b["GetPoserController"] = LUA_LAMBDA({
			s.push(&g_PoserController);
		});
	}
}
