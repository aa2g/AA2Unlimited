#pragma once
#include "defs.h"
#include <string>
#include "Script/ScriptLua.h"

extern class Config
{
public:;
	bool bMTRenderer;
	int screenshotFormat;
	char sPoserHotKeys[16];
	int legacyMode;
	bool bSaveFileBackup;
	bool bSaveFileAutoRemove;
	int savedFileUsage;
	int savedEyeTextureUsage;
	bool bLogPPAccess;
	bool bHAiOnNoPromptH;
	bool bUseDialoguePoser;
	bool bUseClothesPoser;
	bool bEnableHPosButtonReorder;
	bool bEnableFacecam;
	bool bUseShadowing;
	bool bUseHAi;
	bool bUsePPeX;
	bool bUsePP2;
	bool bTriggers;
	unsigned PP2Cache;
	unsigned PP2AudioCache;
	unsigned PP2Buffers;
	bool bDrawFPS;
	bool bUseVisualStyles;
	bool PP2Profiling;

	// templates can't deduce on return type, so we must do it like this
	const char *gets(const char *name) {
		LUA_SCOPE;
		return g_Lua[LUA_CONFIG_TABLE][name];
	}

	const bool getb(const char *name) {
		LUA_SCOPE;
		return g_Lua[LUA_CONFIG_TABLE][name];
	}

	const int geti(const char *name) {
		LUA_SCOPE;
		return g_Lua[LUA_CONFIG_TABLE][name];
	}

	const float getf(const char *name) {
		LUA_SCOPE;
		return g_Lua[LUA_CONFIG_TABLE][name];
	}

	static inline void bindLua() {
		LUA_SCOPE;
#define LUA_CLASS Config
			LUA_BIND(bMTRenderer)
			LUA_BIND(screenshotFormat)
			LUA_BINDSTR(sPoserHotKeys)
			LUA_BIND(bLogPPAccess)
			LUA_BIND(bSaveFileBackup)
			LUA_BIND(bSaveFileAutoRemove)
			LUA_BIND(savedFileUsage)
			LUA_BIND(savedEyeTextureUsage)
			LUA_BIND(bHAiOnNoPromptH)
			LUA_BIND(bUseDialoguePoser)
			LUA_BIND(bUseClothesPoser)
			LUA_BIND(bEnableHPosButtonReorder)
			LUA_BIND(bEnableFacecam)
			LUA_BIND(bUseShadowing)
			LUA_BIND(bUseHAi)
			LUA_BIND(bUsePPeX)
			LUA_BIND(bUsePP2)
			LUA_BIND(bTriggers)
			LUA_BIND(PP2Cache)
			LUA_BIND(PP2AudioCache)
			LUA_BIND(PP2Buffers)
			LUA_BIND(legacyMode)
			LUA_BIND(bDrawFPS)
			LUA_BIND(bUseVisualStyles)
			LUA_BIND(PP2Profiling)

#undef LUA_CLASS
		g_Lua[LUA_BINDING_TABLE]["Config"] = &g_Config;
	}
} g_Config;


