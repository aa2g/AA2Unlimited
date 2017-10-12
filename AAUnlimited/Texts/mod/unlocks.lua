--@INFO Limit removals (hue, ASU..)

-- This script unites virtually all unlocks of known "snowflake" launchers,
-- that is, frontier and ASU (both play and exe), plus some additional features
-- in edit (preserve rainbow, sliders).

-- For ease of use, offsets can be an in-memory offset (when negative), or disk-file
-- offset (when positive).  We translate those to in-memory offsets when applying
-- in either case.

-- When adding a patch, please make sure it doesn't collide with some snowflaked
-- launcher (ie generating corrupt opcodes). Either put here the exact same thing
-- the snowflake has, or move it to other code path.
--
-- If the patch affects gameplay, add a bool config button for it, too. The script
-- ensures that the game can be live patched/unpatched via script settings.
--
-- The patch tables are only for tiny in-place opcode replacements, *not* for detours
-- to undecipherable assembler code blobs on the side.
--
-- There are other facilities better suited to that end - on.* events and g_hook_*.
-- These allow you to route the code path to Lua, do your thing in script, and call
-- the original function from Lua, too.
--
-- To try out patches on the fly, you can just type f_patch("\xAA\xBB...", 0xfileoff..)
-- in the console for patching the memory in live session. It returns the original
-- memory contents, too.

-----------------------------------------------------------------
-- Configurable IG patches
local patches = {}
local options = {
	-- name, default, dialog
	{"roleswap", 1, "Enable H position role swap: %b"},
	{"homosex", 0, "Enable homosex: %b"},
	{"homosex2", 0, "Enable homosex (test fixes): %b"},
}

-- "Are they both girls" check before spawning role swap button
patches.roleswap = {
	["\x39\xc0\x90"] = {
		-0x7D8DE,
	},
}

patches.homosex = {
	-- "Is a boy" check when inviting/forcing for sex
	["\xff"] = {
		-0x7A2C4,
		-0x7FBBF,
		-0x957BB,
	},
	["\x00"] = {
		-0x79280,
		-0x83191,
		-0x8BC40,
		-0x96412,
	},
	-- dynamic_cast<GirlClass> -> static_cast to prevent null deref
	["\x89\xc8\xeb\x01"] = {
		-0x8E122,
	},
}

-- more casting mishap fixes
local rswapfix = parse_asm[[
00000001  05E0000000        add eax,0xe0
00000006  8B4804            mov ecx,[eax+0x4]
00000009  8B09              mov ecx,[ecx]
0000000B  80794001          cmp byte [ecx+0x40],0x1
0000000F  7425              jz 0x36
00000011  8D4804            lea ecx,[eax+0x4]
00000014  8B31              mov esi,[ecx]
00000016  807E4000          cmp byte [esi+0x40],0x0
0000001A  741A              jz 0x36
0000001C  39C8              cmp eax,ecx
0000001E  7408              jz 0x28
00000020  FF30              push dword [eax]
00000022  FF31              push dword [ecx]
00000024  8F00              pop dword [eax]
00000026  8F01              pop dword [ecx]
]]

patches.homosex2 = {
	[rswapfix] = {
		-0x8DF25,
	}
}

-----------------------------------------------------------------

patches.play = {
	-- hue onload check
	["\x67\x01"] = {
		0x12C0B0
	},
}


patches.edit = {

	-- dont hardcode eye resolution (128x128), copy it from bitmap instead
	["\xDB\x46\x0C\x90"] = {
		-0x118C88
	},
	["\xDB\x46\x10\x90"] = {
		-0x118CC8,
	},

	-- nop out hue check
	["\x67\x01"] = {
		0x0002058A,
		0x0011A5A0,
	},

	-- this is for byte value checks for sliders on card load
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
	-- body slider ui value clamp
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

	-- Patch in jumps over the body of functions in charge
	-- of disabling buttons of:

	-- H pref (when over 2)
	["\xe9\x90\x00\x00\x00"] = {
		0x0002F216,
	},

	-- Traits (when over 2)
	["\xe9\x88\x00\x00\x00"] = {
		0x0002f0d3
	},


	-- Remove check for trait and H-pref count on initial load.
	["\xeb"] = {
		0x0002EC47,
		0x0002EDA1,
	},
	
	-- More slider clamps
	["\x3a\xc0"] = {
		0x0002044B,
		0x00020456,
		0x0002049D,
		0x000204A8,
	},
	["\x3A\xD2\x90"] = {
		0x000278CB,
		0x000278D7,
		0x00027916,
		0x00027922,
		0x0002799C,
		0x000279A8,
		0x00027A06,
		0x00027A12,
	},

	-- force all hues sliders and their text field limits to 0..359, not 0..40
	["\xBB\x67\x01\x00\x00"] = {
		0xD5BB9
	},

	-- Don't remove rainbow, if present
	["\xeb\x05"] = {
		0x00123422,
	},
}

local _M = {}

local save = {}
local psave = {}

local function unapply_patchset()
	for offs,bytes in pairs(psave) do
		f_patch(bytes,offs)
	end
	psave = {}
end

local function apply_patchset()
	if exe_type ~= "play" then return end
	for name, set in pairs(patches) do
		if options[name] == 1 then
			for bytes, offs in pairs(set) do
				for _, off in ipairs(offs) do
					psave[off] = f_patch(bytes, off)
				end
			end
		end
	end
end

local send_msg
function _M:load()
	mod_load_config(self, options)

	-- some mandatory patches
	for bytes, offs in pairs(patches[exe_type]) do
		for _, off in ipairs(offs) do
			save[off] = f_patch(bytes, off)
		end
	end

	apply_patchset()
end


function _M:config()
	mod_edit_config(self, options, "Configure unlocks")
	unapply_patchset()
	apply_patchset()
end

function _M:unload()
	for offs,bytes in ipairs(save) do
		f_patch(bytes,offs)
	end
	save = {}
	unapply_patchset()
end

return _M
