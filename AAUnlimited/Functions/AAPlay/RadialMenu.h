#pragma once

struct RDM_BUTTON{
	const char* funcName;
	const char* titleIngame;
	const char* shortDesc;
	int titleD3dKey;
	int descD3dKey;
	int backgrActiveD3dKey;
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
	const char * fontFamily;
	double fontSizeMultiplier;
	int deadzone;
	int inCircleD3dKey;
	int outCircleD3dKey;
	int defaultDescD3dKey;
	int cancelMsgTextD3dKey;
	int cancelMsgBackgrD3dKey;
};

namespace RadialMenu {
	extern bool enabled;

	//void AddNotification(const char *text, int type_id);
	extern void AddButton(int buttons_arr_node, const char * func_name,
		const char * title_ingame, const char * short_desc);
	extern void SetHstatus(bool current_status);
	extern void InitRadialMenuParams(const char *font_family, int mini_version, 
		int font_size, int deadzone, int cancel_time, int toggle_type, 
		const char * default_desc, const char* canceled_button_text);
	extern void CreateHUD();
	extern void Render();
}
