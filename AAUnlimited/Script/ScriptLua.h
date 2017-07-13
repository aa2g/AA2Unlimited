#pragma once
#include <codecvt>
#include "lua.hpp"
#include "Selene/selene.h"
#include "defs.h"

static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8;

class Lua : public sel::State {
public:;
	inline Lua() : sel::State() {};
	Lua(bool libs);
	/*
	template <typename T, typename... Funs>
	inline void extClass(const char *name, Funs... funs) {
		_registry.RegisterExtClass<T>(name, )
		MetatableRegistry::PushNewMetatable(state, typeid(T), _metatable_name);
	}*/

};

#define LUA_EXTCLASS(n,...) g_Lua.ExtClass<LUA_CLASS>(#n, __VA_ARGS__)
#define LUA_FIELD(n) #n, &LUA_CLASS::n

extern Lua g_Lua;