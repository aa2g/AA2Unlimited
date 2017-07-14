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


Lua g_Lua;

Lua::Lua(bool libs) : sel::State(libs) {
	HandleExceptionsWith([](int n, std::string msg, std::exception_ptr) {
		LOGPRIONC(Logger::Priority::ERR) msg << "\r\n";
	});
	Lua &l = *this;

	if (!libs) return;

	l(LUA_BINDING_TABLE " = {}");
	l(LUA_EVENTS_TABLE " = {}");

	g_Logger.bindLua();
	g_Config.bindLua();
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

	// H
	HCamera::bindLua();
	HGUIButton::bindLua();
	HInfo::bindLua();
	HParticipant::bindLua();
	HPosButtonList::bindLua();
	HStatistics::bindLua();

	using namespace AAPlay;
	g_Lua[LUA_BINDING_TABLE]["ApplyRelationshipPoints"] = &ApplyRelationshipPoints;
	g_Lua[LUA_BINDING_TABLE]["GetCharacter"] = [](int idx) {
		if ((idx < 0) || (idx > 25))
			return decltype(g_characters[0].m_char)(0);
		return g_characters[idx].m_char;
	};


	using namespace ExtVars::AAPlay;
	g_Lua[LUA_BINDING_TABLE]["GetGameTimeData"] = GameTimeData();
	g_Lua[LUA_BINDING_TABLE]["GetPlayerCharacter"] = []() {
		return *PlayerCharacterPtr();
	};
	g_Lua[LUA_BINDING_TABLE]["GetPlayerConversation"] = &PlayerConversationPtr;


	using namespace General;
	g_Lua[LUA_BINDING_TABLE]["GameBase"] = unsigned(GameBase);
	g_Lua[LUA_BINDING_TABLE]["IsAAPlay"] = IsAAPlay;
	g_Lua[LUA_BINDING_TABLE]["IsAAEdit"] = IsAAEdit;
	g_Lua[LUA_BINDING_TABLE]["AAEditPath"] = utf8.to_bytes(AAEditPath);
	g_Lua[LUA_BINDING_TABLE]["AAPlayPath"] = utf8.to_bytes(AAPlayPath);
	g_Lua[LUA_BINDING_TABLE]["GameExeName"] = utf8.to_bytes(GameExeName);


}
