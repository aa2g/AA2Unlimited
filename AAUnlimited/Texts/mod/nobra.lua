--@INFO Overrides no bra physics status

local _M = {}
local opts = {
	{ "allstates", "22 26 28 36 42 43 57 58 93 151 152 159 172 180 181 222 223 224 225 235", "Force no-bra in all states of: %s"},
	{ "state1", "", "Force no-bra in state 1 of: %s" },
	{ "state3", "155 156 188 230", "Force no-bra in state 3 of: %s" },
}


local function reload_overrides()
	for slot in string.gmatch(opts.allstates, "%d+") do
		SetNoBraOverride(tonumber(slot), 0xF)
	end
	for slot in string.gmatch(opts.state1, "%d+") do
		SetNoBraOverride(tonumber(slot), 0x1)
	end
	for slot in string.gmatch(opts.state3, "%d+") do
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

