require "iuplua"
require "iupluacontrols"

local _M = { 
	visible = false,
	floating = false
}

-- IUP fonts:
-- The string is parsed and the font typeface, style and size are set
-- according to the parsed values, as if cdCanvasFont was called.
-- The native font string is cleared when a font is set using cdCanvasFont.

-- The common format definition is similar to the the Pango library Font
-- Description, used by GTK+2. It is defined as having 3 parts:
-- <font family>, <font styles> <font size>. For ex: "Times, Bold 18",
-- or "Arial,Helvetica, Italic Underline -24". The supported styles include:
-- Bold, Italic, Underline and Strikeout. Underline, Strikeout, and negative
-- pixel values are not supported by the standard Pango Font Description.
-- The Pango format include many other definitions not supported by the
-- CD format, they are just ignored.

local dialogsliders
local dialogposes
local dialogs
--local style = require "poser.style"
local signals = require "poser.signals"

local unpack = table.unpack

local ctrl = setmetatable({}, {__index=function(t,k)
	return iup.GetDialogChild(dlg, k)
end})


local sshift = 16
local function floatspinner(v,step)
	return iup.text {
		value=tostring(v),
		spin="yes",
		spinauto="no",
		spin_cb = function(e,pos)
			e.value = v + tonumber(pos - (1<<sshift)) * step
			e:valuechanged_cb()
		end,
		mask=iup.MASK_FLOAT,
		spinmin=0,
		spinmax=1<<(sshift+1),
		spinvalue=1<<sshift
	}
end

local function listfilter()
	return iup.vbox {
		iup.text { expand = "horizontal"},
		iup.list {
			expand = "yes"
		},
		expand = "yes"
	}
end

local function slider(label)
	local step = 0.01
	
	local incremented = signals.signal()
	local slidestarted = signals.signal()
	local slidestopped = signals.signal()
	local textbox = iup.text {}
	local sliding = false
	
	local control = iup.hbox {
		iup.label { title = label },
		textbox,
		iup.flatbutton { title = "0", font = "Serif, Courier, 8", size = "15x10", border = "yes", flat_action = function() incremented(0) end },
		iup.flatbutton { title = "-", font = "Serif, Courier, 8", size = "15x10", border = "yes", flat_action = function() incremented(-0.01) end },
		iup.flatbutton { title = "+", font = "Serif, Courier, 8", size = "15x10", border = "yes", flat_action = function() incremented(0.01) end },
		iup.val { orientation = "horizontal", expand = "horizontal", min = -1, max = 1, step = step, value = 0,
			mousemove_cb = function(self)
				if not sliding then
					sliding = true
					slidestarted()
				end
				incremented(self.value)
			end,
			button_press_cb = function(self)
				print("button press")
				incremented(self.value)
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
		incremented = incremented,
		slidestarted = slidestarted,
		slidestopped = slidestopped,
	}
	
	incremented.connect(textbox, "value")
	
	return control
end

local function shapecontrols(title, shapelist, rowsize)
	local controls = {}
	for _, s in ipairs(shapelist) do
		table.insert(controls, iup.flatbutton { title = s, toggle ="yes", padding = 3 })
	end
	local open = iup.label { title = "Open" }
	local spin = iup.text { spin = "yes", spinvalue = 0, spinmin = 0, spinmax = 9, visiblecolumns = 1 }
	
	local norm = iup.normalizer { unpack(controls) }
	norm.normalize = "horizontal"
	return iup.frame { title = title, 
		iup.vbox {
			iup.radio {
				iup.gridbox { numdiv = rowsize, unpack(controls) },
				expand = "yes"
			},
			iup.hbox {
				open,
				spin,
				alignment = "acenter",
				gap = 3,
			},
			alignment = "aright",
			gap = 3,
		}
	}
end

local function facecontrols()
	local leyebrows = iup.label { title = "Eyebrow", alignment = "aright:acenter" }
	local lblush1 = iup.label { title = "Blush", alignment = "aright:acenter" }
	local lblush2 = iup.label { title = "Blush (lines)", alignment = "aright:acenter" }
	local ldimeyes = iup.label { title = "Dim Eyes", alignment = "aright:acenter" }
	local ltears = iup.label { title = "Tears", alignment = "aright:acenter" }
	local leyetracking = iup.label { title = "Eye Tracking", alignment = "aright:acenter" }
	local lyogurt = iup.label { title = "Yogurt", alignment = "aright:acenter" }
	local eyebrows = iup.text { spin = "yes", spinvalue = 0, spinmin = 0, spinmax = 6, visiblecolumns = 1 }
	local blush1 = iup.text { spin = "yes", spinvalue = 0, spinmin = 0, spinmax = 12, visiblecolumns = 1 }
	local blush2 = iup.text { spin = "yes", spinvalue = 0, spinmin = 0, spinmax = 12, visiblecolumns = 1 }
	local dimeyes = iup.toggle { }
	local tears = iup.toggle { }
	local eyetracking = iup.toggle { }
	local yogurt = iup.toggle { }
	local lnorm = iup.normalizer { leyebrows, lblush1, lblush2, ldimeyes, ltears, leyetracking }
	lnorm.normalize = "horizontal"
	return iup.frame {
		title = "Face (more)",
		iup.gridbox { numdiv = 4,
			ldimeyes, dimeyes, leyebrows, eyebrows,
			ltears, tears, lblush1, blush1,
			leyetracking, eyetracking, lblush2, blush2,
			lyogurt, yogurt,
			gapcol = 3, gaplin = 3
		},
		expand = "yes"
	}
end

dialogsliders = iup.dialog {
	iup.hbox {
		listfilter(),
		listfilter(),
		iup.vbox {
			iup.hbox {
				iup.radio {
					iup.hbox {
						iup.flatbutton { title = "Rotate", toggle = "yes", border = "yes", padding = 3},
						iup.flatbutton { title = "Translate", toggle = "yes", border = "yes", padding = 3 },
						iup.flatbutton { title = "Scale", toggle = "yes", border = "yes", padding = 3 },
					}
				},
				iup.label { title = "Modifiers" },
				iup.radio {
					iup.hbox {
						iup.flatbutton { title = "x1", toggle = "yes", border = "yes", padding = 3 },
						iup.flatbutton { title = "x10", toggle = "yes", border = "yes", padding = 3 },
						iup.flatbutton { title = "x100", toggle = "yes", border = "yes", padding = 3 },
					}
				},
				iup.flatbutton { title = "Show", toggle = "yes", border = "yes", padding = 3 },
				alignment = "acenter",
				gap = 3
			},
			slider("X"),
			slider("Y"),
			slider("Z"),
			iup.hbox {
				shapecontrols("Mouth", { ":|", ":)", ":(", ":3", ":3" , ":O", ":s", "", ":[]", ":o", ":·", ":D", ":]", "", ":]", ":>"}, 4),
				shapecontrols("Eyes", { "u_u", "n_n", "^_^", "-_-", "o_u", "u_o", "o_n", "n_o" }, 2),
				facecontrols(),
				expand = "yes",
			}
		},
		--gap = 3,
	},
	maxbox = "no",
	minbox = "no",
}

dialogposes = iup.dialog {
	iup.tabs {
		iup.vbox {
			listfilter(),
			iup.button { title = "Load Pose", expand = "horizontal" },
			iup.button { title = "Save Pose", expand = "horizontal" },
			iup.button { title = "Delete Pose", expand = "horizontal" },
			iup.hbox { 
				iup.label { title = "Clip" },
				iup.text {},
				iup.label { title = "Frame" },
				iup.text {},
				gap = 3,
				alignment = "acenter"
			},
			tabtitle = "Poses"
		},
		iup.vbox {
			listfilter(),
			iup.button { title = "Load Scene", expand = "horizontal" },
			iup.button { title = "Save Scene", expand = "horizontal" },
			iup.button { title = "Delete Scene", expand = "horizontal" },
			iup.hbox { 
				iup.label { title = "Clip" },
				iup.text {},
				iup.label { title = "Frame" },
				iup.text {},
				gap = 3,
				alignment = "acenter"
			},
			tabtitle = "Scenes"
		},
	},
	maxbox = "no",
	minbox = "no",
}

dialogcharacters = iup.dialog {
	iup.vbox {
		iup.label { title = "Character" },
		iup.list { expand = "horizontal" },
		iup.label { title = "Style" },
		iup.list { expand = "horizontal" },
	},
	maxbox = "no",
	minbox = "no",
}

function _M.togglevisible()
	dialogs = dialogs or { dialogcharacters, dialogposes, dialogsliders }
	if not _M.visible then
		for _,v in ipairs(dialogs) do
			v:map()
			_M.updatefloating({v})
			v:show()
		end
		_M.visible = true
	else
		for _,v in ipairs(dialogs) do
			v:hide()
		end
		_M.visible = false
	end
end

function _M.updatefloating(d)
	d = d or dialogs
	if not d then return end
	local parent = 0
	if _M.opts.floating == 0 or _M.opts.forcefullscreen then parent = _M.parentHWND end
	for _,v in ipairs(d) do
		SetParent(v.hwnd, parent)
	end
end

function _M.hotkey_cb(dialog, k, ...)
	if k == iup.K_F12 then
		_M.togglevisible()
	end
end

signals.connect(dialogsliders, "k_any", _M, "hotkey_cb")
signals.connect(dialogposes, "k_any", _M, "hotkey_cb")

return _M
