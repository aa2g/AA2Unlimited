#pragma once

#include <Windows.h>
class GuiSlider
{
public:
	GuiSlider();
	GuiSlider(HWND parent,const TCHAR* label,int xStart,int yStart,float min,float max);
	GuiSlider(GuiSlider&& rhs);
	GuiSlider& operator=(GuiSlider&& rhs);
	~GuiSlider();

	void Sync(bool useEdit);

	float Sld2Val(int sld);
	int Val2Sld(float val);

	inline HWND GetEdit() const { return m_edValue; }
	inline HWND GetSlider() const { return m_slValue; }
	inline HWND GetLabel() const { return m_stLabel; }
	inline float GetCurrVal() const { return m_curr; }
private:
	int m_x;
	int m_y;
	float m_min;
	float m_max;
	float m_curr;

	HWND m_parent;
	HWND m_stLabel;
	HWND m_edValue;
	HWND m_slValue;
};

