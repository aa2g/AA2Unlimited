#include <Windows.h>
#include "Config.h"
#include "Logger.h"
#include "Script\ScriptLua.h"

/*
 * Lua init script actually parses config file and fills in the C++ config fields
 * defined in knownMembers. Fields which are not knownMembers are free-form, and
 * exclusively tracked by lua, and can be be queried by [ifsb]Get() methods.
 * Those Get() call answer queries for knownMember too, but must be avoided
 * in certain situations (performance, callbacks) - hence why we keep some fields
 * confined in C++ struct.
 */

const Config::MemberInfo Config::knownMembers[num_elements] = {
	MemberInfo(Config::BOOL,	"bUseAdditionalTanSlots", false),	//USE_TAN_SLOTS
	MemberInfo(Config::BOOL,	"bUseMeshTextureOverrides", false),	//USE_MESH_TEXTURE_OVERRIDES
	MemberInfo(Config::BOOL,	"bUseHAi", false),	//USE_H_AI
	MemberInfo(Config::BOOL,	"bUseShadowing", false),	//USE_SHADOWING
	MemberInfo(Config::BOOL,	"bEnableFacecam", false), //USE_H_FACECAM
	MemberInfo(Config::BOOL,	"bEnableHPosButtonReorder", false), //USE_H_POS_BUTTON_MOVE
	MemberInfo(Config::BOOL,	"bUseClothesPoser", false), //USE_POSER_CLOTHES
	MemberInfo(Config::BOOL,	"bUseDialoguePoser", false), //USE_POSER_DIALOGUE
	MemberInfo(Config::BOOL,	"bNoPromptHIsRape", false),	//NO_PROMPT_IS_FORCE
	MemberInfo(Config::BOOL,	"bHAiOnNoPromptH", false),	//HAI_ON_NO_PROMPT
	MemberInfo(Config::INT,		"savedEyeTextureUsage", 1),	//SAVED_EYE_TEXTURE_USAGE,	
	MemberInfo(Config::INT,		"savedFileUsage", 1),	////SAVED_FILE_USAGE,
	MemberInfo(Config::BOOL,	"bSaveFileAutoRemove", false),	//SAVED_FILE_REMOVE,	
	MemberInfo(Config::BOOL,	"bSaveFileBackup",true), //SAVED_FILE_BACKUP
	MemberInfo(Config::INT,		"facelessMaleSlot", 90),	//FACELESS_SLOT_MALE,	
	MemberInfo(Config::INT,		"facelessFemaleSlot", 7),	//FACELESS_SLOT_FEMALE
	MemberInfo(Config::FLOAT,	"fPOVOffsetX", 0.0f),	//POV_OFFSET_X
	MemberInfo(Config::FLOAT,	"fPOVOffsetY", 0.0f),	//POV_OFFSET_Y
	MemberInfo(Config::FLOAT,	"fPOVOffsetZ", 0.0f),	//POV_OFFSET_Z
	MemberInfo(Config::INT,		"legacyMode", 0),	//LEGACY_MODE
	MemberInfo(Config::BOOL,		"bPoserHotkeys", false),	//USE_POSER_HOTKEYS
	MemberInfo(Config::INT,		"hkPoserTranslate", 0x57),	//HKEY_POSER_TRANSLATE
	MemberInfo(Config::INT,		"hkPoserRotate", 0x45),	//HKEY_POSER_ROTATE
	MemberInfo(Config::INT,		"hkPoserScale", 0x52),	//HKEY_POSER_SCALE
	MemberInfo(Config::INT,		"screenshotFormat", 0x0)	//HKEY_POSER_SCALE
};

Config g_Config;

Config::Config() {
	//initialize with default values
	for (int i = 0; i < num_elements; i++) {
		switch (knownMembers[i].type) {
		case BOOL:
			m_members[i].bVal = knownMembers[i].data.bVal;
			break;
		case INT:
			m_members[i].iVal = knownMembers[i].data.iVal;
			break;
		case FLOAT:
			m_members[i].fVal = knownMembers[i].data.fVal;
			break;
		case STRING:
			m_members[i].sVal = knownMembers[i].data.sVal;
			break;
		}

	}
}

// API provided to lua to get/set C++ fields
int Config::luaConfig(lua_State *L) {
	const char *k = luaL_checkstring(L, 1);
	for (int i = 0; i < num_elements; i++) {
		if (strcmp(knownMembers[i].name, k)) continue;
		auto d = m_members[i];
		switch (knownMembers[i].type) {
		case INT:
			lua_pushinteger(L, d.iVal);
			if (lua_gettop(L) > 1)
				d.iVal = luaL_checkinteger(L, 2);
			return 1;
		case FLOAT:
			lua_pushnumber(L, d.fVal);
			if (lua_gettop(L) > 1)
				d.fVal = luaL_checknumber(L, 2);
			return 1;
		case STRING:
			lua_pushstring(L, d.sVal);
			if (lua_gettop(L) > 1)
				d.sVal = luaL_checkstring(L, 2);
			return 1;
		case BOOL:
			lua_pushboolean(L, d.bVal);
			if (lua_gettop(L) > 1)
				d.bVal = lua_toboolean(L, 2);
			return 1;
		}
	}
	return 0;
}

static int luaConfig_wrap(lua_State *L) {
	return g_Config.luaConfig(L);
}

// Initialize Lua config context
Config::Config(lua_State *LL) : Config() {
	this->L = LL;
	lua_register(L, "get_set_config", &luaConfig_wrap);
	lua_newtable(L);
	lua_setglobal(L, "_CF");
}

// Config value getters which reach into Lua VM
bool Config::bGet(const char *name) {
	lua_getglobal(L, "_CF");
	lua_getfield(L, -1, name);
	auto v = lua_toboolean(L, -1); lua_pop(L, 2); return v;
}
int Config::iGet(const char *name) {
	lua_getglobal(L, "_CF");
	lua_getfield(L, -1, name);
	auto v = lua_tointeger(L, -1); lua_pop(L, 2); return v;
}
double Config::fGet(const char *name) {
	lua_getglobal(L, "_CF");
	lua_getfield(L, -1, name);
	auto v = lua_tonumber(L, -1); lua_pop(L, 2); return v;
}
const char *Config::sGet(const char *name) {
	lua_getglobal(L, "_CF");
	lua_getfield(L, -1, name);
	auto v = lua_tostring(L, -1); lua_pop(L, 2); return v;
}
