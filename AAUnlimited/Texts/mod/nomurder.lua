--@INFO Disables murder actions

local _M = {}
local opts = {}

function on.move(struct,user)
	if (struct.conversationId == 81 or struct.conversationId == 82) then
		struct.conversationId = -1
		struct.target1 = 0
	end
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