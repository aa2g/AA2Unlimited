// The rare times when C COM interfaces are useful - overloading only certain
// functions without having to make wrappers for everything.
#define CINTERFACE


#include <windows.h>
#include <d3d9.h>
#include "RenderWrap.h"

namespace Render {
IDirect3D9Vtbl   *intf;
IDirect3DDevice9Vtbl   *dev;

HRESULT WINAPI intf_QueryInterface(IDirect3D9 *self, REFIID riid, void ** ppvObj) {
	return (HRESULT)self;
}

HRESULT  WINAPI intf_CreateDevice(IDirect3D9 *self, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) {
	HRESULT hr = intf->CreateDevice(self, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
	if (hr == D3D_OK)
		WrapDevice(*ppReturnedDeviceInterface);
	return hr;
}

HRESULT WINAPI dev_QueryInterface(IDirect3DDevice9 *self, REFIID riid, void ** ppvObj) {
	return (HRESULT)self;
}

HRESULT WINAPI BeginScene(IDirect3DDevice9 *self) {
	return dev->BeginScene(self);
}

HRESULT WINAPI EndScene(IDirect3DDevice9 *self) {
	return dev->EndScene(self);
}

void *WrapInterface(void *p) {
	auto pp = (IDirect3D9*)p;
	IDirect3D9Vtbl *mytbl;
	mytbl = new IDirect3D9Vtbl;
	*mytbl = *pp->lpVtbl;
	intf = pp->lpVtbl;
	pp->lpVtbl = mytbl;
	mytbl->QueryInterface = intf_QueryInterface;
	return p;
}

void *WrapDevice(void *p) {
	auto pp = (IDirect3DDevice9*)p;
	IDirect3DDevice9Vtbl *mytbl;
	mytbl = new IDirect3DDevice9Vtbl;
	*mytbl = *pp->lpVtbl;
	dev = pp->lpVtbl;
	pp->lpVtbl = mytbl;
	mytbl->QueryInterface = dev_QueryInterface;
	return p;
}


}