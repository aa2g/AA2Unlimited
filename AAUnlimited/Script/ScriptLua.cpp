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
	g_Lua[LUA_BINDING_TABLE]["AAEditPath"] = utf8.to_bytes(AAEditPath);
	g_Lua[LUA_BINDING_TABLE]["AAPlayPath"] = utf8.to_bytes(AAPlayPath);
	g_Lua[LUA_BINDING_TABLE]["AAUPath"] = utf8.to_bytes(AAUPath);
	g_Lua[LUA_BINDING_TABLE]["GameExeName"] = utf8.to_bytes(GameExeName);
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
	g_Lua[LUA_BINDING_TABLE]["Peek"] = lua_CFunction([](lua_State *L) {
		// buf = Peek(addr, len)
		int addr = luaL_checkinteger(L, 1);
		int nbytes = luaL_checkinteger(L, 2);
		char *buf = (char*)alloca(nbytes);
		memcpy(buf, (void*)(addr), nbytes);
		lua_pushlstring(L, buf, nbytes);
		return 1;
	});
	g_Lua[LUA_BINDING_TABLE]["Poke"] = lua_CFunction([](lua_State *L) {
		// Poke(addr, buf)
		int addr = luaL_checkinteger(L, 1);
		size_t nbytes;
		const char *buf = luaL_checklstring(L, 2, &nbytes);
		void *ptr = (void*)(addr);
		Memrights unprotect(ptr, nbytes);
		memcpy(ptr, buf, nbytes);
		return 0;
	});
	g_Lua[LUA_BINDING_TABLE]["PInvoke"] = lua_CFunction([](lua_State *L) {
		// eax, edx = PInvoke(addr, stack), stdcall only
		int addr = luaL_checkinteger(L, 1);
		size_t nbytes;
		const char *buf = luaL_checklstring(L, 2, &nbytes);
		void *ptr = (void*)(addr);
		int saved_eax, saved_edx;
		__asm {
			mov eax, addr
			mov esi, buf
			mov edi, esp
			mov ecx, nbytes
			sub esp, ecx
			sub edi, ecx
			rep movsb
			call eax
			mov saved_eax, eax
			mov saved_edx, edx
		}
		lua_pushinteger(L, saved_eax);
		lua_pushinteger(L, saved_edx);
		return 2;
	});
	g_Lua[LUA_BINDING_TABLE]["XPages"] = [](int size) {
		void *d = VirtualAlloc(0, size, MEM_COMMIT, PAGE_EXECUTE_READ);
		return int(d);
	};
	g_Lua[LUA_BINDING_TABLE]["GetProcAddress"] = lua_CFunction([](lua_State *L) {
		if (lua_gettop(L) < 2) return 0;
		void *res = GetProcAddress(LoadLibraryA(lua_tostring(L, 1)), lua_tostring(L, 2));
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
	g_Lua[LUA_BINDING_TABLE]["GetGameTimeData"] = GameTimeData();
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

}
