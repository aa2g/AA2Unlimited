#pragma once
#include <Windows.h>

namespace ExtClass {
	class CharacterStruct;
	class XXFile;
}

namespace Poser {

	void LoadCharacter(ExtClass::CharacterStruct* c);
	void UpdateCharacter(ExtClass::CharacterStruct* c);
	void RemoveCharacter(ExtClass::CharacterStruct* c);

	void FrameModEvent(ExtClass::XXFile* xxFile);

	bool OverrideFile(wchar_t** paramArchive, wchar_t** paramFile, DWORD* readBytes, BYTE** outBuffer);

	void bindLua();

};
