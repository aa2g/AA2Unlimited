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
