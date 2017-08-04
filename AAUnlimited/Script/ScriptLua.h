#pragma once
#include "lua.hpp"
#include "glua.h"
#include "defs.h"

struct Lua : public GLua::State {
	void bindLua();
	void init();
	bool Load(std::wstring path);
	~Lua() = delete;
	Lua() = delete;
};

#define LUA_GLOBAL g_Lua
extern Lua& LUA_GLOBAL;
#define LUA_L LUA_GLOBAL.L()

#define LUA_EVENT(...) LUA_GLOBAL["__DISPATCH_EVENT"](__VA_ARGS__)
#define LUA_LAMBDA(fn) GLua::Function([](auto &s) {fn;return 1;})
#define LUA_LAMBDA0(fn) GLua::Function([](auto &s) {fn;return 0;})
#define LUA_LAMBDA_L(fn) lua_CFunction([](lua_State *L) fn)

// More fancy macros
#include "gluam.h"
