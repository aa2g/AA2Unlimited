require "memory"

local function patch_aaplay()
	GPoke(0x001BEBE3, "\x80");
	GPoke(0x001C208C, "\x80");
	GPoke(0x001C39A1, "\x80");
	GPoke(0x001AEA80, "\x80");
	GPoke(0x0021BD45, "\x90\x90");

	p = XPages(8192)
	local fixcp = "\xC7\x44\x24\x04\xA4\x03\x00\x00"
	Poke(p, fixcp .. "\x68" .. string.pack("<I4", GXchgD(0x002E318C, p)) .. "\xC3")
	Poke(p+128, fixcp .. "\x68" .. string.pack("<I4", GXchgD(0x002E3190, p+128)) .. "\xC3")
end


return function(mod, level)
	level = level or 1
	-- not emulated, nor japanese system (langid 17)
	if (level > (((PInvoke(GetProcAddress("KERNEL32","GetSystemDefaultLangID"), "") & 0x3ff) == 17) and 1 or 0)) then
		if (IsAAPlay) then
			patch_aaplay()
		end
		PInvoke(GetProcAddress("KERNEL32","SetThreadLocale"), "\x11\x04\x00\x00")
	end
end