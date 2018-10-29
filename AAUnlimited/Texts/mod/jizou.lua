--@INFO Portable jizou roster

local _M = {}

-- base game state struct
local base_offset = 0x367f48

-- roster state struct
local roster_offset = base_offset + 0xE2CC

local keys = { 0x02, 0x04, 0x10, 0x11, 0x12 }
local keyf = "%l|None|Right Mouse Button|Middle Mouse Button|Shift Key|Control Key|Alt Key|"
local opts = {
	{"key2", 1, "Jizou screen: "..keyf},
	{"key1", 0, "Select pc screen: "..keyf},
	{"key0", 0, "Class edit screen: "..keyf},
}


function on.ui_event(evt)
	if evt == 13 then
--[[
 0 - edit chars
 1 - edit pc
 3 - normal ig roster
 2 - jizou roster
 ]]

		local typ = g_peek_dword(roster_offset)
		if is_key_pressed(keys[opts.key2]) then
			typ = 2
		end
		if is_key_pressed(keys[opts.key1]) then
			typ = 1
		end
		if is_key_pressed(keys[opts.key0]) then
			typ = 0
		end
		log.spam("setting roster type to %d %d %d %d", typ, opts.key0, opts.key1, opts.key2)
		g_poke_dword(roster_offset, typ)
	end
	return evt
end

function _M:load()
	mod_load_config(self, opts)
end

function _M:unload()
end

function _M:config()
	mod_edit_config(self, opts, "Jizou options")
end

return _M
