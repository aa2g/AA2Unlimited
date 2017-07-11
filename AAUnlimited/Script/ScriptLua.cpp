#include "config.h"
namespace Script {
namespace Lua {
#include "lua.hpp"
	void Initialize()
{
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	lua_close(L);
}
}
}