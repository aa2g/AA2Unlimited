-- this file is a mess and always will be. keep function _only_ to gui.

local nocloseafterlaunch = true

log("loading launcher")

require "iuplua"
require "iupluacontrols"

local dlg
local sshift = 16
local function floatspinner(v,step)
	return iup.text {
		value=tostring(v),
		spin="YES",
		spinauto="NO",
		spin_cb = function(e,pos)
			e.value = v + tonumber(pos - (1<<sshift)) * step
		end,
		valuechanged_cb = function(e,nval)
			e.spinvalue = 1<<sshift
			v = tonumber(e.value)
		end,
		mask=iup.MASK_FLOAT,
		spinmin=0,
		spinmax=1<<(sshift+1),
		spinvalue=1<<sshift
	}
end

local step = "x6"

local buts = {}
local launch = function()
	local b = iup.button{title="Launch game", expand="HORIZONTAL", size="x32", margin="8x8",
	action=function()
		for _,v in ipairs(buts) do v.active="no" end
		iup.ExitLoop()
		if nocloseafterlaunch then
			function dlg:close_cb()
				return iup.IGNORE
			end
		end
	end}
	table.insert(buts, b)
	return b
end

local console = iup.text {
	appendnewline="no",
	multiline="yes",
	wordwrap="yes",
	expand="yes", readonly="yes", canfocus="no",
	value="Console activated\r\n"
}


function console:map_cb()
	console.scrolltopos = #console.value
end

local repl = iup.text {
	expand="horizontal"
}

function console:getfocus_cb()
	iup.SetFocus(repl)
end

local historypos=1
local history = {}
local function evalrepl(v)
	local ch, err = load("return "..v)
	if not ch then
		ch, err = load(v)
	end
	if not ch then
		log.error("%s",err)
	end
	local ret = {xpcall(ch, debug.traceback)}
	if not ret[1] then
		log.error("%s", ret[2])
	else
		if #ret > 1 then
			print(table.unpack(ret,2))
		end
	end
end

function repl:k_any(c)
	if c == iup.K_UP then
		if historypos > 0 then
			historypos = historypos - 1
			repl.value = history[historypos] or ""
		end
		return iup.IGNORE
	end
	if c == iup.K_DOWN then
		if historypos <= #history then
			historypos = historypos + 1
			repl.value = history[historypos] or ""
		end
		return iup.IGNORE
	end
	if c == 13 then
		local v = repl.value
		if history[#history] ~= v then
			table.insert(history, v)
		end
		evalrepl(v)
		repl.value = ""
		historypos = #history + 1
		return iup.IGNORE
	end
	return iup.CONTINUE
end


local tabs =
	iup.tabs {
	
	iup.vbox {
		tabtitle = "Graphics",
		alignment="acenter",
		iup.fill{},
	iup.hbox {
		alignment="acenter",
		ngap="4",
		iup.frame {
			title = "Output",
			iup.gridbox {
				nmargin="x2",
				padding="x6",
				orientation="HORIZONTAL", numdiv=2, normalizesize="HORIZONTAL", homogenouslin="YES", alignmentlin = "ACENTER" ,
				iup.label {title = "Resolution:", }, iup.list { "640x480", "800x600", dropdown="YES",editbox="YES",mask="^[0-9]+x[0-9]+$" },
				iup.label {title = "Antialiasing:",}, iup.list { "2x", "4x", dropdown="YES" },
				iup.label {title = "Shadowmap:", }, iup.list { "2x", "4x", dropdown="YES" },
				iup.label {title = "Mipmap:", }, iup.list { "2x", "4x", dropdown="YES" },
			},
		},
		iup.frame {
			title = "Toggles",
			iup.vbox {
				iup.toggle {title = "Fullscreen" },
				iup.toggle {title = "Type 2 renderer (fast)" },
				iup.toggle {title = "Smooth textures" },
				iup.toggle {title = "Bilinear filtering" },
				iup.toggle {title = "Hardware vertex processing" },
				iup.toggle {title = "Blur" },
				iup.toggle {title = "Rim lighting" },
			}
		},
	}, iup.fill{}, launch()},

	iup.vbox { 
		alignment="acenter",
		tabtitle = "AA2Unlimted",
		iup.fill{},
	iup.hbox {
		alignment="acenter",
		ngap="4",
		iup.frame {
			title = "Settings",
			iup.gridbox {
				padding="x2",
				nmargin="x0",
				orientation="HORIZONTAL", numdiv=2, normalizesize="HORIZONTAL", homogenouslin="YES", alignmentlin = "ACENTER" ,
				iup.label {title = "Default log priority" }, iup.list { "spam","info","warn","error", "critical", dropdown="YES" },
				iup.label {title = "Legacy cards" }, iup.list { "ignore", "load", "reinterpret", "convert", dropdown="YES" },
				iup.label {title = "Card eye files", }, iup.list { "ask", "always extract", "dont extract", dropdown="YES" },
				iup.label {title = "Screenshot format", }, iup.list { ".BMP", ".JPG", dropdown="YES" },
				iup.label {title = "Poser hotkeys", padding=step }, iup.text {"WER", mask="^[A-Z]*$"},
				iup.label {title = "POV X",  }, floatspinner(0,0.1),
				iup.label {title = "POV Y", step }, iup.text {spin="YES"},
				iup.label {title = "POV Z", }, iup.text {spin="YES"},

			},
		},
		iup.frame {
			title = "Toggles",
			iup.vbox {
				iup.toggle {title = "Additional tan slots (DANGEROUS)" },
				iup.toggle {title = "Mesh texture overrides" },
				iup.toggle {title = "H AI" },
				iup.toggle {title = "H AI on noprompt (force by evil)" },
				iup.toggle {title = "Game file shadowing" },
				iup.toggle {title = "Reorder H buttons" },
				iup.toggle {title = "Clothes poser" },
				iup.toggle {title = "Dialogue poser" },
				iup.toggle {title = "Backup card on remove" },
				iup.toggle {title = "Remove savefile from cards" },
			}
		},
	}, iup.fill{}, launch() },

	iup.hbox {
		tabtitle = "Mods",
		iup.hbox {
			iup.list {
				"[x] fixlocale",
				"[x] launcher",
				"[x] catchall",
				expand="yes",
			},
			iup.vbox {
				normalizesize="HORIZONTAL",
				iup.button{title="Enable"},
				iup.button{title="Disable"},
				iup.button{title="Move up"},
				iup.button{title="Move down"},
				iup.button{title="Add"},
				iup.button{title="Delete"},
			}
		}
	},

	iup.hbox {
		tabtitle = "Scripts",
		iup.hbox {
			iup.list {
				"[x] fixlocale",
				"[x] launcher",
				"[x] catchall",
				expand="yes",
			},
			iup.vbox {
				normalizesize="HORIZONTAL",
				iup.button{title="Enable"},
				iup.button{title="Disable"},
				iup.button{title="Move up"},
				iup.button{title="Move down"},
				iup.button{title="Add"},
				iup.button{title="Delete"},
				iup.button{title="Edit / Debug"},
				iup.button{title="Reload"},

			}
		}
	},


	canfocus = "no",
	tabchange_cb = function(ot,nt)
		if nt.tabtitle == "Console" then
			iup.SetFocus(repl)
			console.scrolltopos = #console.value
		end
	end,
	iup.hbox {
		tabtitle = "Console",
		iup.vbox {
			console,
			iup.hbox {
				repl,
				iup.button{title="Run",action=function() repl:k_any(13) end,canfocus="no"}
			}
		}
	}
}

dlg = iup.dialog {
	iup.vbox {
		tabs,
	};
	title = "AA2Unlimited",
}

function dlg:close_cb()
	os.exit(0)
end

return function()
	dlg:showxy(iup.CENTER, iup.CENTER)
	_EVENTS.logger = function(buf)
		console.append = buf
		--console.scrolltopos = #console.value
		iup.SetFocus(repl)
		return true
	end
	log("dialog shown")
	iup.MainLoop()
end

