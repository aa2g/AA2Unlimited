#pragma once

struct RDM_BUTTON{
	std::string funcName;
	std::wstring titleIngame;
	std::wstring shortDesc;
	int titleD3dKey = -1;
	int descD3dKey = -1;
	int backgrActiveD3dKey = -1;
};

struct RDM_MENU {
	bool showed = false;
	int showedType = 0; // 0 - General menu, 1 - H-scene menu
	bool nowHscene = false;
	int posX;
	int posY;
	int selectedBtnNode;
	bool showCancelMsg;
	int cancelMsgStartTime;
	std::wstring fontFamily;
	double fontSizeMultiplier;
	int deadzone;
	int inCircleD3dKey = -1;
	int outCircleD3dKey = -1;
	int defaultDescD3dKey = -1;
	int cancelMsgTextD3dKey = -1;
	int cancelMsgBackgrD3dKey = -1;
};

namespace RadialMenu {
	extern bool enabled;

	extern void AddButton(int buttons_arr_node, const char * func_name,
		const char * title_ingame, const char * short_desc);
	extern void SetHstatus(bool current_status);
	extern void InitRadialMenuParams(const char *font_family, int mini_version, 
		int font_size, int deadzone, int cancel_time, int toggle_type, 
		const char * default_desc, const char* canceled_button_text);
	extern void CreateHUD(bool force_create = false);
	extern void Render();
}
