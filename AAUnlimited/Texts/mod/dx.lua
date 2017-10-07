require "memory"
require 'com'

if false then
dx_dev = create_com_interface [[
QueryInterface AddRef Release TestCooperativeLevel GetAvailableTextureMem EvictManagedResources
GetDirect3D GetDeviceCaps GetDisplayMode GetCreationParameters SetCursorProperties SetCursorPosition
ShowCursor CreateAdditionalSwapChain GetSwapChain GetNumberOfSwapChains Reset Present GetBackBuffer
GetRasterStatus SetDialogBoxMode SetGammaRamp GetGammaRamp CreateTexture CreateVolumeTexture CreateCubeTexture
CreateVertexBuffer CreateIndexBuffer CreateRenderTarget CreateDepthStencilSurface UpdateSurface UpdateTexture
GetRenderTargetData GetFrontBufferData StretchRect ColorFill CreateOffscreenPlainSurface SetRenderTarget
GetRenderTarget SetDepthStencilSurface GetDepthStencilSurface BeginScene EndScene Clear SetTransform
GetTransform MultiplyTransform SetViewport GetViewport SetMaterial GetMaterial SetLight GetLight LightEnable
GetLightEnable SetClipPlane GetClipPlane SetRenderState GetRenderState CreateStateBlock BeginStateBlock
EndStateBlock SetClipStatus GetClipStatus GetTexture SetTexture GetTextureStageState SetTextureStageState
GetSamplerState SetSamplerState ValidateDevice SetPaletteEntries GetPaletteEntries SetCurrentTexturePalette
GetCurrentTexturePalette SetScissorRect GetScissorRect SetSoftwareVertexProcessing GetSoftwareVertexProcessing
SetNPatchMode GetNPatchMode DrawPrimitive DrawIndexedPrimitive DrawPrimitiveUP DrawIndexedPrimitiveUP
ProcessVertices CreateVertexDeclaration SetVertexDeclaration GetVertexDeclaration SetFVF GetFVF CreateVertexShader
SetVertexShader GetVertexShader SetVertexShaderConstantF GetVertexShaderConstantF SetVertexShaderConstantI
GetVertexShaderConstantI SetVertexShaderConstantB GetVertexShaderConstantB SetStreamSource GetStreamSource
SetStreamSourceFreq GetStreamSourceFreq SetIndices GetIndices CreatePixelShader SetPixelShader GetPixelShader
SetPixelShaderConstantF GetPixelShaderConstantF SetPixelShaderConstantI GetPixelShaderConstantI
SetPixelShaderConstantB GetPixelShaderConstantB DrawRectPatch DrawTriPatch DeletePatch CreateQuery]]

--ID3DXBuffer 
dx_buffer = create_com_interface("QueryInterface AddRef Release GetBufferPointer GetBufferSize")
function dx_buffer:gets()
	return peek(self:GetBufferPointer(), self:GetBufferSize())
end

function dx_compile_shader(path, main)
	local flags = 0
	local outptr = retptr()
	local outptr2 = retptr()

	if path:match(".*%.psh$") then
		profile = D3DXGetPixelShaderProfile(dev)
	else
		profile = D3DXGetVertexShaderProfile(dev)
	end

	if D3DXCompileShaderFromFileA(path, 0, 0, main, profile, flags, outptr.p, outptr2.p, 0) ~= 0 then
		local buf = dx_buffer(outptr2:val())
		local msg = buf:gets()
		buf:Release()
		error(msg)
	end
	if outptr2:val() ~= 0 then
		local buf = dx_buffer(outptr2:val())
		buf:Release()
	end
	return outptr:val()
end


end

function on.edit_gui_update()
	local base = GetPropW(g_peek_dword(0x353180), GameBase + 0x3100A4)
	function run(addr,off, ...)
		local val = peek_dword(base+off)
		print("%x"%addr,val)
		proc_invoke(GameBase + addr, nil, val, ...)
	end

	--run(0x1D5B0,128) -- slow

	run(0x1EFC0,136)
	run(0x20E10,144,0) -- updates most sliders
	
	--run(0x22360,152)
	
	run(0x23640,160) -- updates eyes
	run(0x24E20,168,0)
	run(0x25D50,176,0)
	run(0x26FC0,184)
	run(0x28350,192) -- semi-slow?
	run(0x28AA0,192)

	--run(0x2AD20,200)
	--run(0x2BC30,208)

	run(0x2D510,216)
	run(0x2DB00,216) -- pose?
	run(0x2F730,224) -- pose?
end
