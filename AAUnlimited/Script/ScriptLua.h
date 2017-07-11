#pragma once
#include "lua.hpp"

extern lua_State *g_L;

int lua_pushwstring(lua_State *L, std::wstring s);
std::wstring lua_towstring(lua_State *L, int idx);

// TBD: Wrap all of the following into a class deriving from lua_State, to avoid polluting global namespace
lua_State *LuaNewState();
bool LuaRunScript(std::wstring &path);
