#include <Windows.h>
#include "Logger.h"
#include "Script/ScriptLua.h"

#include <string>

Logger g_Logger;

void Logger::bindLua()
{
	auto b = g_Lua[LUA_BINDING_TABLE];
	b["setlogprio"] = ([](int n) {
		g_Logger.SetPriority(Logger::Priority(n));
	});

	b["logger"] = lua_CFunction([](lua_State *L) {
		Logger::Priority prio = (Logger::Priority)luaL_checkinteger(L, 1);
		int top = lua_gettop(L);
		for (int i = 2; i <= top; i++) {
			g_Logger << prio << luaL_checkstring(L, i) << "\r\n";
		}
		return 0;
	});
}


Logger::Logger() : currPrio(Priority::ERR), filter(Priority::SPAM) {

}

Logger::Logger(const TCHAR * file,Priority prio) : currPrio(Priority::ERR),filter(prio)
{
	Initialize(file, prio);
}

void Logger::Initialize(const TCHAR * file, Priority prio) {
	outfile.open(file);
	if (!outfile.good()) {
		MessageBox(0, (std::wstring(TEXT("Could not open Logfile ")) + file).c_str(), TEXT("Error"), 0);
	}
}

Logger::~Logger()
{
	outfile.close();
}

void Logger::SetPriority(Priority prio) {
	filter = prio;
	*this << "------- applied priority " << prio << " -------\r\n";
}

/* Returns true if the given priority is covered by the current priority, or else false */
bool Logger::FilterPriority(Priority prio)
{
	return prio >= filter;
}
