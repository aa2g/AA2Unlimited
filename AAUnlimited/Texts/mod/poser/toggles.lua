local _M = {}

local signals = require "poser.signals"

function _M.button(opts)
	local selected = signals.signal()
	local changedstate= signals.signal()
	
	local data = opts.data or opts.title
	
	local button = iup.flatbutton {
		title = opts.title,
		toggle = "yes",
		border = "yes",
		padding = 3,
		selected = selected,
		changedstate = changedstate,
	}
	
	function button.valuechanged_cb(self)
		if self.value == "ON" then
			selected(data)
		end
		changedstate(data, self.value == "ON")
	end
	
	return button
end

return _M
