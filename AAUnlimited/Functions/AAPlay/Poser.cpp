#include "Poser.h"

#include <string>
#include "Files\Logger.h"

#include "StdAfx.h"
#include "PoserController.h"

namespace Poser {

#define X 0
#define Y 1
#define Z 2
#define W 3

	PoserController g_PoserController;
	SceneType currentScene = NoScene;
	int characterCount = 0;

	bool loc_syncing;

	void StartEvent(SceneType type) {
		currentScene = type;
	}

	void EndEvent() {
		currentScene = SceneType::NoScene;
	}

	void LoadCharacter(ExtClass::CharacterStruct* charStruct) {
		g_PoserController.LoadCharacter(charStruct);
	}

	void UpdateCharacter(ExtClass::CharacterStruct* charStruct) {
		g_PoserController.UpdateCharacter(charStruct);
	}

	void RemoveCharacter(ExtClass::CharacterStruct* charStruct) {
		g_PoserController.RemoveCharacter(charStruct);
	}

	bool OverrideFile(wchar_t** paramArchive, wchar_t** paramFile, DWORD* readBytes, BYTE** outBuffer) {
		if (!g_PoserController.IsActive()) {
			return false;
		}
		std::wstring& override = g_PoserController.GetOverride(std::wstring(*paramFile));
		override = General::BuildOverridePath(override.c_str());
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

	void OpenXXEvent(const wchar_t* file) {
		if (!wcsncmp(file, L"A02", 3))
			g_PoserController.VoidSkirtSliders();
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
		b["GetPoserController"] = LUA_LAMBDA({
			s.push(&g_PoserController);
		});
	}
}
