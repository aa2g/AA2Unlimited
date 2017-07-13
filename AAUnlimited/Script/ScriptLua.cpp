#include <Windows.h>

#include <codecvt>
#include "defs.h"
#include "Files/Config.h"
#include "Files/Logger.h"
#include "ScriptLua.h"
#include "General/ModuleInfo.h"

Lua g_Lua;

Lua::Lua(bool libs) : sel::State(libs) {
	HandleExceptionsWith([](int n, std::string msg, std::exception_ptr) {
		LOGPRIONC(Logger::Priority::ERR) msg << "\r\n";
	});
	Lua &l = *this;
	l(LUA_BINDING_TABLE " = {}");
}
