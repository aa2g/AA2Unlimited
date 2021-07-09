--@INFO Overrides no bra physics status

local _M = {}

local defallstates = "22 26 28 36 42 43 57 58 93 151 152 157 158 159 166 167 172 180 181 222 223 224 225 228 235 252"
local defstate1 = ""
local defstate3 = "37 38 39 155 156 188 198 230 238 240 242"

local opts = {
	{ "usedefaults", 1, "Use AAUnlimited default settings: %b"},
	{ "allstates", defallstates, "Force no-bra in all states of: %s"},
	{ "state1", defstate1, "Force no-bra in state 1 of: %s" },
	{ "state3", defstate3, "Force no-bra in state 3 of: %s" },
}


local function reload_overrides()
	local slots = opts.usedefaults == 1 and defallstates or opts.allstates
	for slot in string.gmatch(slots, "%d+") do
		SetNoBraOverride(tonumber(slot), 0xF)
	end
	slots = opts.usedefaults == 1 and defstate1 or opts.state1
	for slot in string.gmatch(slots, "%d+") do
		SetNoBraOverride(tonumber(slot), 0x1)
	end
	slots = opts.usedefaults == 1 and defstate3 or opts.state3
	for slot in string.gmatch(slots, "%d+") do
		SetNoBraOverride(tonumber(slot), 0x4)
	end
end


function _M:load()
	mod_load_config(self, opts)
	reload_overrides()
end

function _M:unload()
end

function _M:config()
	mod_edit_config(self, opts, "No-bra options")
	reload_overrides()
end

return _M

