--@INFO Editor extensions (sliders, snowflaker etc)


-- Todo: configurable
local RAINBOW_KEY = 0x10 -- shift key
local SELECT_PNG = 0x11 -- ctrl key
local GEN_PNG = 0x12 -- alt key

local CARD_OPTIONS = 0x11 -- ctrlkey


local _M = {}

local LB_SETCURSEL = 0x186
local LB_ADDSTRING = 0x180
local TBM_SETRANGEMAX = 0x408
local EM_SETLIMITTEXT = 0xc5


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

local function open_png(path, buf, sz)
	local fpng = io.open(path, "rb")
	if not fpng then return buf, sz end
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
	if buf ~= 0 then
		free(buf)
	end
	buf = malloc(#pngbuf)
	sz = #pngbuf
	poke(buf, pngbuf)
	return buf, sz
end

local saved_pose, facebmp, rosterbmp

local function illusion_string(s)
	local buf, slen = cp_strdup(65001, s, 16)
	local meta = buf-16
	poke(meta, string.pack("<IIII", GameBase + 0x36f638, slen, 2048, 999))
	return buf
end

function on.pre_save_card(char,outbufp,outlenp)
	local cd = char.m_charData
	local keep = "(keep previous or generate)"
	local gdef = "(game default)"
	local facepng
	local rosterpng
	local faceptr = fixptr(cd.m_pngBuffer)
	local facesz = cd.m_pngBufferSize
	local rosterptr = fixptr(cd.m_pngRosterBuffer)
	local rostersz = cd.m_pngRosterBufferSize
	if not is_key_pressed(CARD_OPTIONS) then return end
	local chname = sjis_to_utf8(string.format("%s %s", cd.m_forename, cd.m_surname))
	local ok, rainbow, club, face, roster,
		currpose, vres, facebg, rosterbg, genface, genroster = iup.GetParam("Card saving options for '%s'" % chname, nil, [[
Custom card data %t
Rainbow %b
Club %i[0,7]
Face %f[OPEN|*.png||YES|NO]
Roster %f[OPEN|*.png||YES|NO]
Render card data %t
Use current pose %b
Resolution %s
Face background %f[OPEN|*.bmp|YES|NO]
Roster background %f[OPEN|*.bmp|YES|NO]
Render face %b]
Render roster %b]
]], cd:m_traitBools(38), cd.m_club, keep, keep,
1, "200x300", gdef, gdef, 0, 0)

	if not ok then return end
	log("tweaking card save path")

	cd:m_traitBools(38, rainbow)
	cd.m_club = club
	faceptr, facesz = open_png(face, faceptr, facesz)
	rosterptr, rostersz = open_png(roster, rosterptr, rostersz)
	if genface == 1 then
		log("resetting face")
		free(faceptr)
		faceptr = 0
		facesz = 0
	end
	if genroster == 1 then
		log("resetting roster")
		free(rosterptr)
		rosterptr = 0
		rostersz = 0
	end

	if facebg ~= gdef then
		facebmp = illusion_string(facebg)
	end
	if rosterbg ~= gdef then
		rosterbmp = illusion_string(rosterbg)
	end

	cd.m_pngBuffer, cd.m_pngBufferSize = faceptr, facesz
	cd.m_pngRosterBuffer, cd.m_pngRosterBufferSize = rosterptr, rostersz
	if currpose == 1 then
		saved_pose = GetPlayerCharacter().m_xxSkeleton.m_animArraySize
		GetPlayerCharacter().m_xxSkeleton.m_animArraySize = 0
	end

	local resx, resy = vres:match("([0-9]+)x([0-9]+)")
	resx, resy = tonumber(resx), tonumber(resy)
	log("using card resolution %dx%d" % {resx,resy})
	g_poke_dword(0x124881, resy)
	g_poke_dword(0x124886, resx)
end

function on.post_save_card()
	if saved_pose then
		GetPlayerCharacter().m_xxSkeleton.m_animArraySize = saved_pose
	end
	saved_pose = nil

end


function _M:load()
	_M.patcher = patcher()
	local p = _M.patcher
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
	p:g_hook_vptr(0x002C43E0, 4, function(orig, this, hdlg, msg, wparam, lparam)
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
	p:g_hook_vptr(0x2C4350, 5, function(orig,this,hinst,template,parent,dlgfun,initpar)
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

	-- stops cardface buffer from being freed
	p:g_poke(0x12626D, "\xeb")
	p:g_poke(0x126285, "\x90\x90\x90\x90\x90\x90")

	-- stop roster buffer from being freed. this patch changes the logic
	-- that if roster image exists, the generator isn't called, otherwise it will be called
	p:g_poke(0x1262B0, "\x75\x1e")

	p:g_hook_call(0x124E5B, -6, function(orig, this, parch, pthis, pfile, x, y, scale)
		log(unicode_to_utf8(peek(peek_dword(pfile), 256, "\x00\x00", 2)))
		if facebmp then
			local s1, s2 = peek_dword(parch), peek_dword(pfile)
			poke_dword(parch, 0)
			poke_dword(pfile, facebmp)
			log(unicode_to_utf8(peek(peek_dword(pfile), 256, "\x00\x00", 2)))
			local ret = proc_invoke(orig, this, parch, pthis, pfile, x, y, scale)
			free(facebmp - 16)
			facebmp = nil
			poke_dword(parch, s1)
			poke_dword(pfile, s2)
			return ret
		end
		return proc_invoke(orig, this, parch, pthis, pfile, x, y, scale)
	end)

	p:g_hook_call(0x125368, -6, function(orig, this, parch, pthis, pfile, x, y, scale)
		log(unicode_to_utf8(peek(peek_dword(pfile), 256, "\x00\x00", 2)))
		if rosterbmp then
			local s1, s2 = peek_dword(parch), peek_dword(pfile)
			poke_dword(parch, 0)
			poke_dword(pfile, rosterbmp)
			local ret = proc_invoke(orig, this, parch, pthis, pfile, x, y, scale)
			free(rosterbmp - 16)
			rosterbmp = nil
			poke_dword(parch, s1)
			poke_dword(pfile, s2)
			return ret
		end
		return proc_invoke(orig, this, parch, pthis, pfile, x, y, scale)
	end)

end

function _M:unload()
	_M.patcher:unload()
end


return _M