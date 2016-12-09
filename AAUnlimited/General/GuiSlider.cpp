#include "GuiSlider.h"

#include <Windows.h>
#include <CommCtrl.h>

#include "resource.h"
#include "General\ModuleInfo.h"
#include "General\Util.h"

GuiSlider::GuiSlider() : m_parent(NULL) {}


GuiSlider::GuiSlider(HWND parent,const TCHAR* label,int xStart,int yStart,float min,float max) 
	: m_parent(parent), m_x(xStart), m_y(yStart), m_min(min), m_max(max)
{
	//TODO: generalize the templates
	HWND templateStatic = GetDlgItem(parent,IDC_PPS_STEXAMPLE);
	HWND templateSlider = GetDlgItem(parent,IDC_PPS_SLDEXAMPLE);
	HWND templateEdit = GetDlgItem(parent,IDC_PPS_EDEXAMPLE);

	this->m_stLabel = CreateWindowEx(0,TEXT("STATIC"),label,WS_CHILD | WS_VISIBLE,
		0,0,0,0,
		parent,0,General::DllInst,0);
	this->m_slValue = CreateWindowEx(0,TEXT("msctls_trackbar32"),label,WS_CHILD | WS_VISIBLE | TBS_BOTH | TBS_NOTICKS | WS_TABSTOP,
		0,0,0,0,
		parent,0,General::DllInst,0);
	this->m_edValue = CreateWindowEx(WS_EX_CLIENTEDGE,TEXT("EDIT"),label,WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
		0,0,0,0,
		parent,0,General::DllInst,0);

	using namespace General;

	auto CopyStyleFromWindow = [](HWND to,HWND from) {
		LONG exStyles = GetWindowLong(from,GWL_EXSTYLE);
		LONG styles = GetWindowLong(from,GWL_STYLE) | WS_VISIBLE;
		SetWindowLong(to,GWL_EXSTYLE,exStyles);
		SetWindowLong(to,GWL_STYLE,styles);
		HFONT font = (HFONT)SendMessage(from,WM_GETFONT,0,0);
		SendMessage(to,WM_SETFONT,(WPARAM)font,FALSE);
	};

	//adjust style from templace
	CopyStyleFromWindow(m_stLabel,templateStatic);
	CopyStyleFromWindow(m_slValue,templateSlider);
	CopyStyleFromWindow(m_edValue,templateEdit);

	//move window according to template
	RECT rctTmplStatic,rctTmplSlider,rctTmplEdit;
	GetWindowRect(templateStatic,&rctTmplStatic);
	GetWindowRect(templateSlider,&rctTmplSlider);
	GetWindowRect(templateEdit,&rctTmplEdit);
	//get top left corner
	LONG left = rctTmplStatic.left,top = min(rctTmplStatic.top,min(rctTmplSlider.top,rctTmplEdit.top));
	//adjust top left corner to (xStart|yStart)
	RectMoveBy(rctTmplStatic,-left+xStart,-top+yStart);
	RectMoveBy(rctTmplSlider,-left+xStart,-top+yStart);
	RectMoveBy(rctTmplEdit,-left+xStart,-top+yStart);
	//move actual window
	MoveWindowRect(m_stLabel,rctTmplStatic,FALSE);
	MoveWindowRect(m_slValue,rctTmplSlider,FALSE);
	MoveWindowRect(m_edValue,rctTmplEdit,FALSE);

	int ret = SendMessage(this->m_slValue,TBM_SETRANGEMIN,TRUE,0);
	ret = SendMessage(this->m_slValue,TBM_SETRANGEMAX,TRUE,0x10000);
}

GuiSlider::GuiSlider(GuiSlider&& rhs) {
	m_x = rhs.m_x;
	m_y = rhs.m_y;
	m_min = rhs.m_min;
	m_max = rhs.m_max;
	m_curr = rhs.m_curr;

	m_parent = rhs.m_parent;
	rhs.m_parent = NULL;
	m_stLabel = rhs.m_stLabel;
	rhs.m_stLabel = NULL;
	m_edValue = rhs.m_edValue;
	rhs.m_edValue = NULL;
	m_slValue = rhs.m_slValue;
	rhs.m_slValue = NULL;
}

GuiSlider & GuiSlider::operator=(GuiSlider && rhs) {
	m_x = rhs.m_x;
	m_y = rhs.m_y;
	m_min = rhs.m_min;
	m_max = rhs.m_max;
	m_curr = rhs.m_curr;

	m_parent = rhs.m_parent;
	rhs.m_parent = NULL;
	m_stLabel = rhs.m_stLabel;
	rhs.m_stLabel = NULL;
	m_edValue = rhs.m_edValue;
	rhs.m_edValue = NULL;
	m_slValue = rhs.m_slValue;
	rhs.m_slValue = NULL;
	return *this;
}

void GuiSlider::Sync(bool useEdit) {
	if (useEdit) {
		//sync slider with edit
		m_curr = General::GetEditFloat(m_edValue);
		int ret = SendMessage(m_slValue,TBM_SETPOS,TRUE,Val2Sld(m_curr));
		ret++;
	}
	else {
		//sync edit with slider
		int pos = SendMessage(m_slValue,TBM_GETPOS,0,0);
		m_curr = Sld2Val(pos);
		TCHAR number[52];
		swprintf_s(number,TEXT("%f"),m_curr);
		SendMessage(m_edValue,WM_SETTEXT,0,(LPARAM)number);
	}
}

float GuiSlider::Sld2Val(int sld) {
	return m_min + (m_max-m_min)/0x10000 * sld;
}

int GuiSlider::Val2Sld(float val) {
	if (val < m_min || val > m_max) return 0;
	float max = m_max - m_min; //map from [min,max] to [0,max]
	val -= m_min;
	float coeff = 0x10000 / max;
	return int(coeff * val);
}

GuiSlider::~GuiSlider() {
	DestroyWindow(m_edValue);
	DestroyWindow(m_slValue);
	DestroyWindow(m_stLabel);
}
