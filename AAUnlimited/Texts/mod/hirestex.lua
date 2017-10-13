--@INFO hi-res texture patch
require 'memory'
local _M = {}

local function do_patch(p,eye1,eye2,clothing,skirt,
	hair,body,unk1,unk2)
	p:g_poke(eye1,"\xDB\x46\x0C\x90")
	p:g_poke(eye2, "\xDB\x46\x10\x90")
	local function set_dimensions(ret, target, typ, delta)
		delta = delta or 8
		--print("inside hook for ",typ)
		if ret == 0 then return end
		local tex = peek_dword(target)
		if tex == 0 then return end
		local newx, newy = peek_dword(tex + 12), peek_dword(tex + 16)


		local origx = xchg_dword(target + delta, newx)
		local origy = xchg_dword(target + delta + 4, newy)

--		local origx = peek_dword(target + delta)
--		local origy = peek_dword(target + delta + 4)

		if newx ~= origx then
			log("overriding %s texture size from %d to %d" % {typ,origx,newx})
		end
	end

	p:g_hook_func(clothing, 7, 9 + (exe_type=="play" and 1 or 0), function(orig, this, a2,a3,a4, ...)
		local target = ((a4 + 6 * a3) << 7) + a2 + 60
		local ret = proc_invoke(orig,this,a2,a3,a4,...)
		set_dimensions(ret, target, "clothing")
		return ret
	end)
	p:g_hook_func(skirt, 7, 8, function(orig, this, a2,a3,a4,...)
		local target = ((a4 + 6 * a3) << 7) + a2 + 60
		local ret = proc_invoke(orig,this,a2,a3,a4,...)
		set_dimensions(ret, target, "skirt/sitagi")
		return ret
	end)

	p:g_hook_func(hair, 7, 5, function(orig, this, a2, a3, a4,...)
		local target = 32 * (a3 + 1) + a2 + 12
		local ret = proc_invoke(orig, this,  a2, a3, a4, ...)
		set_dimensions(ret, target, "hair")
		return ret
	end)

	-- was unk3
	p:g_hook_func(body, 7, 9, function(orig, this, a2, a3, a4,...)
		local ret = proc_invoke(orig, this, a2, a3, a4,...)
		local target = a2 + 56 * a3 + 412
		set_dimensions(ret, target, "body", 16)
		return ret
	end)

	-- possibly lowpoly or tans
	p:g_hook_func(unk1, 7, 5, function(orig, this, a2, a3, a4,...)
		local ret = proc_invoke(orig, this, a2, a3, a4,...)
		log("hirestex FIXME: unk1")
		return ret
	end)
	p:g_hook_func(unk2, 7, 5, function(orig, this, a2, a3, a4,...)
		local ret = proc_invoke(orig, this, a2, a3, a4,...)
		log("hirestex FIXME: unk2")
		return ret
	end)
end

function _M:load()
	local p = patcher()
	_M.patcher = p
	if exe_type == "edit" then
		do_patch(p,
			0x118C88,0x118CC8, -- eyes
			0x12EBA0,0x12F0E0, -- cloth, skirt
			0x135BC0,0x13E5F0, -- hair, body
			0x136E10,0x13DE70)
	else
		do_patch(p,
			0x12A778,0x12A7B8, -- eyes
			0x140A50,0x140F90, -- cloth, skirt
			0x147D70,0x1507F0, -- hair, body
			0x148FE0,0x150070)
	end
end

function _M.unload()
	_M.patcher:unload()
end

return _M