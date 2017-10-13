--@INFO hi-res texture patch
require 'memory'
local _M = {}

function do_patch(p,eye1,eye2,clothing,skirt)
	p:g_poke(eye1,"\xDB\x46\x0C\x90")
	p:g_poke(eye2, "\xDB\x46\x10\x90")
	p:g_hook_func(clothing, 7, 9 + (exe_type=="play" and 1 or 0), function(orig, this, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)
		local target = ((a4 + 6 * a3) << 7) + a2 + 60
		local result = proc_invoke(orig,this,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)
		if result == 0 then return result end
		local tex1, tex2 = peek_dword(target), peek_dword(target+4)
		if tex1 == 0 then return result end
		local new1 = peek_dword(tex1 + 12)
		local new2 = peek_dword(tex1 + 16)
		local orig1 = xchg_dword(target + 8, new1)
		local orig2 = xchg_dword(target + 12, new2)
		if new1 ~= orig1 then
			log("overriding clothing texture size from %d to %d" % {orig1,new1})
		end
		return result
	end)
	p:g_hook_func(skirt, 7, 8, function(orig, this, a2,a3,a4,a5,a6,a7,a8,a9)
		local target = ((a4 + 6 * a3) << 7) + a2 + 60
		local ret = proc_invoke(orig,this,a2,a3,a4,a5,a6,a7,a8,a9)
		local tex1, tex2 = peek_dword(target), peek_dword(target+4)
		if ret == 0 then return ret end
		if tex1 == 0 then return ret end
		local new1 = peek_dword(tex1 + 12)
		local new2 = peek_dword(tex1 + 16)
		local orig1 = xchg_dword(target + 8, new1)
		local orig2 = xchg_dword(target + 12, new2)
		if new1 ~= orig1 then
			log("overriding skirt/sitagi texture size from %d to %d" % {orig1,new1})
		end
		return ret
	end)
end

function _M:load()
	local p = patcher()
	_M.patcher = p
	if exe_type == "edit" then
		do_patch(p,0x118C88,0x118CC8,0x12EBA0,0x12F0E0)
	else
		do_patch(p,0x12A778,0x12A7B8,0x140A50,0x140F90)
	end
end

function _M.unload()
	_M.patcher:unload()
end

return _M