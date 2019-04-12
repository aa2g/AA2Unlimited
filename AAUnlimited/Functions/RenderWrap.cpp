// This file is ugly, because DX is ugly. I'm totally not to blame --ez


#define INITGUID

#define QSIZE 1024*1024
#define QLIMIT 768*1024

#include <windows.h>
#include <d3d9.h>
#include <thread>
#include <mutex>
#include "RenderWrap.h"
#include "MemMods/MemRightsLock.h"
#include "Files/Logger.h"
#include "Files/Config.h"
#include "Render.h"
#include "External/ExternalClasses/CharacterStruct.h"
#include "Functions/AAPlay/Subs.h"


#define FRAME_MASK 15
#define TEST_DISABLE
namespace Render {
bool g_hideUI;

DWORD frameno;

static DWORD tickmap[1024];
static DWORD again = 0;
static DWORD nvert, nprim, drawcalls=1;

enum {
	VERTEX_SHADER,
	PIXEL_SHADER,

	VERTEX_SHADER_CONST,
	PIXEL_SHADER_CONST,

	DRAW_PRIMITIVES,
};

struct WorkItem {
	int op;
	union {
		struct Constant {
			UINT StartRegister;
			UINT Vector4fCount;
			float data[0];
		} c;
		struct Draw {
			D3DPRIMITIVETYPE PrimitiveType;
			INT BaseVertexIndex;
			UINT MinVertexIndex;
			UINT NumVertices;
			UINT startIndex;
			UINT primCount;
		} d;
		IDirect3DVertexShader9* v;
		IDirect3DPixelShader9* p;
	};
};

#define LOGCALL LOGPRIO(Logger::Priority::SPAM) << "Called!" << "\n"

static void tickdump(DWORD now) {
	if (now < again) return;
	LOGSPAM << "DXUSAGE: \n";
	for (int i = 0; i < 1024; i++) {
		if (tickmap[i]) {
			LOGSPAM << std::dec << "#" << i << " " << tickmap[i] << "ms\n";
			tickmap[i] = 0;
		}
	} 
	LOGSPAM << std::dec << "nvert=" << nvert << ", nprim=" << nprim << " draws=" << drawcalls << "\r\n";
	nvert = nprim = 0;
	again = now + 10000;
	drawcalls = 1;

}

#if 0
#define WRAPCALL(x) { \
	int lineno = __LINE__; \
	DWORD begin = GetTickCount(); \
	auto ret = x; \
	tickmap[lineno] += GetTickCount() - begin; \
	tickdump(begin); \
	return ret; \
}

#define WRAPCALLV(x) { \
	int lineno = __LINE__; \
	DWORD begin = GetTickCount(); \
	x; \
	tickmap[lineno] += GetTickCount() - begin; \
	tickdump(begin); \
}
#define DWRAPCALL(x) { \
	return x; \
}

#else

#define WRAPCALL(x) { \
	if (!g_Config.bMTRenderer) return x; \
    while (play < record) Sleep(1); \
	{ \
std::unique_lock<std::mutex> lock(mainthread); \
return x; \
	} \
}

#define WRAPCALLV(x) { \
	if (!g_Config.bMTRenderer) return x; \
	while (play < record) Sleep(1); \
	{ \
std::unique_lock<std::mutex> lock(mainthread); \
return x; \
	} \
}

#define DWRAPCALL(x) { \
	return x; \
}

#endif

/*#define WRAPCALL(x) return x;
#define WRAPCALLV(x) x;*/


IDirect3DDevice9 *d3dev;

class AAUIDirect3DDevice9 : public IDirect3DDevice9 {
public:;
	DWORD render_begin;
	ULONG ref;
	IDirect3DDevice9 *orig;
	std::mutex mainthread;
	std::mutex workq;

	std::thread worker;
	std::condition_variable condition;
	volatile size_t play;
	volatile size_t record;
	bool exiting;
	char buf[QSIZE];
	IUnknown *font;
	void *(WINAPI *DrawText)(IUnknown *, void*, LPCTSTR, int, LPRECT, DWORD, D3DCOLOR);
	double real_time;

	void DrawFPS() {
		RECT rekt = { 0,0,256,64 };
		wchar_t buf[64];

		// every FRAME_MASK
		if ((frameno & FRAME_MASK) == 0) {
			DWORD now = GetTickCount();
			real_time = (real_time + (now - render_begin)) / 2.0;
			render_begin = now;
		}

		_swprintf(buf, L"%02.2lf", 1000.0 * (FRAME_MASK + 1) / (real_time + 1));
		//, real_time / (FRAME_MASK+1)
		DrawText(font, 0, buf, -1, &rekt, DT_LEFT, 0xFFFFFFFF);
	}

	void DrawSubs() {
		Subtitles::PopSubtitles();
		if (!Subtitles::lines.empty()) {

			if (Subtitles::outlineLayersCount != 0 || Subtitles::separateColorMale)
			{
				int line_num = 0;
				for each (const auto line in Subtitles::lines) // for each subs line
				{
					int top_offset = Subtitles::lineHeight * line_num;
					RECT *tempRect;
					for (int i = 0; i < Subtitles::outlineLayersCount; i++) // outline layers
					{
						tempRect = &Subtitles::rect[i];
						tempRect->top = tempRect->top + top_offset;
						tempRect->bottom = tempRect->bottom + top_offset;
						DrawText(font, 0, std::get<0>(line).c_str(), -1, tempRect, DT_NOCLIP | Subtitles::subsCentered, Subtitles::colors[0]);
						tempRect->top = tempRect->top - top_offset;
						tempRect->bottom = tempRect->bottom - top_offset;
					}
					// Colorized text
					tempRect = &Subtitles::rect[Subtitles::fontLayersCount - 1];
					tempRect->top = tempRect->top + top_offset;
					tempRect->bottom = tempRect->bottom + top_offset;
					DrawText(font, 0, std::get<0>(line).c_str(), -1, &Subtitles::rect[Subtitles::fontLayersCount - 1], DT_NOCLIP | Subtitles::subsCentered, Subtitles::colors[std::get<1>(line)]);
					tempRect->top = tempRect->top - top_offset;
					tempRect->bottom = tempRect->bottom - top_offset;

					line_num++;
				}
			}
			else { // Only Colorized text
				Subtitles::text.clear();
				for each (const auto line in Subtitles::lines)
					Subtitles::text += std::get<0>(line);
				DrawText(font, 0, Subtitles::text.c_str(), -1, &Subtitles::rect[Subtitles::fontLayersCount - 1], DT_NOCLIP | Subtitles::subsCentered, Subtitles::colors[1]);
			}
		}
	}

	void MakeFont() {
		// fuck you microsoft for the d3dx9 SDK stupidity, no way im installing that shit
		font = 0;

		const char *text = Subtitles::fontFamily;
		int fontSize = Subtitles::fontSize;

		if (text && fontSize) {
			std::wstring fontName = General::utf8.from_bytes(text);

			HMODULE hm = GetModuleHandleA("d3dx9_42");
			void *(WINAPI *D3DXCreateFont)(
				IDirect3DDevice9 *pDevice,
				INT               Height,
				UINT              Width,
				UINT              Weight,
				UINT              MipLevels,
				BOOL              Italic,
				DWORD             CharSet,
				DWORD             OutputPrecision,
				DWORD             Quality,
				DWORD             PitchAndFamily,
				LPCTSTR           pFacename,
				IUnknown        **ppFont
				);
			D3DXCreateFont = decltype(D3DXCreateFont)(GetProcAddress(hm, "D3DXCreateFontW"));

			D3DXCreateFont(orig, fontSize, 0, FW_ULTRABOLD, 1, false, DEFAULT_CHARSET,
				OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontName.c_str(), &font);

			if (!font) return;

			DrawText = decltype(DrawText)(((void***)font)[0][15]);
		}
	}

	AAUIDirect3DDevice9(IDirect3DDevice9* old) {
		play = record = 0;
		orig = old;
		ref = 1;
		exiting = false;

		MakeFont();

		if (!g_Config.bMTRenderer)
			return;

		std::thread w([this] {
//			LOGSPAM << "Worker thread running\n";
			for (;;)
				if (this->render_thread())
					return;
		});
		worker.swap(w);
	}

#define SIZEOF(base,mem) (intptr_t(&mem) - intptr_t(base) + sizeof(mem))
	bool render_thread() {
		while (play == record && (!exiting))
			Sleep(1);
		if (exiting)
			return true;

		mainthread.lock();
		while (play < record)
			play += replay_work();
		if (record > QLIMIT) {
			play = record = 0;
		}
		mainthread.unlock();
		return false;
	}

	size_t replay_work() {
		// Now we replay the queue, hoping the main thread is busy doing something else
		WorkItem *wi = (WorkItem*)(buf + play);
		switch (wi->op) {
		case PIXEL_SHADER:
			orig->SetPixelShader(wi->p);
			return SIZEOF(wi, wi->p);
		case VERTEX_SHADER:
			orig->SetVertexShader(wi->v);
			return SIZEOF(wi, wi->v);
		case VERTEX_SHADER_CONST:
			orig->SetVertexShaderConstantF(wi->c.StartRegister, wi->c.data, wi->c.Vector4fCount);
			return SIZEOF(wi, wi->c) + wi->c.Vector4fCount * 4;
		case PIXEL_SHADER_CONST:
			orig->SetPixelShaderConstantF(wi->c.StartRegister, wi->c.data, wi->c.Vector4fCount);
			return SIZEOF(wi, wi->c) + wi->c.Vector4fCount * 4;
		case DRAW_PRIMITIVES:
			orig->DrawIndexedPrimitive(wi->d.PrimitiveType, wi->d.BaseVertexIndex, wi->d.MinVertexIndex, wi->d.NumVertices, wi->d.startIndex, wi->d.primCount);
			return SIZEOF(wi, wi->d);
		}
		// forces crash
		return 0xdeadbae;
	}
	HRESULT WINAPI DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
	{
		TEST_DISABLE;
		nvert += NumVertices;
		nprim += primCount;
		drawcalls++;

		if (g_Config.bMTRenderer && record < QLIMIT) {
			WorkItem *wi = (WorkItem*)(buf + record);
			wi->op = DRAW_PRIMITIVES;
			wi->d.PrimitiveType = PrimitiveType;
			wi->d.BaseVertexIndex = BaseVertexIndex;
			wi->d.MinVertexIndex = MinVertexIndex;
			wi->d.NumVertices = NumVertices;
			wi->d.startIndex = startIndex;
			wi->d.primCount = primCount;
			record += SIZEOF(wi, wi->d);
			return NOERROR;
		}

		WRAPCALL(orig->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount));
	}

	HRESULT WINAPI SetPixelShader(IDirect3DPixelShader9* pShader)
	{
		if (g_Config.bMTRenderer && record < QLIMIT) {
			WorkItem *wi = (WorkItem*)(buf + record);
			wi->op = PIXEL_SHADER;
			wi->p = pShader;
			record += SIZEOF(wi, wi->p);
			return NOERROR;
		}
		WRAPCALL(orig->SetPixelShader(pShader));
	}

	HRESULT WINAPI SetVertexShader(IDirect3DVertexShader9* pShader)
	{
		if (g_Config.bMTRenderer && record < QLIMIT) {
			WorkItem *wi = (WorkItem*)(buf + record);
			wi->op = VERTEX_SHADER;
			wi->v = pShader;
			record += SIZEOF(wi, wi->v);
			return NOERROR;
		}
		WRAPCALL(orig->SetVertexShader(pShader));
	}

	HRESULT WINAPI SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
	{
		TEST_DISABLE;
		if (g_Config.bMTRenderer && record < QLIMIT) {
			WorkItem *wi = (WorkItem*)(buf + record);
			wi->op = VERTEX_SHADER_CONST;
			wi->c.StartRegister = StartRegister;
			wi->c.Vector4fCount = Vector4fCount;
			memcpy(wi->c.data, pConstantData, Vector4fCount * 4);
			record += SIZEOF(wi, wi->c) + Vector4fCount * 4;
			return NOERROR;
		}
		WRAPCALL(orig->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount));
	}

	HRESULT WINAPI SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
	{
		TEST_DISABLE;
		if (g_Config.bMTRenderer && record < QLIMIT) {
			WorkItem *wi = (WorkItem*)(buf + record);
			wi->op = PIXEL_SHADER_CONST;
			wi->c.StartRegister = StartRegister;
			wi->c.Vector4fCount = Vector4fCount;
			memcpy(wi->c.data, pConstantData, Vector4fCount * 4);
			record += SIZEOF(wi, wi->c) + Vector4fCount * 4;
			return NOERROR;
		}
		WRAPCALL(orig->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount));
	}


	virtual ~AAUIDirect3DDevice9(void) {
		if (!g_Config.bMTRenderer)
			return;

		{
			std::unique_lock<std::mutex> lock(workq);
			exiting = true;
		}
		condition.notify_one();
		worker.join();
	}

	HRESULT WINAPI QueryInterface(REFIID riid, void ** ppvObj) {
		HRESULT hres = orig->QueryInterface(riid, ppvObj);
		if (hres == NOERROR && (IsEqualGUID(riid, IID_IDirect3DDevice9) || IsEqualGUID(riid, IID_IUnknown))) {
			ref++;
			orig->AddRef();
			*ppvObj = this;
		}
		return hres;
	}


	ULONG WINAPI AddRef(void)
	{
		ref++;
		return orig->AddRef();
	}

	ULONG WINAPI Release(void) {
		ULONG count = orig->Release();
		if (!count) {
//			LOGSPAM << "Releasing! ref=" << ref << "\n";
			if (font)
				font->Release();
			delete this;
		}
		return count;
	}


	HRESULT WINAPI TestCooperativeLevel(void)
	{
		WRAPCALL(orig->TestCooperativeLevel());
	}

	UINT WINAPI GetAvailableTextureMem(void)
	{
		WRAPCALL(orig->GetAvailableTextureMem());
	}

	HRESULT WINAPI EvictManagedResources(void)
	{
		WRAPCALL(orig->EvictManagedResources());
	}

	HRESULT WINAPI GetDirect3D(IDirect3D9** ppD3D9)
	{
		WRAPCALL(orig->GetDirect3D(ppD3D9));
	}

	HRESULT WINAPI GetDeviceCaps(D3DCAPS9* pCaps)
	{
		WRAPCALL(orig->GetDeviceCaps(pCaps));
	}

	HRESULT WINAPI GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode)
	{
		WRAPCALL(orig->GetDisplayMode(iSwapChain, pMode));
	}

	HRESULT WINAPI GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
	{
		WRAPCALL(orig->GetCreationParameters(pParameters));
	}


	HRESULT WINAPI SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap)
	{
		WRAPCALL(orig->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap));
	}

	void WINAPI SetCursorPosition(int X, int Y, DWORD Flags)
	{
		WRAPCALLV(orig->SetCursorPosition(X, Y, Flags));
	}

	BOOL WINAPI ShowCursor(BOOL bShow)
	{
		WRAPCALL(orig->ShowCursor(bShow));
	}

	HRESULT WINAPI CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain)
	{
		WRAPCALL(orig->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain));
	}

	HRESULT WINAPI GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain)
	{
		WRAPCALL(orig->GetSwapChain(iSwapChain, pSwapChain));
	}

	UINT WINAPI GetNumberOfSwapChains(void)
	{
		WRAPCALL(orig->GetNumberOfSwapChains());
	}

	HRESULT WINAPI Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		WRAPCALL(orig->Reset(pPresentationParameters));
	}

	HRESULT WINAPI Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
	{
		WRAPCALL(orig->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion));
	}

	HRESULT WINAPI GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer)
	{
		WRAPCALL(orig->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer));
	}

	HRESULT WINAPI GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus)
	{
		WRAPCALL(orig->GetRasterStatus(iSwapChain, pRasterStatus));
	}

	HRESULT WINAPI SetDialogBoxMode(BOOL bEnableDialogs)
	{
		WRAPCALL(orig->SetDialogBoxMode(bEnableDialogs));
	}

	void WINAPI SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp)
	{
		WRAPCALLV(orig->SetGammaRamp(iSwapChain, Flags, pRamp));
	}

	void WINAPI GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp)
	{
		WRAPCALLV(orig->GetGammaRamp(iSwapChain, pRamp));
	}

	HRESULT WINAPI CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle)
	{
		WRAPCALL(orig->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle));
	}

	HRESULT WINAPI CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle)
	{
		WRAPCALL(orig->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle));
	}

	HRESULT WINAPI CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle)
	{
		WRAPCALL(orig->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle));
	}

	HRESULT WINAPI CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle)
	{
		WRAPCALL(orig->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle));
	}

	HRESULT WINAPI CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle)
	{
		WRAPCALL(orig->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle));
	}

	HRESULT WINAPI CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
	{
		WRAPCALL(orig->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle));
	}

	HRESULT WINAPI CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
	{
		WRAPCALL(orig->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle));
	}

	HRESULT WINAPI UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint)
	{
		WRAPCALL(orig->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint));
	}

	HRESULT WINAPI UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture)
	{
		WRAPCALL(orig->UpdateTexture(pSourceTexture, pDestinationTexture));
	}

	HRESULT WINAPI GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface)
	{
		WRAPCALL(orig->GetRenderTargetData(pRenderTarget, pDestSurface));
	}

	HRESULT WINAPI GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface)
	{
		WRAPCALL(orig->GetFrontBufferData(iSwapChain, pDestSurface));
	}

	HRESULT WINAPI StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
	{
		WRAPCALL(orig->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter));
	}

	HRESULT WINAPI ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color)
	{
		WRAPCALL(orig->ColorFill(pSurface, pRect, color));
	}

	HRESULT WINAPI CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
	{
		WRAPCALL(orig->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle));
	}

	HRESULT WINAPI SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
	{
		WRAPCALL(orig->SetRenderTarget(RenderTargetIndex, pRenderTarget));
	}

	HRESULT WINAPI GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
	{
		WRAPCALL(orig->GetRenderTarget(RenderTargetIndex, ppRenderTarget));
	}

	HRESULT WINAPI SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
	{
		WRAPCALL(orig->SetDepthStencilSurface(pNewZStencil));
	}

	HRESULT WINAPI GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
	{
		WRAPCALL(orig->GetDepthStencilSurface(ppZStencilSurface));
	}

	HRESULT WINAPI BeginScene(void)
	{
		ExtClass::CharacterStruct::ApplyAnimData();
		WRAPCALL(orig->BeginScene());
	}

	HRESULT WINAPI EndScene(void)
	{
		//onEndScene();
		if (font && g_Config.bDrawFPS || g_Config.bDisplaySubs) {
			D3DVIEWPORT9 vp;
			GetViewport(&vp);
			if (vp.Width > 1024) {
				if (g_Config.bDrawFPS)
					DrawFPS();
				if (g_Config.bDisplaySubs)
					DrawSubs();
			}
		}
		frameno++;
		WRAPCALL(orig->EndScene());
	}

	HRESULT WINAPI Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
	{
		WRAPCALL(orig->Clear(Count, pRects, Flags, Color, Z, Stencil));
	}

	HRESULT WINAPI SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
	{
		WRAPCALL(orig->SetTransform(State, pMatrix));
	}

	HRESULT WINAPI GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix)
	{
		WRAPCALL(orig->GetTransform(State, pMatrix));
	}

	HRESULT WINAPI MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
	{
		WRAPCALL(orig->MultiplyTransform(State, pMatrix));
	}

	HRESULT WINAPI SetViewport(CONST D3DVIEWPORT9* pViewport)
	{
		WRAPCALL(orig->SetViewport(pViewport));
	}

	HRESULT WINAPI GetViewport(D3DVIEWPORT9* pViewport)
	{
		WRAPCALL(orig->GetViewport(pViewport));
	}

	HRESULT WINAPI SetMaterial(CONST D3DMATERIAL9* pMaterial)
	{
		WRAPCALL(orig->SetMaterial(pMaterial));
	}

	HRESULT WINAPI GetMaterial(D3DMATERIAL9* pMaterial)
	{
		WRAPCALL(orig->GetMaterial(pMaterial));
	}

	HRESULT WINAPI SetLight(DWORD Index, CONST D3DLIGHT9* pLight)
	{
		WRAPCALL(orig->SetLight(Index, pLight));
	}

	HRESULT WINAPI GetLight(DWORD Index, D3DLIGHT9* pLight)
	{
		WRAPCALL(orig->GetLight(Index, pLight));
	}

	HRESULT WINAPI LightEnable(DWORD Index, BOOL Enable)
	{
		WRAPCALL(orig->LightEnable(Index, Enable));
	}

	HRESULT WINAPI GetLightEnable(DWORD Index, BOOL* pEnable)
	{
		WRAPCALL(orig->GetLightEnable(Index, pEnable));
	}

	HRESULT WINAPI SetClipPlane(DWORD Index, CONST float* pPlane)
	{
		WRAPCALL(orig->SetClipPlane(Index, pPlane));
	}

	HRESULT WINAPI GetClipPlane(DWORD Index, float* pPlane)
	{
		WRAPCALL(orig->GetClipPlane(Index, pPlane));
	}

	HRESULT WINAPI SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
	{
		WRAPCALL(orig->SetRenderState(State, Value));
	}

	HRESULT WINAPI GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue)
	{
		WRAPCALL(orig->GetRenderState(State, pValue));
	}

	HRESULT WINAPI CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB)
	{
		WRAPCALL(orig->CreateStateBlock(Type, ppSB));
	}

	HRESULT WINAPI BeginStateBlock(void)
	{
		WRAPCALL(orig->BeginStateBlock());
	}

	HRESULT WINAPI EndStateBlock(IDirect3DStateBlock9** ppSB)
	{
		WRAPCALL(orig->EndStateBlock(ppSB));
	}

	HRESULT WINAPI SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus)
	{
		WRAPCALL(orig->SetClipStatus(pClipStatus));
	}

	HRESULT WINAPI GetClipStatus(D3DCLIPSTATUS9* pClipStatus)
	{
		WRAPCALL(orig->GetClipStatus(pClipStatus));
	}

	HRESULT WINAPI GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture)
	{
		WRAPCALL(orig->GetTexture(Stage, ppTexture));
	}

	HRESULT WINAPI SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture)
	{
		WRAPCALL(orig->SetTexture(Stage, pTexture));
	}

	HRESULT WINAPI GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue)
	{
		WRAPCALL(orig->GetTextureStageState(Stage, Type, pValue));
	}

	HRESULT WINAPI SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
	{
		WRAPCALL(orig->SetTextureStageState(Stage, Type, Value));
	}

	HRESULT WINAPI GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue)
	{
		WRAPCALL(orig->GetSamplerState(Sampler, Type, pValue));
	}

	HRESULT WINAPI SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
	{
		WRAPCALL(orig->SetSamplerState(Sampler, Type, Value));
	}

	HRESULT WINAPI ValidateDevice(DWORD* pNumPasses)
	{
		WRAPCALL(orig->ValidateDevice(pNumPasses));
	}

	HRESULT WINAPI SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries)
	{
		WRAPCALL(orig->SetPaletteEntries(PaletteNumber, pEntries));
	}

	HRESULT WINAPI GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries)
	{
		WRAPCALL(orig->GetPaletteEntries(PaletteNumber, pEntries));
	}

	HRESULT WINAPI SetCurrentTexturePalette(UINT PaletteNumber)
	{
		WRAPCALL(orig->SetCurrentTexturePalette(PaletteNumber));
	}

	HRESULT WINAPI GetCurrentTexturePalette(UINT *PaletteNumber)
	{
		WRAPCALL(orig->GetCurrentTexturePalette(PaletteNumber));
	}

	HRESULT WINAPI SetScissorRect(CONST RECT* pRect)
	{
		WRAPCALL(orig->SetScissorRect(pRect));
	}

	HRESULT WINAPI GetScissorRect(RECT* pRect)
	{
		WRAPCALL(orig->GetScissorRect(pRect));
	}

	HRESULT WINAPI SetSoftwareVertexProcessing(BOOL bSoftware)
	{
		WRAPCALL(orig->SetSoftwareVertexProcessing(bSoftware));
	}

	BOOL WINAPI GetSoftwareVertexProcessing(void)
	{
		WRAPCALL(orig->GetSoftwareVertexProcessing());
	}

	HRESULT WINAPI SetNPatchMode(float nSegments)
	{
		WRAPCALL(orig->SetNPatchMode(nSegments));
	}

	float WINAPI GetNPatchMode(void)
	{
		WRAPCALL(orig->GetNPatchMode());
	}

	bool stopdraw;
	HRESULT WINAPI DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
	{
		if (stopdraw)
			return D3D_OK;
		WRAPCALL(orig->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount));
	}

	HRESULT WINAPI DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
	{
		if (stopdraw)
			return D3D_OK;

		WRAPCALL(orig->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride));
	}

	HRESULT WINAPI DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
	{
		if (stopdraw)
			return D3D_OK;

		WRAPCALL(orig->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride));
	}

	HRESULT WINAPI ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags)
	{
		WRAPCALL(orig->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags));
	}

	HRESULT WINAPI CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
	{
		WRAPCALL(orig->CreateVertexDeclaration(pVertexElements, ppDecl));
	}

	HRESULT WINAPI SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
	{
		WRAPCALL(orig->SetVertexDeclaration(pDecl));
	}

	HRESULT WINAPI GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
	{
		WRAPCALL(orig->GetVertexDeclaration(ppDecl));
	}

	DWORD old_FVF;
	HRESULT WINAPI SetFVF(DWORD FVF)
	{
		if (g_hideUI && (FVF == 0x1c4)) {
			stopdraw = true;
		}
		else {
			stopdraw = false;
		}
		if (FVF != old_FVF) {
			old_FVF = FVF;
		}
		WRAPCALL(orig->SetFVF(FVF));
	}

	HRESULT WINAPI GetFVF(DWORD* pFVF)
	{
		WRAPCALL(orig->GetFVF(pFVF));
	}

	HRESULT WINAPI CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader)
	{
		WRAPCALL(orig->CreateVertexShader(pFunction, ppShader));
	}


	HRESULT WINAPI GetVertexShader(IDirect3DVertexShader9** ppShader)
	{
		WRAPCALL(orig->GetVertexShader(ppShader));
	}


	HRESULT WINAPI GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
	{
		WRAPCALL(orig->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount));
	}

	HRESULT WINAPI SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
	{
		WRAPCALL(orig->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount));
	}

	HRESULT WINAPI GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount)
	{
		WRAPCALL(orig->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount));
	}

	HRESULT WINAPI SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
	{
		WRAPCALL(orig->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount));
	}

	HRESULT WINAPI GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
	{
		WRAPCALL(orig->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount));
	}

	HRESULT WINAPI SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
	{
		WRAPCALL(orig->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride));
	}

	HRESULT WINAPI GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* OffsetInBytes, UINT* pStride)
	{
		WRAPCALL(orig->GetStreamSource(StreamNumber, ppStreamData, OffsetInBytes, pStride));
	}

	HRESULT WINAPI SetStreamSourceFreq(UINT StreamNumber, UINT Divider)
	{
		WRAPCALL(orig->SetStreamSourceFreq(StreamNumber, Divider));
	}

	HRESULT WINAPI GetStreamSourceFreq(UINT StreamNumber, UINT* Divider)
	{
		WRAPCALL(orig->GetStreamSourceFreq(StreamNumber, Divider));
	}

	HRESULT WINAPI SetIndices(IDirect3DIndexBuffer9* pIndexData)
	{
		WRAPCALL(orig->SetIndices(pIndexData));
	}

	HRESULT WINAPI GetIndices(IDirect3DIndexBuffer9** ppIndexData)
	{
		WRAPCALL(orig->GetIndices(ppIndexData));
	}

	HRESULT WINAPI CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader)
	{
		WRAPCALL(orig->CreatePixelShader(pFunction, ppShader));
	}

	HRESULT WINAPI GetPixelShader(IDirect3DPixelShader9** ppShader)
	{
		WRAPCALL(orig->GetPixelShader(ppShader));
	}


	HRESULT WINAPI GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
	{
		//LOGSPAM << "pixelshader! " << StartRegister << " " << Vector4fCount << "\r\n";
		WRAPCALL(orig->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount));
	}

	HRESULT WINAPI SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
	{
		WRAPCALL(orig->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount));
	}

	HRESULT WINAPI GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount)
	{
		WRAPCALL(orig->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount));
	}

	HRESULT WINAPI SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
	{
		WRAPCALL(orig->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount));
	}

	HRESULT WINAPI GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
	{
		WRAPCALL(orig->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount));
	}

	HRESULT WINAPI DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo)
	{
		WRAPCALL(orig->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo));
	}

	HRESULT WINAPI DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo)
	{
		WRAPCALL(orig->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo));
	}

	HRESULT WINAPI DeletePatch(UINT Handle)
	{
		WRAPCALL(orig->DeletePatch(Handle));
	}

	HRESULT WINAPI CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery)
	{
		WRAPCALL(orig->CreateQuery(Type, ppQuery));
	}

};

class AAUIDirect3D9 : public IDirect3D9 {
public:;
	IDirect3D9 *orig;

	AAUIDirect3D9(IDirect3D9 *old) {
		orig = old;
		orig->AddRef();
	}

/*	virtual ~AAUIDirect3D9(void) {
	}*/

	HRESULT WINAPI QueryInterface(REFIID riid, void ** ppvObj) {
		*ppvObj = this;
		return D3D_OK;
	}
	ULONG WINAPI AddRef() {
		return orig->AddRef();
	}
 	ULONG WINAPI Release() {
		ULONG count = orig->Release();
		if (!count)
			delete this;
		return count;
	}
	HRESULT WINAPI RegisterSoftwareDevice(void *p) {
		DWRAPCALL(orig->RegisterSoftwareDevice(p));
	}
	UINT WINAPI GetAdapterCount() {
		DWRAPCALL(orig->GetAdapterCount());
	}
	UINT WINAPI GetAdapterModeCount(UINT Adapter, D3DFORMAT Format)
	{
		DWRAPCALL(orig->GetAdapterModeCount(Adapter, Format));
	}

	HRESULT WINAPI GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier) {
		DWRAPCALL(orig->GetAdapterIdentifier(Adapter, Flags, pIdentifier));
	}
	HRESULT WINAPI EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode) {
		DWRAPCALL(orig->EnumAdapterModes(Adapter, Format, Mode, pMode));
	}
	HRESULT WINAPI GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE* pMode) {
		DWRAPCALL(orig->GetAdapterDisplayMode(Adapter, pMode));
	}
	HRESULT WINAPI CheckDeviceType(UINT iAdapter, D3DDEVTYPE DevType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed) {
		DWRAPCALL(orig->CheckDeviceType(iAdapter, DevType, DisplayFormat, BackBufferFormat, bWindowed));
	}
	HRESULT WINAPI CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) {
		DWRAPCALL(orig->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat));
	}
	HRESULT WINAPI CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels) {
		DWRAPCALL(orig->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels));
	}
	HRESULT WINAPI CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) {
		DWRAPCALL(orig->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat));
	}
	HRESULT WINAPI CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat) {
		DWRAPCALL(orig->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat));
	}
	HRESULT WINAPI GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps) {
		DWRAPCALL(orig->GetDeviceCaps(Adapter, DeviceType, pCaps));
	}
	HMONITOR WINAPI GetAdapterMonitor(UINT Adapter)
	{
		DWRAPCALL(orig->GetAdapterMonitor(Adapter));
	}

	HRESULT WINAPI CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) {
		static bool created;
//		LOGSPAM << "D3d behavior :" << std::hex << BehaviorFlags << "\n";
/*		BehaviorFlags &= ~D3DCREATE_DISABLE_PSGP_THREADING;
		BehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
		BehaviorFlags &= ~D3DCREATE_SOFTWARE_VERTEXPROCESSING;*/
		BehaviorFlags &= ~D3DCREATE_MULTITHREADED;
		HRESULT hres = orig->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
		auto pret = *ppReturnedDeviceInterface;
		d3dev = new AAUIDirect3DDevice9(pret);
		if (hres == D3D_OK) {
			Subtitles::gameWindowWidth = pPresentationParameters->BackBufferWidth; // Game window Width for Subtitles
			Subtitles::CorrectSubsAreaSize();
			*ppReturnedDeviceInterface = d3dev;
			if (!created && General::IsAAPlay && g_Config.getb("bFullscreen")) {
				DEVMODE dmScreenSettings = { 0 };
				dmScreenSettings.dmSize = sizeof(dmScreenSettings);
				dmScreenSettings.dmPelsWidth = pPresentationParameters->BackBufferWidth;
				dmScreenSettings.dmPelsHeight = pPresentationParameters->BackBufferHeight;
				dmScreenSettings.dmBitsPerPel = 32;
				dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
				ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
			}
			if (!created)
				LUA_EVENT_NORET("post_d3d", (DWORD)(hFocusWindow));
			created = true;
		}
		return hres;
	}
};

void *Wrap(void *orig) {
	return new AAUIDirect3D9((IDirect3D9*)orig);
}
}