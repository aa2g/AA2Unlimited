-- this file is a mess and always will be. keep function _only_ to gui.

local nocloseafterlaunch = true
local reslist = {
	"800x480","854x480","960x600","1024x600","1280x720","1280x800","1280x854","1280x960","1366x768",
	"1440x900","1440x960","1600x900","1680x1050","1920x1080","1920x1200","2048x1080","2048x1536",
	"2560x1600","2560x2048","3840x2160","3840x1600","4096x1716","4096x2160"
}

log("loading launcher")

require "iuplua"
require "iupluacontrols"
local dlglist = require "launcher.dlglist"

local ctrl = setmetatable({}, {__index=function(t,k)
	return iup.GetDialogChild(dlg, k)
end})


local wantexit = false
local gsdlib = require "launcher.gsd"
local gsdconfig


local function update_res(text)
	text = text or "1280x720"
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
	_CONFIG["res_"..exe_type] = x .. "x" .. y
	Config.save()
end

local function gsdres(elem)
	elem.action = function(e,text,itno,state)
		if state ~= 1 then return end
		update_res(text)
	end
	elem.edit_cb = function(e,c,str)
		elem:action(str, -1, 1)
	end
	local res = _CONFIG["res_" .. exe_type]
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
		else
			Config[name] = s
		end
		if name:match("^fPOV.*") then
			_BINDING.ApplyCameraAdjust()
		end
		Config.save()
	end
	elem.valuechanged_cb = function(e)
		elem:action(-1, e.value)
	end
	elem.value = tostring(Config[name])
	return elem
end


local sshift = 16
local function floatspinner(v,step)
	return iup.text {
		value=tostring(v),
		spin="YES",
		spinauto="NO",
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

local function change_font(btn)
	local fsel = iup.fontdlg{value=btn.font, title="Select game font"}
	fsel:popup(iup.CENTERPARENT, iup.CENTERPARENT)
	if (tonumber(fsel.status) == 1) then
		local fn = tostring(fsel.value):match("^[^,]*")
		btn.font = fn .. ", 9"
		btn.title = fn
		Config.sFont = fn
		Config.save()
	end
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
				gsdt("dynlight", iup.toggle {title = "DynLight shader" }),
				gsdt("outline", iup.toggle {title = "Outline shader" }),
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
				iup.label {title = "Poser hotkeys" }, aauv("sPoserHotKeys", iup.text {"WER", mask="^[A-Z]*$"}),
				iup.label {title = ".pp2 cache MB", },  aauv("PP2Cache", iup.text { spin="yes", spinmax=1024}),
				iup.label {title = ".pp2 audio MB", },  aauv("PP2AudioCache", iup.text { spin="yes", spinmax=1024} ),
--				iup.label {title = "Game font", padding=step}, iup.button {title=Config.sFont, font=Config.sFont .. ", 9", action=change_font},
			},
		},
		iup.frame {
			title = "Toggles",
			iup.vbox {
				gap="3",
				aaut("bUseAA2Face", iup.toggle {title = "Load AA2Face in edit (a must have)" }),
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
				aaut("bUseMKIII", iup.toggle {title = "MKIII (chinpo .bmp->.tga texture)" }),
			}
		},
	}, iup.fill{}, launch() },

	require("launcher.dlgmod")(),

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
	update_res(_CONFIG["res_"..exe_type])
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
--	while not wantexit do
		iup.MainLoop()
--		log("wantexit "..wantexit)
--	end
	log("handing over to game thread")
end

