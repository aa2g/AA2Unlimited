--@INFO Triggers supplemental code

require 'text'

local _M = {}
local opts = {}

function _M:load()
	mod_load_config(self, opts)
end

function _M:unload()
end

--function _M:config()
--	mod_edit_config(self, opts, "Extended save options")
--end

return _M