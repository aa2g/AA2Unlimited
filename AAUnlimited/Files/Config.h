#pragma once
#include "defs.h"
#include <string>
#include "Script/ScriptLua.h"

extern class Config
{
public:;
	int screenshotFormat;
	const char *sPoserHotKeys;
	int legacyMode;
	double fPOVOffsetZ;
	double fPOVOffsetY;
	double fPOVOffsetX;
	bool bSaveFileBackup;
	bool bSaveFileAutoRemove;
	int savedFileUsage;
	int savedEyeTextureUsage;
	bool bHAiOnNoPromptH;
	bool bUseDialoguePoser;
	bool bUseClothesPoser;
	bool bEnableHPosButtonReorder;
	bool bEnableFacecam;
	bool bUseShadowing;
	bool bUseHAi;
	bool bUsePPeX;

	inline auto operator[](const char *name) const {
		return g_Lua[LUA_CONFIG_TABLE][name];
	}

	static inline void bindLua() {
#define LUA_CLASS Config
		LUA_EXTCLASS(Config,
			LUA_FIELD(screenshotFormat),
			LUA_FIELD(sPoserHotKeys),
			LUA_FIELD(fPOVOffsetX),
			LUA_FIELD(fPOVOffsetY),
			LUA_FIELD(fPOVOffsetZ),
			LUA_FIELD(bSaveFileBackup),
			LUA_FIELD(bSaveFileAutoRemove),
			LUA_FIELD(savedFileUsage),
			LUA_FIELD(bHAiOnNoPromptH),
			LUA_FIELD(bUseDialoguePoser),
			LUA_FIELD(bUseClothesPoser),
			LUA_FIELD(bEnableHPosButtonReorder),
			LUA_FIELD(bEnableFacecam),
			LUA_FIELD(bUseShadowing),
			LUA_FIELD(bUseHAi),
			LUA_FIELD(bUsePPeX)

		);
#undef LUA_CLASS
		g_Lua[LUA_BINDING_TABLE]["Config"] = &g_Config;
	}
} g_Config;


