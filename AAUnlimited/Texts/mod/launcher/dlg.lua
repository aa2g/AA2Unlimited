-- this file is a mess and always will be. keep function _only_ to gui.

local nocloseafterlaunch = true
local reslist = {
	"320x240",
	"512x384",
	"640x480",
	"768x576",
	"800x480",
	"854x480",
	"800x600",
	"960x600",
	"1024x600",
	"1024x768",
	"1152x768",
	"1280x720",
	"1280x800",
	"1280x854",
	"1280x960",
	"1366x768",
	"1400x1050",
	"1440x900",
	"1440x960",
	"1600x1200",
	"1680x1050",
	"1920x1080",
	"1920x1200",
	"2048x1080",
	"2048x1538",
	"2560x1600",
	"2560x2048",
	"3840x2160",
	"3840x1600",
	"4096x1716",
	"4096x2160"
}

log("loading launcher")

require "iuplua"
require "iupluacontrols"


local wantexit = false
local gsdlib = require "launcher.gsd"
local gsdconfig



local function gsdres(elem)
	elem.action = function(e,text,itno,state)
		if state ~= 1 then return end
		local x,y = text:match("^([0-9]+)x([0-9]+).*$")
		if not x or not y then return end
		x = tonumber(x)
		y = tonumber(y)
		if x< 100 or y < 100 or x > 10000 or y > 10000 then return end

		local function gcd(m, n)
		    while m ~= 0 do
				m, n = n%m, m
			end
			return n
		end

		factor = gcd(x,y)
		local ax = x/factor
		local ay = y/factor

		if ax > 128 or ay > 128 then return end

		gsdconfig.aspectx = ax
		gsdconfig.aspecty = ay
		gsdconfig.x = x
		gsdconfig.y = y
		gsdlib.save_gsd(gsdconfig)
	end
	elem.edit_cb = function(e,c,str)
		elem:action(str, -1, 1)
	end
	local res = gsdconfig.x .. "x" .. gsdconfig.y
	log("setting "..res)
	elem.valuestring = res
	elem.value = res
	return elem
end

local function gsdl(name, elem)
	elem.action = function(e,text,idx,state)
		if state ~= 1 then return end
		gsdconfig[name] = idx-1	
		gsdlib.save_gsd(gsdconfig)
	end
	elem.value = gsdconfig[name]+1
	return elem
end

local function gsdt(name, elem)
	elem.action = function(e,state)
		gsdconfig[name] = state
		gsdlib.save_gsd(gsdconfig)
	end
	elem.value = gsdconfig[name]
	return elem
end

local function aaul(name, elem)
	elem.action = function(e,text,idx,state)
		if state ~= 1 then return end
		Config[name] = idx-1	
		Config.save()
	end
	elem.value = Config[name]+1
	return elem
end

local function aaut(name, elem)
	elem.action = function(e,state)
		Config[name] = state == 1
		Config.save()
	end
	elem.value = Config[name] and 1 or 0
	return elem
end

local function aauv(name, elem)
	elem.action = function(e,n,s)
		if not name:match("^s.*") then
			Config[name] = tonumber(s)
		end
		Config.save()
	end
	elem.valuechanged_cb = function(e)
		elem:action(-1, e.value)
	end
	elem.value = tostring(Config[name])
	return elem
end


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
	local b = iup.button{title="Launch the " .. (_BINDING.IsAAPlay and "game" or "editor"), expand="HORIZONTAL", size="x32", margin="8x8",
	action=function()
		for _,v in ipairs(buts) do v.active="no" end
		wantexit  = true
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
	--iup.SetFocus(repl)
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


local function buildtabs() return
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
				padding="x8",
				orientation="HORIZONTAL", numdiv=2, normalizesize="HORIZONTAL", homogenouslin="YES", alignmentlin = "ACENTER" ,
				iup.label {title = "Resolution:", }, gsdres(iup.list(table.append({ dropdown="YES",editbox="YES",mask="^[0-9]+x[0-9]+$", visibleitems=#reslist }, reslist))),
				iup.label {title = "Antialiasing:",}, gsdl("aa", iup.list { 
					"None", "2xMSAA", "4xMSAA", "8xCSAA", "8xQ CSAA", "16x CSAA", "16xQ CSAA", dropdown="YES", visibleitems=9 }),
				iup.label {title = "Shadowmap:", }, gsdl("shadowmap", iup.list {
					"None", "256", "512", "1024", dropdown="YES" }),
				iup.label {title = "Mipmap level:", }, gsdl("mipmap", iup.list { "None", "Normal", "Best", dropdown="YES" }),
			},
		},
		iup.frame {
			title = "Toggles",
			iup.vbox {
				gsdt("fullscreen", iup.toggle {title = "Fullscreen" }),
				gsdt("fastrender", iup.toggle {title = "Type 2 renderer (fast)" }),
				gsdt("sharp", iup.toggle {title = "Smooth textures" }),
				gsdt("bilinear", iup.toggle {title = "Bilinear filtering" }),
				gsdt("svp", iup.toggle {title = "Software vertex processing" }),
				gsdt("blur", iup.toggle {title = "Blur" }),
				gsdt("rim", iup.toggle {title = "Rim lighting" }),
				gsdt("shaders", iup.toggle {title = "Shaders (tan/outline)" }),
			}
		},
	}, iup.fill{}, launch()},

	iup.vbox { 
		alignment="acenter",
		tabtitle = "AA2Unlimited",
		iup.fill{},
	iup.hbox {
		alignment="acenter",
		ngap="4",
		iup.frame {
			title = "Settings",
			iup.gridbox {
				padding="3x3",
				nmargin="x4",
				orientation="HORIZONTAL", numdiv=2, normalizesize="HORIZONTAL", homogenouslin="YES", alignmentlin = "ACENTER", floating = "yes",
				iup.label {title = "Default log priority" }, aaul("logPrio", iup.list { "spam","info","warn","error", "critical", dropdown="YES" }),
				iup.label {title = "Legacy cards" }, aaul("legacyMode", iup.list { "ignore", "load", "reinterpret", "convert", dropdown="YES" }),
				iup.label {title = "Card eye files", }, aaul("savedFileUsage", iup.list { "ask", "always extract", "dont extract", dropdown="YES" }),
				iup.label {title = "Screenshot format", }, aaul("screenshotFormat", iup.list { ".BMP", ".JPG", ".PNG", dropdown="YES" }),
				iup.label {title = "Poser hotkeys", padding=step }, aauv("sPoserHotKeys", iup.text {"WER", mask="^[A-Z]*$"}),
				iup.label {title = "POV X",  }, aauv("fPOVOffsetX", floatspinner(0,0.1)),
				iup.label {title = "POV Y", step },  aauv("fPOVOffsetY", floatspinner(0,0.1)),
				iup.label {title = "POV Z", },  aauv("fPOVOffsetZ", floatspinner(0,0.1)),
				iup.label {title = ".pp2 cache MB", },  aauv("PP2Cache", iup.text { spin="yes", spinmax=1024}),
				iup.label {title = ".pp2 audio MB", },  aauv("PP2AudioCache", iup.text { spin="yes", spinmax=1024} ),
			},
		},
		iup.frame {
			title = "Toggles",
			iup.vbox {
				gap="3",
				aaut("bUseAdditionalTanSlots", iup.toggle {title = "Additional tan slots (DANGEROUS)" }),
				aaut("bUseMeshTextureOverrides", iup.toggle {title = "Mesh/texture overrides/hooks" }),
				aaut("bUseHAi", iup.toggle {title = "H AI" }),
				aaut("bHAiOnNoPromptH", iup.toggle {title = "(evil) AI can force H on PC" }),
				aaut("bUseShadowing", iup.toggle {title = "Game file shadowing" }),
				aaut("bEnableHPosButtonReorder", iup.toggle {title = "Reorder H buttons" }),
				aaut("bUseClothesPoser", iup.toggle {title = "Clothes poser" }),
				aaut("bUseDialoguePoser", iup.toggle {title = "Dialogue poser" }),
				aaut("bSaveFileAutoRemove", iup.toggle {title = "Strip files from cards after extracting" }),
				aaut("bSaveFileBackup", iup.toggle {title = "Backup the card before stripping" }),
				aaut("bUsePP2", iup.toggle {title = "Use .pp2 resource loader" }),
				aaut("bUsePPeX", iup.toggle {title = "Use .ppx resource loader" }),
			}
		},
	}, iup.fill{}, launch() },

--[[	iup.hbox {
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
]]

	canfocus = "no",
	tabchange_cb = function(ot,nt)
		if nt.tabtitle == "Console" then
			--iup.SetFocus(repl)
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

end



return function()
	gsdconfig = gsdlib.load_gsd()
	iup.SetGlobal("UTF8MODE","YES")
	dlg = iup.dialog {
		iup.vbox {
			buildtabs(),
		};
		title = "AA2Unlimited 0.5 preview",
	}
	dlg.startfocus = buts[1]

	function dlg:close_cb()
		os.exit(0)
	end

	dlg:showxy(iup.CENTER, iup.CENTER)
	_EVENTS.logger = function(buf)
		console.append = buf
		--console.scrolltopos = #console.value
		--iup.SetFocus(repl)
		return true
	end
	log("dialog shown")
	while not wantexit do
		iup.MainLoop()
		log("wantexit "..wantexit)
	end
	log("handing over to game thread")
end

