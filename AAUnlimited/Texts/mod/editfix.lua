--@INFO UI dialog fixes of edit (slider ranges)


-- RMB key to save as rainbow
local RAINBOW_KEY = 2

local _M = {}

local send_msg, dlg_param

local LB_SETCURSEL = 0x186
local LB_ADDSTRING = 0x180
local TBM_SETRANGEMAX = 0x408
local EM_SETLIMITTEXT = 0xc5

function _M.load()
	require "iuplua"
	if exe_type ~= "edit" then return end
	local avoid = {
		[10176] = true,
		[10292] = true,
		[10356] = true,
		[10440] = true,
		[10551] = true,
		[10355] = true,
	}
	local eye_select

	-- SendMessageW
	send_msg = g_hook_vptr(0x002C43E0, 4, function(orig, this, hdlg, msg, wparam, lparam)
		if msg == LB_ADDSTRING and eye_select then
			local idx = proc_invoke(orig, this, hdlg, msg, wparam, lparam)
			local fn = unicode_to_utf8(peek(lparam, 256, "\x00\x00", 2))
			if fn == eye_select then
				SendMessageW(hdlg, LB_SETCURSEL, idx, 0)
			end
			return idx
		end
		-- slider range - something asking for 100, make it 255
		local itemid = GetWindowLongW(hdlg, -12)
		if not avoid[itemid] then

			if msg == TBM_SETRANGEMAX and wparam == 1 and lparam == 100 then
				--log("OVERRIDE 100->255 %x %x %x %x %x %x", orig,this,hdlg,msg,wparam,lparam)
				lparam = 255 
			end

			-- text field size, 6 and 26 -> 255
			if msg == EM_SETLIMITTEXT and (wparam == 26) or (wparam == 6) then
				wparam = 255
			end
		end


		local ret = proc_invoke(orig, this, hdlg, msg, wparam, lparam)
		return ret
	end)

	--DialogBoxParamW
	dlg_param = g_hook_vptr(0x2C4350, 5, function(orig,this,hinst,template,parent,dlgfun,initpar)
		if template == 167 then
			local kind = peek_dword(initpar+100)&0xff
			local path
			local typ
			if kind == 1 then
				path = host_path("data","texture","eye")
				typ = "iris bmp file"
			elseif kind == 2 then
				path = host_path("data","texture","hilight")
				typ = "highlight bmp file"
			end
			if path then
				local dlg = iup.filedlg {
					dialogtype = "OPEN",
					filter = "*.bmp",
					filterinfo = typ,
					directory=path,
					showpreview="yes",
				}
				dlg:popup()
				if dlg.status ~= "-1" then
					eye_select = dlg.value:match("[^\\]*$")
					log("selected %s", eye_select)
					local res = proc_invoke(orig,this,hinst,template,parent,dlgfun,initpar)
					eye_select = nil
					return res
				end
			end
		end

		return proc_invoke(orig,this,hinst,template,parent,dlgfun,initpar)
	end)

end


-- just before the saving card, add rainbow
function on.save_card(char,stat,outbufp,outlenp,outlen)
	local pngdata = peek(peek_dword(fixptr(outbufp)), outlen)
	local of = io.open("test.png", "wb")
	of:write(pngdata)
	of:close()

	info(char,stat,outbufp,outlenp,outlen)
	if (GetAsyncKeyState(RAINBOW_KEY) & 0x8000) ~= 0 then
		info("Rainbow applied")
		char.m_charData:m_traitBools(38, 1)
	end
	return char
end

function _M.unload()
	if exe_type ~= "edit" then return end
	g_poke_dword(0x002C43E0, send_msg)
	g_poke_dword(0x2C4350, dlg_param)
end

return _M