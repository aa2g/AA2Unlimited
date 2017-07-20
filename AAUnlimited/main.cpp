#include <Windows.h>
#include "Files/Logger.h"
#include "Files/Config.h"
#include "General/ModuleInfo.h"
#include "MemMods/Hook.h"
#include "Script/ScriptLua.h"
#include "Files/PPeX.h"
#include "defs.h"

#include <time.h>

BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
	)
{ 
	if (fdwReason == DLL_PROCESS_ATTACH) {
		srand(time(NULL));

		General::DllInst = hinstDLL;
		if (!General::Initialize()) {
			return FALSE;
		}

		g_Logger.Initialize(General::BuildAAUPath(LOGGER_FILE_PATH).c_str(), Logger::Priority::SPAM);
		g_Lua.init();

		while (!g_Lua.Load(utf8.to_bytes(General::BuildAAUPath(LUA_FILE_PATH)))) {
			switch (MessageBox(NULL, L"Lua bootstrap script failed (see logfile.txt)", L"Error", MB_ICONERROR | MB_ABORTRETRYIGNORE)) {
			case IDABORT:
				// Kill it
				ExitProcess(1);
			case IDIGNORE:
				// The module DLL will unload and the game will run vanilla.
				return FALSE;
			}
		}

		InitializeHooks();

		// This must be done post-hook initialization
		g_Lua.bind();
		SetDllDirectory(General::BuildAAUPath(L"lib").c_str());
		g_Lua["load_modules"]();
		g_Logger.flush(); // make lua see pending log entries
		g_Lua["init_modules"]();
		if (g_Config.bUsePPeX)
			g_PPeX.Connect(L"\\.\\pipe\\PPEX");
		return TRUE;
	}
}