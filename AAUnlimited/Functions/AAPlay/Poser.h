#pragma once
#include <Windows.h>

namespace ExtClass {
	class CharacterStruct;
	class XXFile;
}

namespace Poser {

	enum SceneType {
		NoScene = 0,
		ClothingScene = 1,
		DialogueScene = 2,
		InterruptionScene = 4,
		HScene = 8
	};

	void StartEvent(SceneType type);
	void EndEvent();
	void LoadCharacter(ExtClass::CharacterStruct* c);
	void UpdateCharacter(ExtClass::CharacterStruct* c);
	void RemoveCharacter(ExtClass::CharacterStruct* c);

	void OpenXXEvent(const wchar_t* file);
	void FrameModEvent(ExtClass::XXFile* xxFile);

	bool OverrideFile(wchar_t** paramArchive, wchar_t** paramFile, DWORD* readBytes, BYTE** outBuffer);

	void bindLua();

};
