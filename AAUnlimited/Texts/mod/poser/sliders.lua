local _M = {}

local signals = require "poser.signals"

function _M.slider(opts)
	local step = 0.01
	
	local data = opts.data
	local increment = signals.signal()
	local slidestarted = signals.signal()
	local slidestopped = signals.signal()
	local textbox = iup.text {}
	local sliding = false
	
	local control = iup.hbox {
		iup.label { title = opts.title },
		textbox,
		iup.flatbutton { title = "0", font = "Serif, Courier, 8", size = "15x10", border = "yes", flat_action = function() increment(0, data) end },
		iup.flatbutton { title = "-", font = "Serif, Courier, 8", size = "15x10", border = "yes", flat_action = function() increment(-0.01, data) end },
		iup.flatbutton { title = "+", font = "Serif, Courier, 8", size = "15x10", border = "yes", flat_action = function() increment(0.01, data) end },
		iup.val { orientation = "horizontal", expand = "horizontal", min = -1, max = 1, step = step, value = 0,
			mousemove_cb = function(self)
				if not sliding then
					sliding = true
					slidestarted()
				end
				increment(self.value, data)
			end,
			button_press_cb = function(self)
				print("button press")
				increment(self.value, data)
				self.value = 0
			end,
			button_release_cb = function(self)
				self.value = 0
				sliding = false
				slidestopped()
			end,
		},
		alignment = "acenter",
		gap = 3,
		increment = increment,
		slidestarted = slidestarted,
		slidestopped = slidestopped,
	}
	
	increment.connect(textbox, "value")
	
	return control
end

return _M
