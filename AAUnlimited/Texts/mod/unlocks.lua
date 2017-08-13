--@INFO Unlocks hue and sliders in editor.

local offsets = {
	-- unlocks hue
	["\x67\x01"] = {
		0x0002058A,
		0x0011A5A0,
	},

	-- cmp cl, 100 -> nop
	-- this is for byte value checks on card load
	["\x3A\xC9\x90"] = {
		0x0001CBE9,
		0x0001CBF5,
		0x0001CC14,
		0x0001CC20,
		0x0001CC3F,
		0x0001CC4B,
		0x0001E647,
		0x0001E653,
		0x0001E672,
		0x0001E67E,
		0x0001E69D,
		0x0001E6A9,
		0x0001E6C8,
		0x0001E6D4,
		0x0001E6F3,
		0x0001E6FF,
		0x0001E71E,
		0x0001E72A,
		0x0001E749,
		0x0001E755,
		0x00022DF7,
		0x00022E03,
		0x00022E22,
		0x00022E2E,
		0x00022E4D,
		0x00022E59,
		0x00022E78,
		0x00022E84,
		0x00022EA3,
		0x00022EAF,
		0x00022ECE,
		0x00022EDA,
		0x00022EF9,
		0x00022F05,
		0x00022F24,
		0x00022F30,
	},

	-- cmp eax, 100 -> nop
	-- most likely slider values clamp
	["\x3B\xC0\x90"] = {
		0x00016A4A,
		0x0001C801,
		0x0001C80A,
		0x0001C824,
		0x0001C82D,
		0x0001C847,
		0x0001C850,
		0x0001E15D,
		0x0001E166,
		0x0001E180,
		0x0001E189,
		0x0001E1A3,
		0x0001E1AC,
		0x0001E1C6,
		0x0001E1CF,
		0x0001E1E9,
		0x0001E1F2,
		0x0001E20C,
		0x0001E215,
		0x0001E22F,
		0x0002264D,
		0x00022656,
		0x00022670,
		0x00022679,
		0x00022693,
		0x0002269C,
		0x000226B6,
		0x000226BF,
		0x000226D9,
		0x000226E2,
		0x000226FC,
		0x00022705,
		0x0002271F,
		0x00022728,
		0x00022742,
		0x0002274B,
	},
	["\x3A\xD2\x90"] = {
		-- NOT YET IMPLEMENTED
	}
}

local _M = {}
local mcfg

local save = {}

function _M:load()
	if exe_type == "play" then
		-- play has only the hue patch
		f_patch("\x67\x01", 0x12C0B0)
	elseif exe_type == "edit" then
		for bytes, offs in pairs(offsets) do
			for _, off in ipairs(offs) do
				save[off] = f_patch(bytes, off)
			end
		end
	end
end

function _M:unload()
	for offs,bytes in ipairs(save) do
		f_patch(bytes,offs)
	end
	save = {}

	do_unpatch()
end

return _M
