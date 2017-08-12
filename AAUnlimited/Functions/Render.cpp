// Very primtitive compositing 2D graphics engine
// NOT FUNCTIONAL, WIP

#include "StdAfx.h"
#include <windows.h>
#include "RenderWrap.h"

namespace Render {

struct D3DVertex
{
	float X, Y, Z, RHW;
//	unsigned int color;
	float U, V;
};

#define BEGIN_TARGET(tex) \
	IDirect3DSurface9 *saved_target; \
	IDirect3DSurface9 *target; \
	d3dev->GetRenderTarget(0, &saved_target); \
	tex->GetSurfaceLevel(0, &target); \
	d3dev->SetRenderTarget(0, target);

#define END_TARGET() \
	d3dev->SetRenderTarget(0, saved_target)

struct TexCoords {
	float w,h,x,y;
};

std::map<IDirect3DTexture9 *, TexCoords> textures;

// We're last to draw
void onEndScene() {
	d3dev->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
	d3dev->SetPixelShader(nullptr);
	d3dev->SetVertexShader(nullptr);

//	d3dev->SetRenderState(D3DRS_LIGHTING, true);
//	d3dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
//	d3dev->SetRenderState(D3DRS_ZENABLE, false);
//	d3dev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
/*	d3dev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	d3dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	d3dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);*/



	for (auto &t : textures) {
		auto *tex = t.first;
		auto &coords = t.second;
		float X1 = coords.x;
		if (X1 == INFINITY)
			continue;
		float Y1 = coords.y;
		float X2 = coords.x + coords.w;
		float Y2 = coords.y + coords.h;
		float U = 0.5f / (float)(X2 - X1);
		float V = 0.5f / (float)(Y2 - Y1);
		D3DVertex vertices[] =
		{
			/*
			1->-2
			  /
			3 -> 4
				
			{X1, Y1, 1.0f, 1.0f, D3DCOLOR_RGBA(0xFF, 0xFF, 0xFF, 0xFF), 0.0f + U, 0.0f + V},
			{X2, Y1, 1.0f, 1.0f, D3DCOLOR_RGBA(0xFF, 0xFF, 0xFF, 0xFF), 1.0f + U, 0.0f + V},
			{X1, Y2, 1.0f, 1.0f, D3DCOLOR_RGBA(0xFF, 0xFF, 0xFF, 0xFF), 0.0f + U, 1.0f + V},
			{X2, Y2, 1.0f, 1.0f, D3DCOLOR_RGBA(0xFF, 0xFF, 0xFF, 0xFF), 1.0f + U, 1.0f + V}*/

			/*
			 1  2
			 | /|
			 3/ 4
			
			*/

#define Z 32.0
#define RHW 1.0

			{ X1, Y1, Z, RHW, /*D3DCOLOR_RGBA(0xFF, 0xFF, 0xFF, 0xFF),*/ 0.0f + U, 0.0f + V },
			{ X1, Y2, Z, RHW, /*D3DCOLOR_RGBA(0xFF, 0xFF, 0xFF, 0xFF),*/ 0.0f + U, 1.0f + V },
			{ X2, Y1, Z, RHW, /*D3DCOLOR_RGBA(0xFF, 0xFF, 0xFF, 0xFF),*/ 1.0f + U, 0.0f + V },
			{ X2, Y2, Z, RHW, /*D3DCOLOR_RGBA(0xFF, 0xFF, 0xFF, 0xFF),*/ 1.0f + U, 1.0f + V }

		};
		d3dev->SetTexture(0, tex);
		d3dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(D3DVertex));
	}
}


//d3dev->CreateTexture(s.get(1), s.get(2), 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, 0, &tex, NULL) == D3D_OK

void bindLua() {
	auto b = g_Lua[LUA_BINDING_TABLE];

	b["CreateTexture"] = LUA_LAMBDA({
		IDirect3DTexture9 *tex;
		int w = s.get(1);
		int h = s.get(2);
		if (!d3dev) return 0;

		if (d3dev->CreateTexture(s.get(1), s.get(2), 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &tex, NULL) == D3D_OK) {
			auto &coords = textures[tex];
			coords.w = s.get(1);
			coords.h = s.get(2);
			coords.x = INFINITY;
			s.push(tex);
			return 1;
		}
	});
#define LUA_CLASS IDirect3DTexture9
	LUA_METHOD(Release, {
		IDirect3DTexture9 *tex = _gl.get(1);
		tex->Release();
		textures.erase(tex);
	});

	LUA_METHOD(Clear, {
		IDirect3DTexture9 *tex = _gl.get(1);
		BEGIN_TARGET(tex);
		d3dev->Clear(0, NULL, D3DCLEAR_TARGET, int(_gl.get(2)), 1.0f, 0);
		END_TARGET();
	});
	LUA_METHOD(Draw, {
		IDirect3DTexture9 *tex = _gl.get(1);
		if (_gl.isnil(2)) {
			textures[tex].x = INFINITY;
		}
		else {
			auto &t = textures[tex];
			t.x = _gl.get(2);
			t.y = _gl.get(3);
		}
	});
#undef LUA_CLASS
}

}