#include <Windows.h>
#include "Files/Logger.h"
#include "Files/Config.h"
#include "General/ModuleInfo.h"
#include "MemMods/Hook.h"
#include "config.h"

BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
	)
{ 
	if (fdwReason == DLL_PROCESS_ATTACH) {
		//change calls in code to ours
		//MessageBox(NULL, "hi", "hi", 0);
		General::DllInst = hinstDLL;
		if (!General::Initialize()) {
			return FALSE;
		}
		g_Logger.Initialize(General::BuildPlayPath(LOGGER_FILE_PATH).c_str(), Logger::Priority::SPAM);
		g_Config = Config(General::BuildPlayPath(CONFIG_FILE_PATH).c_str());
		InitializeHooks();
		return TRUE;
	}
}