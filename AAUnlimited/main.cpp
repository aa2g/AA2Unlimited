#include "StdAfx.h"

#include <Dbghelp.h>
#include "General\ModuleInfo.h"
#include "Functions/RenderWrap.h"

// Needs: EarlyInit. Note that you can't log until you call InitLua.
static void InitLogger()
{
	g_Logger.Initialize(General::BuildAAUPath(LOGGER_FILE_PATH).c_str(), Logger::Priority::SPAM);
	SetDllDirectory(General::BuildAAUPath(L"lib").c_str());
}

// Needs: EarlyInit, Logger
static BOOL InitLua()
{
	g_Lua_p = (Lua*)GLua::newstate();

	// bind stuff
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


// Last step before handing contrlo over
static const char *NormalInit()
{
	srand(time(NULL));
	InitLogger();

	HANDLE hActCtx;
	ACTCTX actCtx;
	ULONG_PTR cookie;

	if (!InitLua())
		return NULL;

	if (g_Config.bUseVisualStyles) {
		ZeroMemory(&actCtx, sizeof(actCtx));
		actCtx.cbSize = sizeof(actCtx);
		actCtx.hModule = General::DllInst;
		actCtx.lpResourceName = MAKEINTRESOURCE(2);
		actCtx.dwFlags = ACTCTX_FLAG_HMODULE_VALID | ACTCTX_FLAG_RESOURCE_NAME_VALID;

		hActCtx = CreateActCtx(&actCtx);
		ActivateActCtx(hActCtx, &cookie);

	}
	if (!General::InitializeExeType()) {
		LOGPRIONC(Logger::Priority::CRIT_ERR) "Can't determine exe type, bail\r\n";
		return NULL;
	}

	LUA_SCOPE;

	// Now give chance to lua to run early. This loads that side of config, but
	// doesn't do anything with the game yet, for that we must wait for inithooks.
	g_Lua.bindLua();
	LOGPRIONC(Logger::Priority::SPAM) "Base API bound\r\n";
	try {
		g_Lua["load_modules"]();
	}
	catch (const char *err) {
		LOGPRIONC(Logger::Priority::CRIT_ERR) err;
		return NULL;
	}

	// Will load paths from registry only if lua didn't change those
	General::InitializePaths();

	LOGPRIONC(Logger::Priority::SPAM) "cfg test" << g_Config.getb("bUseMeshTextureOverrides") << "\r\n";
	InitializeHooks();
	LOGPRIONC(Logger::Priority::SPAM) "Memory hooks initialized.\r\n";
	LOGPRIONC(Logger::Priority::SPAM) std::hex << "memalloc: routing unified heap to 0x" << _get_heap_handle() << "\n";

	// And run rest of lua
	g_Logger.luaFlush(); // make lua see pending log entries

	if (g_Config.bUsePP2)
		g_PP2.Init();

	try {
		g_Lua["init_modules"]();
	}
	catch (const char *err) {
		LOGPRIONC(Logger::Priority::CRIT_ERR) err;
		return NULL;
	}

	if (g_Config.bUsePPeX)
		g_PPeX.Connect(L"\\\\.\\pipe\\PPEX");
	if (g_Config.bUsePP2) {
		g_PP2.InitProfiling();
		g_PP2.AddPath(General::BuildPlayPath(L"data"));
	}

	if (g_Config.bUseVisualStyles && General::IsAAEdit) {
		DeactivateActCtx(0, cookie);
		ReleaseActCtx(hActCtx);
	}

	return g_Config.gets("pathD3D9");
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

	for (int idx = 0; idx < 100; idx = idx + 1)
	{
		//Dump last triggers without slowing down the game
		LOGPRIONC(Logger::Priority::WARN) Shared::Triggers::triggers_log[Shared::Triggers::triggers_idxLog];
		Shared::Triggers::triggers_idxLog = (Shared::Triggers::triggers_idxLog + 1) % 100;

	}
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
		if (!General::EarlyInit())
			return FALSE;
		if (General::IsAAEdit || General::IsAAPlay) {
			Shared::Init();
			MemAlloc::Init();
		}
	}
	return TRUE;
}



static std::mutex mutex;

extern "C" __declspec(dllexport)
void* WINAPI AA2Unlimited(UINT SDKVersion)
{
	static IDirect3D9* (WINAPI *orig)(UINT SDKVersion);
	{
		std::lock_guard<std::mutex> guard(mutex);

		if (orig)
			return Render::Wrap(orig(SDKVersion));
	}

	SetErrorMode(0);
	SetUnhandledExceptionFilter(panic);

	const char *d3d = NormalInit();
	if (!d3d)
		d3d = "d3d9.dll";
	DWORD ptr = 0;
	LUA_EVENT("d3d9_preload", ptr);
	HMODULE h;
	if (ptr)
		h = (HMODULE)ptr;
	else
		h = LoadLibraryA(d3d);
	if (!h)
		LOGPRIONC(Logger::Priority::CRIT_ERR) "Failed to load " << d3d << " crash imminent\r\n";

	orig = decltype(orig)(GetProcAddress(h, "Direct3DCreate9"));
	if (!orig)
		LOGPRIONC(Logger::Priority::CRIT_ERR) "Failed to get Direct3DCreate9 constructor, crash imminent\r\n";

	LUA_EVENT_NORET("launch");
	ClimaxButton::InitCfg();

	char buf[1024];

	GetModuleFileNameA(h, buf, sizeof buf);
	LOGPRIONC(Logger::Priority::INFO) "Using " << buf << " for graphics rendering.\n";
	return Render::Wrap(orig(SDKVersion));
}

extern "C" __declspec(dllexport)
void WINAPI CALLBACK AA2UPatcher(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow) {
	SetUnhandledExceptionFilter(panic);
	General::InitializePaths();

	InitLogger();
	InitLua();
	g_Lua.bindLua();

	try {
		g_Lua["require"]("patcher")((const char*)lpszCmdLine, nCmdShow);
	}
	catch (const char *msg) {
		LOGPRIONC(Logger::Priority::CRIT_ERR) msg;
	}
}
