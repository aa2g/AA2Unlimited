

local function do_patch(isplay,path,unpatch,dll)
	local rdsize = 0x6daa2
	-- all rva offsets are real file offset + this much
	local delta = 0x1800


	local dllpos = 0x32e178
	local namepos = 0x32e790
	local nameexpect = 0x330098


	-- in-place values, +deltad
	local dllexpect = 0x3300aa
--	local scratchpos = 0x330230

	if isplay then
		rdsize = 0x71482

		delta = 0x1400
		dllpos = 0x350ec0
		namepos = 0x3514f0
--		scratchpos = 0x353090

		
		dllexpect = 0x352a0a
		nameexpect = 0x3529f8
	end

	local pf, msg = io.open(path, "r+b")
	if not pf then iup.Message("Patch failed", msg)
		return
	end

	local hdr = 0x248

	local rsize, val1, val2, roff = string.unpack("<IIII", pf:readat(hdr, 16))

	if math.abs(rsize - rdsize) > 256 then
		iup.Message("Failure", path .. " doesn't appear to be AA2 play/edit exe file.\n"..
		string.format("rdata.size = 0x%x, but expected around 0x%x", rsize, rdsize))
		return
	end

	local ispatched = rsize ~= rdsize
	if unpatch and ispatched then
		pf:writeat(hdr, string.pack("<IIII", rdsize, val1, val2, roff))
		pf:put32(dllpos, dllexpect)
		pf:put32(namepos, nameexpect)
		iup.Message("Unpatch success", path .. " unpatched, should be whatever it was originally.")
		pf:close()
		return
	end

	if unpatch and not ispatched then
		iup.Message("Failed to unpatch", path .. " is not aau patched, keeping it as is.")
		pf:close()
		return
	end

	-- otherwise we be patchin'
	local dllpath = dll .. "\0"
	local symbol = (aaplay and "\x02" or "\x09") .. "\x00_AA2Unlimited@4\0"

	pf:writeat(hdr, string.pack("<IIII", rdsize + #dllpath + #symbol, val1, val2, roff))

	local scratchpos = rdsize + roff
	pf:writeat(scratchpos, dllpath .. symbol)

	pf:put32(dllpos, scratchpos + delta)
	pf:put32(namepos, scratchpos + delta + #dllpath)
	if isplay then
		-- permanently patch out the annoying features (codepage check, cd check)
		pf:writeat(0x21b145,"\x90\x90")
		pf:writeat(0x1236,"\xeb")
	end
	-- bigaddr enable
	pf:writeat(0x136, "\x22")
	pf:close()
	iup.Message("Patch success.", path .. " patched to load " .. dllpath)
end

return function(arg,showcmd)
	require "iuplua"
	require "iupluacontrols"
	local meta = getmetatable(io.stdin)
	function meta:readat(pos, n)
		assert(type(pos)=="number")
		self:seek('set',pos)
		return self:read(n)
	end
	function meta:writeat(pos, data)
		assert(type(pos)=="number")
		assert(data)
		self:seek('set',pos)
		return self:write(data)
	end
	function meta:get32(pos)
		return string.unpack("<I", self:readat(pos, 4))
	end
	function meta:put32(pos, word)
		self:writeat(pos, string.pack("<I", word))
	end

	local switch, exe, dll = arg:match("^/(.?) *([^ ]*) *(.*)")
	if switch == "P" then
		return do_patch(true, exe, false, dll)
	elseif switch == 'p' then
		return do_patch(true, exe, true, dll)
	elseif switch == 'E' then
		return do_patch(false, exe, false, dll)
	elseif switch == 'e' then
		return do_patch(false, exe, true, dll)
	elseif switch then
		iup.Message("Usage", [[
To patch:
rundll32 AAUnlimitedDLL.dll,_AA2UPatcher@16 /P play.exe path\to\aau.dll
rundll32 AAUnlimitedDLL.dll,_AA2UPatcher@16 /E edit.exe path\to\aau.dll

To unpatch:
rundll32 AAUnlimitedDLL.dll,_AA2UPatcher@16 /p play.exe
rundll32 AAUnlimitedDLL.dll,_AA2UPatcher@16 /e edit.exe
	]])
		return
	end

	local play = _BINDING.GetAAPlayPath()
	local edit = _BINDING.GetAAEditPath()


	local _, play, edit, dopatch = iup.GetParam("AA2U "..AAU_VERSION.." game patcher", nil, [[
Select files to patch %t
AA2 Play: %f[OPEN|*.exe|]]..play..[[|YES|NO]
AA2 Edit: %f[OPEN|*.exe|]]..edit..[[|YES|NO]
Mode: %b[Unpatch - AAU will be disabled (if present),Patch - AAU will be enabled (if not present),]
	]], play .. "aa2play.exe", edit .. "aa2edit.exe", 1)

	if not play then return end
	local dllpath = _BINDING.GetAAUPath() .. "AAUnlimitedDLL.dll" .. "\0"

	local ok, msg = pcall(function()
		do_patch(true, play, dopatch == 0,_BINDING.GetAAUPath() .. "AAUnlimitedDLL.dll")
		do_patch(false, edit, dopatch == 0,_BINDING.GetAAUPath() .. "AAUnlimitedDLL.dll")
	end)
	if not ok then
		iup.Message("Patcher failed with", msg)
	end

end