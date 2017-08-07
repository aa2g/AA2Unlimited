#include "StdAfx.h"

#include <codecvt>
#include <string>

static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8;
Lua *g_Lua_p;

static const char *to_utf8(std::wstring &ws) {
	static std::string s;
	s = utf8.to_bytes(ws);
	return s.c_str();
}

// direct assembly code callback, stdcall/thiscall
int __stdcall callback_ptr(int _this, const DWORD *argbuf, int narg, int idx) {
	lua_State *L = LUA_GLOBAL.L();
	lua_getglobal(L, LUA_EVENTS_TABLE);
	lua_getfield(L, -1, "callback");
	lua_rawgeti(L, -1, idx);

	lua_pushinteger(L, _this);
	for (int i = 0; i < narg; i++)
		lua_pushinteger(L, argbuf[i]);

	int ok = lua_pcall(L, 1+narg, 1, 0);
	if (ok != LUA_OK) {
		LOGPRIONC(Logger::Priority::CRIT_ERR) "Callback failed " << lua_tostring(L, -1) << "\r\n";
	}

	int ret = lua_tointeger(L, -1);
	lua_pop(L, 3);
	return ret;
}

bool Lua::Load(std::wstring wpath) {
	if (luaL_loadfile(L(), to_utf8(wpath)) != LUA_OK || lua_pcall(L(),0,0,0) != LUA_OK) {
		LOGPRIONC(Logger::Priority::CRIT_ERR) "Bootstrap failed with error " << lua_tostring(L(), -1) << "\r\n";
		lua_pop(L(), 1);
		return false;
	}
	LOGPRIONC(Logger::Priority::SPAM) "Bootstrapped lua from " << wpath << "\r\n";
	return true;
}

void Lua::init() {
	LUA_SCOPE;

	g_Lua.eval(LUA_BINDING_TABLE " = {}");
	g_Lua.eval(LUA_EVENTS_TABLE " = {}");
	g_Logger.bindLua();
	g_Config.bindLua();


	using namespace General;
	auto _BINDING = g_Lua[LUA_BINDING_TABLE].get();
	_BINDING["Config"] = &g_Config;
	_BINDING["GameBase"] = DWORD(GameBase);
	_BINDING["IsAAPlay"] = IsAAPlay;
	_BINDING["IsAAEdit"] = IsAAEdit;
	_BINDING["GameExeName"] = to_utf8(GameExeName);

	_BINDING["GetAAEditPath"] = GLua::Function([](auto &s) {
		s.push(to_utf8(AAEditPath));
		return 1;
	});
	_BINDING["GetAAPlayPath"] = GLua::Function([](auto &s) {
		s.push(to_utf8(AAPlayPath));
		return 1;
	});
	_BINDING["GetAAUPath"] = GLua::Function([](auto &s) {
		s.push(to_utf8(AAUPath));
		return 1;
	});
}

void Lua::bindLua() {
	LUA_SCOPE;
	auto _BINDING = g_Lua[LUA_BINDING_TABLE].get();

	using namespace ExtClass;

	// General
	Frame::bindLua();
	TimeData::bindLua();
	XXFile::bindLua();

	// Character/interaction
	CharacterActivity::bindLua();
	CharacterData::bindLua();
	CharacterRelation::bindLua();
	CharacterStruct::bindLua();
	ConversationSubStruct::bindLua();
	NpcPcInteractiveConversationStruct::bindLua();
	PcConversationStruct::bindLua();
	CharInstData::ActionParamStruct::bindLua();

	// H
	HCamera::bindLua();
	HGUIButton::bindLua();
	HInfo::bindLua();
	HParticipant::bindLua();
	HPosButtonList::bindLua();
	HStatistics::bindLua();

	// Very low level utilities
	using namespace General;
	_BINDING["peek"] = lua_CFunction([](lua_State *L) {
		// buf = peek(addr, len)
		int addr = (int)lua_touserdata(L, 1);
		if (addr == 0) addr = luaL_checkinteger(L, 1);
		int scanmax = luaL_checkinteger(L, 2);	
		char *p = (char*)addr;
		int i = scanmax;

		if (lua_gettop(L) > 2) {
			size_t untilsz;
			int step = 1;
			const char *until = luaL_checklstring(L, 3, &untilsz);
			if (lua_gettop(L) > 3)
				step = lua_tointeger(L, 4);
			if (step < 1)
				step = 1;

			for (i = 0; i < scanmax; i += step) {
				if (!memcmp(p + i, until, untilsz))
					break;
			}
		}
		
		lua_pushlstring(L, p, i);
		return 1;
	});

	_BINDING["poke"] = lua_CFunction([](lua_State *L) {
		// poke(addr, buf)
		int addr = luaL_checkinteger(L, 1);
		size_t nbytes;
		const char *buf = luaL_checklstring(L, 2, &nbytes);
		void *ptr = (void*)(addr);
		Memrights unprotect(ptr, nbytes);
		memcpy(ptr, buf, nbytes);
		return 0;
	});

	_BINDING["proc_invoke"] = lua_CFunction([](lua_State *L) {
		// eax, edx = proc_invoke(addr, args...), stdcall/thiscall only
		DWORD *argbuf = (DWORD*)alloca((lua_gettop(L) - 2)*4);
		int addr = luaL_checkinteger(L, 1);
		int _this = lua_tointeger(L, 2);
		for (int i = 3; i <= lua_gettop(L); i++) {
			if (lua_type(L, i) == LUA_TSTRING) {
				argbuf[i - 3] = (DWORD)lua_tostring(L, i);
			}
			else {
				argbuf[i - 3] = lua_tointeger(L, i);
			}
		}
		size_t nbytes = (lua_gettop(L)-2)*4;
		void *ptr = (void*)(addr);
		DWORD saved_eax, saved_edx;
		__asm {
			mov eax, addr
			mov edx, _this
			mov esi, argbuf
			mov edi, esp
			mov ecx, nbytes
			sub esp, ecx
			sub edi, ecx
			rep movsb
			mov ecx, edx
			call eax
			mov saved_eax, eax
			mov saved_edx, edx
		}
		lua_pushinteger(L, saved_eax);
		lua_pushinteger(L, saved_edx);
		return 2;
	});
	_BINDING["callback"] = DWORD(&callback_ptr);
	_BINDING["x_pages"] = LUA_LAMBDA({
		void *d = VirtualAlloc(0, s.get(1), MEM_COMMIT, PAGE_EXECUTE_READ);
		s.push(DWORD(d));
	});
	_BINDING["GetProcAddress"] = LUA_LAMBDA_L({
		if (lua_gettop(L) < 2) return 0;
		void *res = GetProcAddress(GetModuleHandleA(lua_tostring(L, 1)), lua_tostring(L, 2));
		lua_pushinteger(L, int(res));
		return int(res != NULL);
	});

	// Low level triggers
	using namespace AAPlay;
	_BINDING["GetCharacter"] = LUA_LAMBDA({
		int idx = s.get(1);
		if ((idx < 0) || (idx > 25) || !g_characters[idx].IsValid())
			return 0;
		s.push(g_characters[idx].m_char);
	});

	using namespace ExtVars::AAPlay;
	_BINDING["GetGameTimeData"] = LUA_LAMBDA({
		s.push(GameTimeData());
	});
	_BINDING["GetPlayerCharacter"] = LUA_LAMBDA({
		s.push(*PlayerCharacterPtr());
	});
	_BINDING["SetPlayerCharacter"] = LUA_LAMBDA0({
		*PlayerCharacterPtr() = s.get(1);
	});

//	_BINDING["GetPlayerConversation"] = &PlayerConversationPtr;

	// Higher level triggers
	_BINDING["SafeAddCardPoints"] = LUA_LAMBDA0({
		Shared::Triggers::SafeAddCardPoints(s.get(1), s.get(2), s.get(3), s.get(4));
	});

	_BINDING["SetFocusBone"] = LUA_LAMBDA0({
		HCamera::SetFocusBone(s.get(1), s.get(2), s.get(3), s.get(4));
	});

	using namespace General;
	_BINDING["SetAAPlayPath"] = LUA_LAMBDA0({
		AAPlayPath = utf8.from_bytes((const char*)s.get(1));
	});
	_BINDING["SetAAEditPath"] = LUA_LAMBDA0({
		AAEditPath = utf8.from_bytes((const char*)s.get(1));
		return 0;
	});

	GameTick::RegisterMsgFilter(GameTick::MsgFilterFunc([](MSG *m) {
		if (m->hwnd != *GameTick::hwnd) return false;
		const char *mstr = NULL;
		switch (m->message) {
		case WM_KEYDOWN: mstr = "keydown"; break;
		case WM_KEYUP: mstr = "keyup"; break;
		case WM_CHAR: mstr = "char"; break;
		}
		if (mstr) {
			LUA_EVENT(mstr, m->wParam);
			if (m->wParam == -1)
				return true;
		}
		return false;
	}));

}
