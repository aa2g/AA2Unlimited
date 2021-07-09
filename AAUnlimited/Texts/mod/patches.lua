-- some simple patches to be made right before the game launch

local function patches_play()
	g_poke(0x32E48A, Config.bUseMKIII and "t\0g\0a\0" or "b\0m\0p\0")
	if Config.bUsePP2 or Config.bUsePPeX then
		g_poke(0x216510, "\xb8\x01\x00\x00\x00\xc3")
	end
end

local function patches_edit()
	g_poke(0x30D552, Config.bUseMKIII and "t\0g\0a\0" or "b\0m\0p\0")
	if Config.bUsePP2 or Config.bUsePPeX then
		g_poke(0x1F8A90, "\xb8\x01\x00\x00\x00\xc3")
	end
end

if exe_type == "play" then
	patches_play()
else
	patches_edit()
end
