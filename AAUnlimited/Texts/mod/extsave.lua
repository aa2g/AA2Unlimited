--@INFO Saves game everywhere, multi save

local _M = {}
require 'memory'

local codes = { 114, 115, 116, 117, 118, 119 }
local bcodes = { 0x02, 0x04, 0x10, 0x11, 0x12 }


local keyf = ""
local opts = {
	{"qskey", 1, "Quick save key: %l|None|F3|F4|F5|F6|F7|F8|"},
	{ "bkey", 3, "Backup on save key: %l|None|Right Mouse Button|Middle Mouse Button|Shift Key|Control Key|Alt Key|" },
	{ "signal", 1, "Signal save done: %b" },

}


local patch = parse_asm [[
00000000  893578563412      mov [dword 0x12345678],esi
00000006  83EC10            sub esp,byte +0x10
00000009  EB03              jmp short 0xe
0000000B  EBF3              jmp short 0x0
0000000D  90                nop
]]

--rawset(_ENV, "g_var", true)

local g_var


local function save_handler(data)
	local cfile = unicode_to_utf8(fixptr(data) + 100):match("^(.*)%.sav$")
	local function mkp(p)
		return play_path("data", "save", "class", p .. ".sav")
	end
	local fn = 	mkp(cfile)
	log.info("Saving game to %s.sav...", fn)
	if not is_key_pressed(bcodes[opts.bkey]) then return end
	local fd = io.open(fn, "rb")
	if fd then
		local bak = fd:read("*a")
		fd:close()
		local idx = 0
		while true do
			idx = idx+1
			fn = mkp(cfile .. idx)
			fd = io.open(fn, "rb")
			if not fd then
				log.info("...previous save exists, backed up into %s." % fn)
				fd = io.open(fn, "wb")
				fd:write(bak)
				fd:close()
				return true
			else
				fd:close()
			end
		end
	end
end

function on.keyup(key)
	if codes[opts.qskey] == key then
		local data = ptr_walk(g_var,0x18,0x28,0)
		save_handler(data)
		proc_invoke(GameBase+0xF36D0, 0, data)
		if opts.signal == 1 then
			SetCursorPos(0,0)
		end
	end
end

function on.save_class(data)
	save_handler(data)
end

function _M:load()
	mod_load_config(self, opts)
	if exe_type ~= "play" then return end
	g_var = malloc(4)
	local addr = 0x470B0
	g_poke(addr - #patch + 3, patch)
	g_poke_dword(addr - #patch + 3 + 2, g_var)
end


function _M:unload()
end

function _M:config()
	mod_edit_config(self, opts, "Extended save options")
end

--proc_invoke(GameBase+0xF36D0, 0, peek_dword(peek_dword(peek_dword(g_var)+0x18)+0x28))
--proc_invoke(GameBase+0xF36D0, 0, peek_dword(peek_dword(peek_dword(g_var)+0x18)+0x28))
--ptr_walk(g_var,0x18,0x28,0)
return _M