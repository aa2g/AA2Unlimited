--@INFO Removes the need for locale emulator

require "memory"

local opts = {
	{"level", 1, "Force jp system langid: %l|Never|Auto|Always|" }
}

local function patch_aaplay()
	g_poke(0x001BEBE3, "\x80");
	g_poke(0x001C208C, "\x80");
	g_poke(0x001C39A1, "\x80");
	g_poke(0x001AEA80, "\x80");
	g_poke(0x0021BD45, "\x90\x90");

	local p = x_pages(8192)
	local fixcp = "\xC7\x44\x24\x04\xA4\x03\x00\x00"
	poke(p, fixcp .. "\x68" .. string.pack("<I4", g_xchg_dword(0x002E318C, p)) .. "\xC3")
	poke(p+128, fixcp .. "\x68" .. string.pack("<I4", g_xchg_dword(0x002E3190, p+128)) .. "\xC3")
end

function on.launch()
	if not ((opts.level > (((GetSystemDefaultLangID() & 0x3ff) == 17) and 1 or 0))) then return end
	if (_BINDING.IsAAPlay) then
		patch_aaplay()
	end
	SetThreadLocale(1041)
end


local _M={}

function _M:load()
	mod_load_config(self, opts)
end

function _M:unload()
end

function _M:config()
	mod_edit_config(self, opts, "Fixlocale settings")
end

return _M