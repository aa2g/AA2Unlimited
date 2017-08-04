#include "StdAfx.h"

Logger g_Logger;

Logger::Logger() : currPrio(Priority::ERR), filter(Priority::SPAM) {

}

void Logger::luaFlush()
{
	// Lua can't survive on multiple threads. If the log entry comes in from different
	// thread, it will live on recorded in the buffer. Main thread picks it up
	// at some point later.
	if (std::this_thread::get_id() != tid)
		return;

	// Note that we can't use fancy selene templating in here, because that
	// by itself uses logger, ie if something goes awry, it would loop.
	lua_State *L = g_Lua.L();
	lua_getglobal(L, LUA_EVENTS_TABLE);
	if (!lua_isnil(L, -1)) {
		lua_getfield(L, -1, "logger");
		auto ostr = outbuf.str();
		lua_pushlstring(L, ostr.c_str(), ostr.length());
		if (lua_pcall(L, 1, 1, 0) == LUA_OK && lua_toboolean(L, -1))
			outbuf.str("");
		lua_pop(L, 1);
	}
	lua_pop(L, 1);
}

Logger::Logger(const TCHAR * file,Priority prio) : currPrio(Priority::ERR),filter(prio)
{
	Initialize(file, prio);
}

void Logger::Initialize(const TCHAR * file, Priority prio) {
	tid = std::this_thread::get_id();
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
