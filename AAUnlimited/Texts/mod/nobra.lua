--@INFO Overrides no bra physics status

local _M = {}
--local opts = {
--	{ "maxlines", 4, "Maximum number of lines to show %i[1,10]"},
--	{ "duration", 3, "Subtitles duration %i[1,10]:" },
--}


local function reload_overrides()
	for slot in string.gmatch("26 180 181 222 223 224 225 172", "%d+") do
		log.info("override nobra slot %s", slot)
		SetNoBraOverride(tonumber(slot), 1)
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

