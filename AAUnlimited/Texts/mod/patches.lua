-- some simple patches to be made right before the game launch

local function patches_play()
	g_poke(0x32E48A, Config.bUseMKIII and "t\0g\0a\0" or "b\0m\0p\0")

	if Config.bUsePP2 or Config.bUsePPeX then
		g_poke(0x216510, "\xb8\x01\x00\x00\x00\xc3")
	end		

	if (Config.bPngSkirtSitagi) then
		g_poke(0x3280C8, "t\0x\0_\0%\0\x30\0\x33\0d\0.\0p\0n\0g\0\0\0\0")	-- スカート_%03d.bmp -> tx_%03d.png
		g_poke(0x3280E4, "t\0x\0_\0%\0\x30\0\x33\0d\0.\0p\0n\0g\0\0\0\0")	-- 下着_%03d.bmp -> tx_%03d.png
		g_poke(0x327FA8, "t\0x\0\0\0\0\0")	-- スカート -> tx
		g_poke(0x328000, "t\0x\0\0\0")	-- 下着 -> tx
		g_poke_dword(0xA913A, GameBase+0x32540A) -- use .png constant instead of .bmp
	end	
end

local function patches_edit()
	g_poke(0x30D552, Config.bUseMKIII and "t\0g\0a\0" or "b\0m\0p\0")

	if Config.bUsePP2 or Config.bUsePPeX then
		g_poke(0x1F8A90, "\xb8\x01\x00\x00\x00\xc3")
	end

	if (Config.bPngSkirtSitagi) then
		g_poke(0x3081D0, "t\0x\0_\0%\0\x30\0\x33\0d\0.\0p\0n\0g\0\0\0\0")	-- スカート_%03d.bmp -> tx_%03d.png
		g_poke(0x3081EC, "t\0x\0_\0%\0\x30\0\x33\0d\0.\0p\0n\0g\0\0\0\0")	-- 下着_%03d.bmp -> tx_%03d.png
		g_poke(0x3080B4, "t\0x\0\0\0\0\0")	-- スカート -> tx
		g_poke(0x30810C, "t\0x\0\0\0")	-- 下着 -> tx
		g_poke_dword(0x9F77A, GameBase+0x30568E) -- use .png constant instead of .bmp
	end
end

if exe_type == "play" then
	patches_play()
else
	patches_edit()
end
