#include <codecvt>
#include "config.h"
#include "Files/Logger.h"
#include "ScriptLua.h"
#include "General/ModuleInfo.h"



static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8;

// Pseudo-API
int lua_pushwstring(lua_State *L, std::wstring w) {
	auto s = utf8.to_bytes(w);
	lua_pushlstring(L, s.c_str(), s.length());
	return 1;
}

std::wstring luaL_checkwstring(lua_State *L, int idx) {
	size_t l;
	const char *s = luaL_checklstring(L, idx, &l);
	std::string sl(s, l);
	return utf8.from_bytes(sl);
}


// TBD: Wrap all of the following into singleton class to lessen the impact of having global namespace
lua_State *g_L;

#define EXPORT_NAME(n) lua_setfield(L, -2, #n)
#define EXPORT_INT(n) lua_pushinteger(L, n); EXPORT_NAME(n)
#define EXPORT_BOOL(n) lua_pushboolean(L, n); EXPORT_NAME(n)
#define EXPORT_WSTR(n) lua_pushwstring(L, n); EXPORT_NAME(n)

lua_State *LuaNewState()
{
	// Make state
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	lua_close(L);

	// Export bound values
	lua_newtable(L);

	using namespace General;
	EXPORT_INT(GameBase);
	EXPORT_BOOL(IsAAPlay);
	EXPORT_BOOL(IsAAEdit);
	EXPORT_WSTR(AAEditPath);
	EXPORT_WSTR(AAPlayPath);
	EXPORT_WSTR(AAUPath);
	EXPORT_WSTR(GameExeName);

	lua_setglobal(L, LUA_BINDING_TABLE);
	return L;
}

bool LuaRunScript(std::wstring &path) {
	lua_State *L = g_L; // for now
	std::wstring r;
	if (luaL_loadfile(L, utf8.to_bytes(path).c_str()) == LUA_OK && lua_pcall(L, 0, 0, 0) == LUA_OK)
		return 1;
	LOGPRIO(Logger::Priority::ERR) << "Script " << path << " failed with:\r\n" << lua_tostring(L, -1);
	lua_pop(L, 1);
	return 0;
}



