require "iuplua"
require "iupluacontrols"

local _M = { 
	visible = false,
	floating = false
}

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

local function slider()
	return iup.hbox {
		iup.label { title = "X" , size = "12x10" },
		iup.text {},
		iup.flatbutton { title = "0", font = "Courier", size = "12x10", border = "yes" },
		iup.flatbutton { title = "-", font = "Courier", size = "12x10", border = "yes" },
		iup.flatbutton { title = "+", font = "Courier", size = "12x10", border = "yes" },
		iup.val { orientation = "horizontal", expand = "horizontal", min = -1, max = 1, value = 0 },
		alignment = "acenter",
		gap = 3
	}
end

local function shapecontrols(shapelist, rowsize)
	local controls = {}
	for _, s in ipairs(shapelist) do
		table.insert(controls, iup.flatbutton { title = s, toggle ="yes" })
	end
	table.insert(controls, iup.text { spin = "yes", spinvalue = 0, spinmin = 0, spinmax = #shapelist - 1, visiblecolumns = 1 })
	
	local rows = {}
	local a, b = 1, math.min(rowsize, #controls)
	while b <= #controls do
		table.insert(rows, iup.hbox { unpack(controls, a, b) })
		a = a + rowsize
		if b >= #controls then
			break
		end
		b = math.min(b + rowsize, #controls)
	end
	
	local norm = iup.normalizer { unpack(controls) }
	norm.normalize = "horizontal"
	return iup.radio { iup.vbox { unpack(rows) } }
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
				iup.toggle { title = "Show" },
				alignment = "acenter",
				gap = 3
			},
			slider(),
			slider(),
			slider(),
			iup.hbox {
				shapecontrols({ ":|", ":)", ":(", ":3", ":3" , ":O", ":s", "", ":[]", ":o", ":·", ":D", ":]", "", ":]", ":>"}, 4),
				shapecontrols({ "u_u", "n_n", "^_^", "-_-", "o_u", "u_o", "o_n", "n_o" }, 2)
			}
		},
		--gap = 3,
		--k_any = hotkey_cb
	},
	title = "Sliders"
}

dialogposes = iup.dialog {
	iup.vbox {
		listfilter(),
		iup.button { title = "Load", expand = "horizontal" },
		iup.button { title = "Save", expand = "horizontal" },
		iup.button { title = "Delete", expand = "horizontal" },
		iup.hbox { 
			iup.label { title = "Clip" },
			iup.text {},
			iup.label { title = "Frame" },
			iup.text {},
			gap = 3,
			alignment = "acenter"
		}
	},
	title = "Poses",
	--k_any = hotkey_cb
}

function _M.togglevisible()
	dialogs = dialogs or { dialogsliders, dialogposes }
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
	if _M.opts.floating == 0 then parent = _M.parentHWND end
	for _,v in ipairs(d) do
		SetParent(v.hwnd, parent)
	end
end

function _M.hotkey_cb(dialog, k, ...)
	print(dialog, "-", k, "-", ...)
	if k == iup.K_F12 then
		_M.togglevisible()
	end
end

signals.connect(dialogsliders, "k_any", _M, "hotkey_cb")
signals.connect(dialogposes, "k_any", _M, "hotkey_cb")

return _M
