#include "StdAfx.h"

#include <Dbghelp.h>
#include "General\ModuleInfo.h"


static BOOL bootstrap()
{
	SetDllDirectory(General::BuildAAUPath(L"lib").c_str());

	g_Lua_p = (Lua*)GLua::newstate();
	g_Logger.Initialize(General::BuildAAUPath(LOGGER_FILE_PATH).c_str(), Logger::Priority::SPAM);
	g_Lua.init();

	while (!g_Lua.Load(General::BuildAAUPath(LUA_FILE_PATH))) {
		switch (MessageBox(NULL, L"Lua bootstrap script failed (see logfile.txt)", L"Error", MB_ICONERROR | MB_ABORTRETRYIGNORE)) {
		case IDABORT:
			ExitProcess(1);
		case IDIGNORE:
			// The game will run vanilla
			return FALSE;
		}
	}

	LOGPRIONC(Logger::Priority::SPAM) "Bootstrap finished\r\n";
	return TRUE;
}

static const char *startup()
{
	srand(time(NULL));

	if (!bootstrap())
		return NULL;

	// Now give chance to lua to run early. This loads that side of config, but
	// doesn't do anything with the game yet, for that we must wait for inithooks.
	g_Lua.bindLua();
	LOGPRIONC(Logger::Priority::SPAM) "Base API bound\r\n";
	g_Lua["load_modules"]();

	General::InitializePaths();

	// Now initialize the rest
	if (!General::Initialize()) {
		LOGPRIONC(Logger::Priority::CRIT_ERR) "General init failed, bail\r\n";
		return NULL;
	}
	InitializeHooks();
	LOGPRIONC(Logger::Priority::SPAM) "Memory hooks initialized.\r\n";

	// And run rest of lua
	g_Logger.luaFlush(); // make lua see pending log entries
	g_Lua["init_modules"]();
	if (g_Config.bUsePPeX)
		g_PPeX.Connect(L"\\\\.\\pipe\\PPEX");
	if (g_Config.bUsePP2) {
		g_PP2.AddPath(General::BuildPlayPath(L"data"));
	}

	return g_Config["pathD3D9"];
}

static LONG WINAPI panic(EXCEPTION_POINTERS *exceptionInfo) {
	char message[255];
	sprintf_s<255>(message,
		"Code: 0x%08X\nAddress: 0x%08X\n\nDo you want to write aaucrash.dmp?",
		(DWORD)exceptionInfo->ExceptionRecord->ExceptionCode,
		(DWORD)exceptionInfo->ExceptionRecord->ExceptionAddress);

	if (MessageBoxA(0, message, "Unhandled exception", MB_YESNO) == IDNO)
		return EXCEPTION_CONTINUE_SEARCH;

	HANDLE hFile = CreateFile(L"aaucrash.dmp",
		GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return EXCEPTION_CONTINUE_SEARCH;

	MINIDUMP_EXCEPTION_INFORMATION mdei;
	mdei.ThreadId = GetCurrentThreadId();
	mdei.ExceptionPointers = exceptionInfo;
	mdei.ClientPointers = TRUE;
	MINIDUMP_TYPE mdt = MiniDumpNormal;
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &mdei, 0, 0);
	CloseHandle(hFile);
	return EXCEPTION_EXECUTE_HANDLER;
}

/////////////////////// EXPORTS

BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {
		General::DllInst = hinstDLL;
		if (!General::InitializeAAU())
			return FALSE;
	}
	return TRUE;
}

extern "C" __declspec(dllexport)
IDirect3D9* WINAPI AA2Unlimited(UINT SDKVersion)
{
	static IDirect3D9* (WINAPI *orig)(UINT SDKVersion);
	if (orig)
		return orig(SDKVersion);

	SetErrorMode(0);
	SetUnhandledExceptionFilter(panic);

	const char *d3d = startup();
	//if (!d3d)
		d3d = "d3d9.dll";
	HMODULE h = LoadLibraryA(d3d);
	if (!h)
		LOGPRIONC(Logger::Priority::CRIT_ERR) "Failed to load " << d3d << " crash imminent\r\n";

	orig = decltype(orig)(GetProcAddress(h, "Direct3DCreate9"));
	if (!orig)
		LOGPRIONC(Logger::Priority::CRIT_ERR) "Failed to get Direct3DCreate9 constructor, crash imminent\r\n";

	return orig(SDKVersion);
}

extern "C" __declspec(dllexport)
void WINAPI CALLBACK AA2UPatcher(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow) {
	bootstrap();
	General::InitializePaths();
	try {
		g_Lua["require"]("patcher")((const char*)lpszCmdLine, nCmdShow);
	}
	catch (const char *msg) {
		LOGPRIONC(Logger::Priority::CRIT_ERR) msg;
	}
}
