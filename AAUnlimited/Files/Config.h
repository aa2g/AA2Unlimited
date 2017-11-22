#pragma once
#include "defs.h"
#include <string>
#include "Script/ScriptLua.h"

extern class Config
{
public:;

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

	bool bMTRenderer;
	int screenshotFormat;
	int savedFileUsage;
	int bLogPPAccess;
	bool bHAiOnNoPromptH;
	bool bEnableHPosButtonReorder;
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
	bool bListFilenames;
	bool bUnlimitedOnTop;
	bool bExtractOnListing;

	static inline void bindLua() {
		LUA_SCOPE;
#define LUA_CLASS Config
			LUA_BIND(bMTRenderer)
			LUA_BIND(screenshotFormat)
			LUA_BIND(bLogPPAccess)
			LUA_BIND(savedFileUsage)
			LUA_BIND(bHAiOnNoPromptH)
			LUA_BIND(bEnableHPosButtonReorder)
			LUA_BIND(bUseShadowing)
			LUA_BIND(bUseHAi)
			LUA_BIND(bUsePPeX)
			LUA_BIND(bUsePP2)
			LUA_BIND(bTriggers)
			LUA_BIND(PP2Cache)
			LUA_BIND(PP2AudioCache)
			LUA_BIND(PP2Buffers)
			LUA_BIND(bDrawFPS)
			LUA_BIND(bUseVisualStyles)
			LUA_BIND(PP2Profiling)
			LUA_BIND(bListFilenames)
			LUA_BIND(bUnlimitedOnTop)
			LUA_BIND(bExtractOnListing)

#undef LUA_CLASS
		g_Lua[LUA_BINDING_TABLE]["Config"] = &g_Config;
	}
} g_Config;


