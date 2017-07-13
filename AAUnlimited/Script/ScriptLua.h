#pragma once
#include <codecvt>
#include "lua.hpp"
#include "Selene/selene.h"

static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8;

class Lua : public sel::State {
public:;
	inline Lua() : sel::State() {};
	Lua(bool libs);
};

extern Lua g_Lua;