-- snowflaked card saving

-- TODO: configurable key
local CARD_OPTIONS = 0x11 -- ctrlkey

local opts
local saved_pose, facebmp, rosterbmp

-- open png file and return its buffer
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


function on.post_save_card()
	if saved_pose then
		GetPlayerCharacter().m_xxSkeleton.m_animArraySize = saved_pose
	end
	saved_pose = nil

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

	if not is_key_pressed(CARD_OPTIONS) then
		if opts.face == 1 then
			free(faceptr)
			cd.m_pngBuffer = 0
		end

		if opts.roster == 1 then
			free(rosterpng)
			cd.m_pngRosterBuffer = 0
		end

		return
	end
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
1, "200x300", gdef, gdef, opts.face, opts.roster)

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


return function(_M, _opts, p)
	opts = _opts
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

