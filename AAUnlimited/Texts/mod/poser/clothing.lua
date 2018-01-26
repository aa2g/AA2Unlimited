local _M = {}

local proxy = require "poser.proxy"
local signals = require "poser.signals"

function _M.slotbuttons(label, list, lambda)
	local controls = {}
	if label then
		table.insert(controls, iup.label { title = label })
		table.insert(controls, iup.fill {})
	end
	for i, s in ipairs(list) do
		local button = iup.flatbutton { title = s, border = "yes", padding = 3, font = "Serif, Courier, 8", size = "15x10" }
		function button.flat_action(self)
			lambda(i - 1)
		end
		table.insert(controls, button)
	end
	
	return iup.hbox {
		expand = true,
		gap = 3,
		alignment="acenter",
		table.unpack(controls)
	}
end

return _M
