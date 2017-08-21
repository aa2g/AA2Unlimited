#pragma once
#include "lua.hpp"
#include "glua.h"
#include "defs.h"

#include <string>

struct Lua : public GLua::State {
	void bindLua();
	void init();
	bool Load(std::wstring path);
};

static_assert(sizeof(Lua) == sizeof(GLua::State), "Lua state must not have instance members");

#define g_Lua (*g_Lua_p)
#define LUA_GLOBAL g_Lua
extern Lua *g_Lua_p;
#define LUA_L LUA_GLOBAL.L()

#define LUA_EVENT(name, ret,...) { \
	LUA_SCOPE; \
	ret = LUA_GLOBAL["__DISPATCH_EVENT"](name, ret, __VA_ARGS__); \
}

#define LUA_EVENT_NORET(...) { \
	LUA_SCOPE; \
	LUA_GLOBAL["__DISPATCH_EVENT"](__VA_ARGS__); \
}

#define LUA_LAMBDA(fn) GLua::Function([](auto &s) {fn;return 1;})
#define LUA_LAMBDA0(fn) GLua::Function([](auto &s) {fn;return 0;})
#define LUA_LAMBDA_L(fn) lua_CFunction([](lua_State *L) {fn;return 1;})

#define LUA_SCOPE GLua::Scope scope(g_Lua_p)

// More fancy macros
#include "gluam.h"
