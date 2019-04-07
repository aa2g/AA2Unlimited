--@INFO Overrides no bra physics status

local _M = {}
--local opts = {
--	{ "maxlines", 4, "Maximum number of lines to show %i[1,10]"},
--	{ "duration", 3, "Subtitles duration %i[1,10]:" },
--}


local function reload_overrides()
	for slot in string.gmatch("22 26 28 36 42 43 57 58 93 151 152 159 172 180 181 222 223 224 225 235", "%d+") do
		SetNoBraOverride(tonumber(slot), 0xF)
	end
	for slot in string.gmatch("155 156 188 230", "%d+") do
		SetNoBraOverride(tonumber(slot), 0x4)
	end
end


function _M:load()
	-- mod_load_config(self, opts)
	reload_overrides()
end

function _M:unload()
end

--function _M:config()
--	mod_edit_config(self, opts, "Subtitles options")
--end

return _M

