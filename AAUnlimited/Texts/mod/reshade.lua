--@INFO Sideloads ReShade.me / SweetFX
require "memory"
require "strutil"

local _M = {}

function on.d3d9_preload(wfh)
	if wfh == 0 then
		LoadLibraryA("d3d9.dll")
		return LoadLibraryA("reshade/dxgi.dll")
	end

	-- We have a prior DLL.
	-- Reshade is phenomenal in it's stupidity to *always* force load from windows dir,
	-- so we casually lie about it and just say the prior dll is the system one
	local hfn = _BINDING.GetProcAddress("KERNEL32","GetModuleHandleExW")
	local _, saved = hook_func(hfn, 6, 3, function(orig, this, flags, name, out)
		local nam = unicode_to_utf8(peek(name, 256, "\x00\x00", 2))
		if (nam:match(".*d3d9%.dll$")) then
			poke_dword(out, wfh)
			return 1
		end
		local han = GetModuleHandleA(name)
		poke_dword(out, han)
		return han ~= 0
	end)
	local d3dh = LoadLibraryA("reshade/dxgi.dll")
	poke(hfn, saved)
	return d3dh
end

function _M:load()
end

function _M:unload() end

return _M