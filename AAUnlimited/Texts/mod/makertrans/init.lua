require "memory"
require "strutil"
local dict = require "makertrans.dict"

local WM_INITDIALOG = 0x0110
local WM_SETTEXT = 0x000C
local WM_GETTEXT = 0x000D
local TCM_INSERTITEMW = 0x1300 + 62

-- resource ids of elements which need icon cleared
local clearicon = {
	[10758] = true,
	[10759] = true,
	[10014] = true,
	[10015] = true
}


local _M = {}

local function translate()
	local mem = malloc(4096)
	local transtab = {}
	local count = 0
	for k,v in pairs(dict) do
		transtab[utf8_to_unicode(k)] = utf8_to_unicode(v) .. "\x00\x00"
		count = count+1
	end
	log.spam("maker translation has "..count.." strings")

	-- hook main dialog proc, and scan dialog items right before init
	g_hook_func(0x19AB90, 6, 4, function(orig, this, hdlg, msg, wparam, lparam)
		local ret = proc_invoke(orig, this, hdlg, msg, wparam, lparam)
		if msg ~= WM_INITDIALOG then
			return ret
		end
		local sub = 0
		while true do
			sub = FindWindowExA(hdlg,sub,0,0)
			if sub == 0 then break end
			local itemid = GetWindowLongW(sub, -12)
			if clearicon[itemid] then
				SetWindowLongW(sub, -16, GetWindowLongW(sub, -16) & 0xffffff7f)
			end
			local nlen = GetClassNameA(sub, mem, 256)
			if nlen < 0 then nlen = 0 end
			local cls = peek(mem, nlen)
			if cls == "Button" or cls == "Static" then
				local nlen = SendMessageW(sub, WM_GETTEXT, 2048, mem)
				if nlen < 0 then nlen = 0 end
				if nlen > 0 then
					local gots = peek(mem, nlen * 2)
					if transtab[gots] then
						SendMessageW(sub, WM_SETTEXT, 0, transtab[gots])
					end
				end
			end
		end
		return ret
	end)

	-- SendMessageW() hook, post dialog init items are added to the lists
	g_hook_vptr(0x002C43E0, 4, function(orig, this, hdlg, msg, wparam, lparam)
		if msg == TCM_INSERTITEMW then
			local ptr = peek_dword(lparam + 12)
			local str = peek(ptr, 256, "\0\0", 2)
			local tr = transtab[str]
			if tr then
				poke_dword(lparam + 12, mem)
				poke(mem, tr)
			end
		end
		local ret = proc_invoke(orig, this, hdlg, msg, wparam, lparam)
		return ret
	end)

	-- SetWindowTextW() as well
	g_hook_vptr(0x2C43B4, 2, function(orig, this, hwnd, text)
		local got = peek(text, 256, "\x00\x00", 2)
		if transtab[got] then
			got = transtab[got]
			text = got
		end
		local r = proc_invoke(orig, this, hwnd, text)
		return r
	end)
end

function _M:load()
	if not _BINDING.IsAAEdit then
		return
	end
	translate()
end

function _M:unload()
	-- TODO reloading
end

return _M