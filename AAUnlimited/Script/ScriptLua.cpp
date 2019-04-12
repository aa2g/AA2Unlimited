#include "StdAfx.h"
#include "Functions/Render.h"
#include "Files/PersistentStorage.h"
#include "Functions/AAPlay/PoserController.h"
#include "MemMods/AAPlay/Events/UiEvent.h"

#include <string>

Lua *g_Lua_p;
using namespace General;
Shared::Triggers::KeyPressData keyPressData;

// direct assembly code callback, stdcall/thiscall/cdecl
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
	_BINDING["cast"] = GLua::Function([](auto &s) {
		s.cast(s.get(1), (void*)int(s.get(2)));
		return 1;
	});
}

void Lua::bindLua() {
	LUA_SCOPE;
	auto _BINDING = g_Lua[LUA_BINDING_TABLE].get();

	using namespace ExtClass;

	// General
	Frame::bindLua();
	Material::bindLua();
	TimeData::bindLua();
	XXFile::bindLua();
	Light::LightMaterial::bindLua();
	Light::bindLua();

	// Character/interaction
	CharacterActivity::bindLua();
	CharacterData::bindLua();
	CharacterRelation::bindLua();
	CharacterStruct::bindLua();
	ConversationSubStruct::bindLua();
	NpcPcInteractiveConversationStruct::bindLua();
	PcConversationStruct::bindLua();
	ExtClass::ActionParamStruct::bindLua();
	CharInstData::bindLua();
	CharacterStatus::bindLua();
	NpcData::bindLua();
	NpcStatus::bindLua();

	PlayInjections::NpcActions::AnswerStruct::bindLua();

	// H
	Camera::bindLua();
	HGUIButton::bindLua();
	HInfo::bindLua();
	HParticipant::bindLua();
	HPosButtonList::bindLua();

	Poser::bindLua();
	Poser::PoserController::PoserCharacter::bindLua();
	Poser::PoserController::SliderInfo::bindLua();
	Poser::PoserController::PoserProp::bindLua();
	Render::bindLua();
	SharedInjections::ArchiveFile::bindLua();

	// Very low level utilities
	using namespace General;
	_BINDING["peek"] = lua_CFunction([](lua_State *L) {
		// buf = peek(addr, len)
		int addr = (int)lua_touserdata(L, 1);
		if (addr == 0) addr = luaL_checkinteger(L, 1);
		int scanmax = luaL_checkinteger(L, 2);	
		char *buf = (char*)alloca(scanmax);
		SIZE_T gotread;

		if (!ReadProcessMemory(GetCurrentProcess(), (void*)addr, buf, scanmax, &gotread))
			return 0;
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
		if (!addr)
			addr = (int)lua_topointer(L, 1);
		size_t nbytes;
		const char *buf = luaL_checklstring(L, 2, &nbytes);
		void *ptr = (void*)(addr);
		Memrights unprotect(ptr, nbytes);
		SIZE_T wrote = 0;
		WriteProcessMemory(GetCurrentProcess(), ptr, buf, nbytes, &wrote);
		lua_pushinteger(L, wrote);
		return 1;
	});

	_BINDING["proc_invoke"] = lua_CFunction([](lua_State *L) {
		// eax, edx = proc_invoke(addr, args...), stdcall/thiscall/cdecl only
		DWORD *argbuf = (DWORD*)alloca((lua_gettop(L) - 2)*4);
		int addr = lua_tointeger(L, 1);
		if (!addr) addr = (int)lua_topointer(L, 1);
		int _this = lua_tointeger(L, 2);
		if (!_this) _this = (int)lua_topointer(L, 2);
		for (int i = 3; i <= lua_gettop(L); i++) {
			if (lua_type(L, i) == LUA_TSTRING) {
				argbuf[i - 3] = (DWORD)lua_tostring(L, i);
			}
			else if (lua_type(L, i) == LUA_TNUMBER) {
				argbuf[i - 3] = lua_tointeger(L, i);
			}
			else {
				argbuf[i - 3] = (DWORD)lua_topointer(L, i);
			}
		}
		size_t nbytes = (lua_gettop(L)-2)*4;
		void *ptr = (void*)(addr);
		DWORD saved_eax, saved_edx;
		__asm {
			mov eax, addr
			mov edx, _this
			mov esi, argbuf
			mov ecx, nbytes
			push ebp
			mov ebp, esp
			mov edi, esp
			sub esp, ecx
			sub edi, ecx
			rep movsb
			mov ecx, edx
			call eax
			mov esp, ebp
			pop ebp
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
	using namespace PlayInjections::Loads;
	_BINDING["SetLoadOverrides"] = LUA_LAMBDA({
		s.top(3);
		s.push(g_skirtOffOverride);
		s.push(g_boobGravityOverride);
		s.push(g_eyeTracking);

		if (!s.isnil(1)) g_skirtOffOverride = s.get(1);
		if (!s.isnil(2)) g_boobGravityOverride = s.get(2);
		if (!s.isnil(3)) g_eyeTracking = s.get(3);
		return 3;
	});
	_BINDING["SetNoBraOverride"] = LUA_LAMBDA({
		BYTE index = s.get(1);
		BYTE state = s.get(2);
		g_invisibraOverride[index] = state;
	});
	_BINDING["GetCharacter"] = LUA_LAMBDA({
		int idx = s.get(1);
		if ((idx < 0) || (idx > 24) || !g_characters[idx].IsValid())
			return 0;
		s.push(g_characters[idx].m_char);
	});

	_BINDING["GetCharInstData"] = LUA_LAMBDA({
		if (General::IsAAEdit) {
			if (!AAEdit::g_currChar.Editable())
				return 0;
			s.push(&AAEdit::g_currChar);
			return 1;
		}
		else {
			int idx = s.get(1);
			if ((idx < 0) || (idx > 24) || !g_characters[idx].IsValid())
				return 0;
			s.push(&g_characters[idx]);
		}
	});


	using namespace ExtVars::AAPlay;
	_BINDING["GetGameTimeData"] = LUA_LAMBDA({
		s.push(GameTimeData());
	});
	_BINDING["GetPlayerCharacter"] = LUA_LAMBDA({
		if (General::IsAAEdit) {
			if (!AAEdit::g_currChar.Editable())
				return 0;
			s.push(AAEdit::g_currChar.m_char);
			return 1;
		}
		s.push(*PlayerCharacterPtr());
	});
	_BINDING["SetPlayerCharacter"] = LUA_LAMBDA0({
		*PlayerCharacterPtr() = s.get(1);
	});

	_BINDING["AddSubtitles"] = LUA_LAMBDA0({
		Subtitles::AddSubtitles(s.get(1), s.get(2));
	});

	_BINDING["InitSubtitlesParams"] = LUA_LAMBDA0({
		Subtitles::InitSubtitlesParams(s.get(1), s.get(2), s.get(3), s.get(4), s.get(5), s.get(6),
			s.get(7), s.get(8), s.get(9), s.get(10), s.get(11), s.get(12), s.get(13), s.get(14), s.get(15));
	});

//	_BINDING["GetPlayerConversation"] = &PlayerConversationPtr;

	// Higher level triggers
	_BINDING["SafeAddCardPoints"] = LUA_LAMBDA0({
		Shared::Triggers::SafeAddCardPoints(s.get(1), s.get(2), s.get(3), s.get(4));
	});

	_BINDING["GetCamera"] = LUA_LAMBDA({
		s.push(Camera::GetCamera());
	});

	_BINDING["SetFocusBone"] = LUA_LAMBDA0({
		Camera::SetFocusBone(s.get(1), s.get(2), s.get(3), s.get(4), s.get(5));
	});

	_BINDING["SetClassJSONData"] = LUA_LAMBDA({
		std::string key((const char*)s.get(1));
		std::string json((const char*)s.get(2));
		picojson::value v;
		std::string err = picojson::parse(v, json);
		if (!err.empty()) {
			s.nil();
			s.push(err.c_str());
			return 2;
		}
		PersistentStorage::current().set(key, v);
		s.push(true);
	});

	_BINDING["GetClassJSONData"] = LUA_LAMBDA({
		std::string key((const char*)s.get(1));
		std::string json = PersistentStorage::current().get(key).serialize();
		s.push(json.c_str());
	});

	using namespace General;
	_BINDING["SetAAPlayPath"] = LUA_LAMBDA0({
		AAPlayPath = utf8.from_bytes((const char*)s.get(1));
	});
	_BINDING["SetAAEditPath"] = LUA_LAMBDA0({
		AAEditPath = utf8.from_bytes((const char*)s.get(1));
		return 0;
	});

	_BINDING["GetGameTick"] = LUA_LAMBDA({
		s.push(GameTick::tick);
	});
	_BINDING["SetHideUI"] = LUA_LAMBDA({
		Render::g_hideUI = s.get(1);
	});

	_BINDING["GetGameHwnd"] = LUA_LAMBDA({
		s.push((DWORD)(*GameTick::hwnd));
	});


	_BINDING["SwitchUI"] = LUA_LAMBDA({
		s.push(PlayInjections::UIEvent::UiEventHook(s.get(1), s.get(2), s.get(3), s.get(4), s.get(5), s.get(6), s.get(7)));
	});

	GameTick::RegisterMsgFilter(GameTick::MsgFilterFunc([](MSG *m) {
		const char *mstr = NULL;

		switch (m->message) {
		case WM_KEYDOWN: mstr = "keydown"; break;
		case WM_KEYUP: mstr = "keyup"; break;
		case WM_CHAR: mstr = "char"; break;
		}
		if ((mstr) && (m->hwnd != *GameTick::hwnd)) return false;

		switch (m->message) {
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
			mstr = "mousedown";
			break;
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
			mstr = "mouseup";
			break;
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
			mstr = "mousedblclk";
			break;
/*		case WM_MOUSEMOVE:
			mstr = "mousemove";
			break;*/
		}

		if (mstr) {
			LUA_EVENT(mstr, m->wParam, m->lParam, (DWORD)m->hwnd, m->pt.x, m->pt.y);
			if (mstr == "keydown") {
				if (General::IsAAPlay) {
					if (Shared::GameState::getPlayerCharacter() != nullptr) {
						if (Shared::GameState::getPlayerCharacter()->IsValid()) {
							keyPressData.card = Shared::GameState::getPlayerCharacter()->m_char->m_seat;
							keyPressData.keyVal = m->wParam;
							Shared::Triggers::ThrowEvent(&keyPressData);
						}
					}
				}
			}

			if (m->wParam == -1)
				return true;
		}
		return false;
	}));

}
