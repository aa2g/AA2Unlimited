#include <Windows.h>

#include <codecvt>
#include "defs.h"
#include "Files/Config.h"
#include "Files/Logger.h"
#include "ScriptLua.h"
#include "General/ModuleInfo.h"
#include "External/ExternalClasses/HClasses/HInfo.h"
#include "External/ExternalClasses/Frame.h"
#include "External/ExternalClasses/ConversationStruct.h"
#include "External/ExternalClasses/TimeData.h"
#include "External/ExternalVariables/AAPlay/GameGlobals.h"
#include "Functions/AAPlay/Globals.h"
#include "Functions/AAPlay/GameState.h"
#include "Functions/Shared/Triggers/Actions.h"
#include "MemMods/MemRightsLock.h"

Lua g_Lua(true);

Lua::Lua(bool libs) : sel::State(libs) {
}

// direct assembly code callback, stdcall/thiscall
int __stdcall callback_ptr(int _this, const DWORD *argbuf, int narg, int idx) {
	lua_State *L = g_Lua._l;
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

void Lua::init() {
	HandleExceptionsWith([](int n, std::string msg, std::exception_ptr) {
		LOGPRIONC(Logger::Priority::ERR) msg << "\r\n";
	});

	g_Lua(LUA_BINDING_TABLE " = {}");
	g_Lua(LUA_EVENTS_TABLE " = {}");
	g_Logger.bindLua();
	g_Config.bindLua();


	using namespace General;
	g_Lua[LUA_BINDING_TABLE]["GameBase"] = unsigned(GameBase);
	g_Lua[LUA_BINDING_TABLE]["IsAAPlay"] = IsAAPlay;
	g_Lua[LUA_BINDING_TABLE]["IsAAEdit"] = IsAAEdit;
	g_Lua[LUA_BINDING_TABLE]["GameExeName"] = utf8.to_bytes(GameExeName);

	g_Lua[LUA_BINDING_TABLE]["GetAAEditPath"] = []() { return utf8.to_bytes(AAEditPath); };
	g_Lua[LUA_BINDING_TABLE]["GetAAPlayPath"] = []() { return utf8.to_bytes(AAPlayPath); };
	g_Lua[LUA_BINDING_TABLE]["GetAAUPath"] = []() { return utf8.to_bytes(AAUPath); };
}

void Lua::bind() {
	using namespace ExtClass;

	// General
	Frame::bindLua();
	TimeData::bindLua();

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
	g_Lua[LUA_BINDING_TABLE]["peek"] = lua_CFunction([](lua_State *L) {
		// buf = peek(addr, len)
		int addr = luaL_checkinteger(L, 1);
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

	g_Lua[LUA_BINDING_TABLE]["poke"] = lua_CFunction([](lua_State *L) {
		// poke(addr, buf)
		int addr = luaL_checkinteger(L, 1);
		size_t nbytes;
		const char *buf = luaL_checklstring(L, 2, &nbytes);
		void *ptr = (void*)(addr);
		Memrights unprotect(ptr, nbytes);
		memcpy(ptr, buf, nbytes);
		return 0;
	});

	g_Lua[LUA_BINDING_TABLE]["proc_invoke"] = lua_CFunction([](lua_State *L) {
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
		int saved_eax, saved_edx;
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
	g_Lua[LUA_BINDING_TABLE]["callback"] = int(&callback_ptr);
	g_Lua[LUA_BINDING_TABLE]["x_pages"] = [](int size) {
		void *d = VirtualAlloc(0, size, MEM_COMMIT, PAGE_EXECUTE_READ);
		return int(d);
	};
	g_Lua[LUA_BINDING_TABLE]["GetProcAddress"] = lua_CFunction([](lua_State *L) {
		if (lua_gettop(L) < 2) return 0;
		void *res = GetProcAddress(GetModuleHandleA(lua_tostring(L, 1)), lua_tostring(L, 2));
		lua_pushinteger(L, int(res));
		return int(res != NULL);
	});

	// Low level triggers
	using namespace AAPlay;
	g_Lua[LUA_BINDING_TABLE]["ApplyRelationshipPoints"] = &ApplyRelationshipPoints;
	g_Lua[LUA_BINDING_TABLE]["GetCharacter"] = [](int idx) {
		if ((idx < 0) || (idx > 25) || !g_characters[idx].IsValid())
			return decltype(g_characters[0].m_char)(0);
		return g_characters[idx].m_char;
	};

	using namespace ExtVars::AAPlay;
	g_Lua[LUA_BINDING_TABLE]["GetGameTimeData"] = []() {
		return GameTimeData();
	};
	g_Lua[LUA_BINDING_TABLE]["GetPlayerCharacter"] = []() {
		return *PlayerCharacterPtr();
	};
	g_Lua[LUA_BINDING_TABLE]["SetPlayerCharacter"] = [](ExtClass::CharacterStruct *np) {
		*PlayerCharacterPtr() = np;
	};

	g_Lua[LUA_BINDING_TABLE]["GetPlayerConversation"] = &PlayerConversationPtr;

	// Higher level triggers
	using namespace Shared::Triggers;
	g_Lua[LUA_BINDING_TABLE]["SafeAddCardPoints"] = &SafeAddCardPoints;

	using namespace General;
	g_Lua[LUA_BINDING_TABLE]["SetAAPlayPath"] = [](std::string p) { AAPlayPath = utf8.from_bytes(p); };
	g_Lua[LUA_BINDING_TABLE]["SetAAEditPath"] = [](std::string p) { AAEditPath = utf8.from_bytes(p); };

}
