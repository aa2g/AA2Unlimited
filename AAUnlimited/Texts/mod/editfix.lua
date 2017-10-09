--@INFO UI dialog fixes of edit (slider ranges)


-- RMB key to save as rainbow
local RAINBOW_KEY = 0x10 -- shift key
local SELECT_PNG = 0x11 -- ctrl key
local GEN_PNG = 0x12 -- alt key

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
			if fn:lower() == eye_select:lower() then
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

	-- stops png buffer from being freed
	g_poke(0x12626D, "\xeb")
	g_poke(0x126285, "\x90\x90\x90\x90\x90\x90")

end

function on.update_edit_gui()
	local base = GetPropW(g_peek_dword(0x353180), GameBase + 0x3100A4)
	local function run(addr,off, ...)
		local val = peek_dword(base+off)
		proc_invoke(GameBase + addr, nil, val, ...)
	end

	run(0x1D5B0,128) -- slow

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



function on.pre_save_card(char,outbufp,outlenp)
	log("presave orig ptr%x %d", peek_dword(fixptr(outbufp)), peek_dword(fixptr(outlenp)))

	local bufp = peek_dword(fixptr(outbufp))
	local bufsz = peek_dword(fixptr(outlenp))

	-- just before the saving card, add rainbow
	if is_key_pressed(RAINBOW_KEY) then
		info("Rainbow applied")
		char.m_charData:m_traitBools(38, 1)
	end

	-- generator requested, or no no image and no png select requested
	if is_key_pressed(GEN_PNG) then
		if bufp ~= 0 then
			log("genpng: freeing previous png")
			free(bufp)
		end
		bufp = 0
		bufsz = 0
	end

	-- replace png buffer with custom file. aaud will be filled in later by caller.
	if is_key_pressed(SELECT_PNG) then
		local png = iup.filedlg {filter="*.png", title="Select PNG as card face"}
		png:popup(iup.ANYWHERE, iup.ANYWHERE)
		if png then
			local fpng = io.open(png.value, "rb")
			fpng:read(8)
			while true do
					local len = fpng:read(4)
					len = string.unpack(">I", len)
					local tag = fpng:read(4)
					fpng:seek("cur",len+4)
					if tag == "IEND" then break end
			end
			local fsize = fpng:seek("cur")
			fpng:seek("set", 0)
			local pngbuf = fpng:read(fsize)
			fpng:close()
			if bufp ~= 0 then
				free(bufp)
			end
			bufp = malloc(#pngbuf)
			bufsz = #pngbuf
			poke(bufp, pngbuf)
		end
	end
	poke_dword(fixptr(outbufp), bufp)
	poke_dword(fixptr(outlenp), bufsz)
end

function _M.unload()
	if exe_type ~= "edit" then return end
	g_poke_dword(0x002C43E0, send_msg)
	g_poke_dword(0x2C4350, dlg_param)
end

return _M