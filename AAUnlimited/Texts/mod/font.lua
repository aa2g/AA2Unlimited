require 'memory'
require 'strutil'

local _M = {}

local new_d3dfont
local new_gdifont

function _M.load()
	if exe_type ~= "play" then return end
	local font = Config.sFont or "Arial"
	new_d3dfont = strdup(utf8_to_unicode(font .. "\0"))
	new_gdifont = strdup(utf8_to_unicode(font .. "\0"))

--	new_d3dfont = strdup(font .. "\0")
--	new_gdifont = strdup(font .. "\0")
--[[
	for addr in ([.[
	00471A16 00471A8D 0049E300 004A6412 004CF15F 004CF247 004DD0E3 004DD1E1
	004DF2F6 004DF90B 004DF9EC 004E58CF 005641A1 005AE9F3 005BEB87 005C2039
	005AE9F3
	].]):gmatch("%S+") do
		local addr = tonumber(addr, 16) + 1 - 0x00400000
		log("d3dfont: patching %x", addr)
		g_poke_dword(addr, new_d3dfont)
	end
]]
--[[	for addr in ("005BEBB3 005C2065 005C3975"):gmatch("%S+") do
		local addr = tonumber(addr, 16) + 1 - 0x00400000
		log("gdifont: patching %x", addr)
		g_poke_dword(addr, new_gdifont)
	end]]

	log("using font "..Config.sFont)
--[[
	log("peek font %x", g_peek_dword(0x2C44D4))
	g_hook_vptr(0x2C44D4, 12, function(orig, this, dev, height, width, weight, mips, italic, charset, outprec, q, pif, pface, ppfont)
		local name = unicode_to_utf8(peek(pface, 32, "\0\0", 2))
		log("d3d font: %s", name)
		return proc_invoke(orig, this, dev, height, width, weight, mips, italic, charset, outprec, q, pif, face, ppfont)
	end)
	log("peek font 2 %x", g_peek_dword(0x2E3058))
	g_hook_vptr(0x2E3058, 1, function(orig, this, lpf)
		local name = unicode_to_utf8(peek(lpf + 28, 32, "\0\0", 2))
		log("gdi font: %s", name)
		return proc_invoke(orig, this, lpf)
	end)]]
end

return _M
