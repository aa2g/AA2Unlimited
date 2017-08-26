require "iuplua"
require "iupluacontrols"
require "memory"

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

local signals = require "poser.signals"
local lists = require "poser.lists"
local toggles = require "poser.toggles"
local charamgr = require "poser.charamgr"
local unpack = table.unpack

local boneentries = {}
local categoryentries = {}

local framecfg = require "poser.framelist"

local bones = {
	bonemap = {},
	bones = {},
	categories = { },
	props = {},
	rooms = {},
}

for _,v in ipairs(framecfg) do
	local name = v[3]
	local frame = v[1]
	bones.bonemap[name] = frame
	table.insert(bones.bones, name)
	for _,g in ipairs(v[2]) do
		bones.categories[g] = bones.categories[g] or {}
		table.insert(bones.categories[g], name)
	end
end

local function shapecontrols(title, shapelist, opts)
	local shapename = opts.name
	local shapeopen = shapename .. "open"
	local shapeselected = function(shape)
		local character = charamgr.current
		if character then
			character[shapename] = shape
		end
	end
	local shapeopened = function(open)
		local character = charamgr.current
		if character then
			character[shapeopen] = open
		end
	end
	
	local controls = {}
	for i, s in ipairs(shapelist) do
		local button = iup.flatbutton { title = s, toggle ="yes", padding = 3 }
		function button.flat_action(self)
			if self.value == "ON" then
				shapeselected(i - 1)
			end
		end
		table.insert(controls, button)
	end
	local open = iup.label { title = "Open" }
	local spin = iup.text { spin = "yes", spinvalue = 0, spinmin = 0, spinmax = 9, visiblecolumns = 1 }
	function spin.valuechanged_cb(self)
		shapeopened(tonumber(self.value))
	end
	
	local norm = iup.normalizer { unpack(controls) }
	norm.normalize = "horizontal"
	return iup.frame { title = title, 
		iup.vbox {
			iup.radio {
				iup.gridbox { numdiv = opts.cols, unpack(controls) },
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
		},
		shapeselected = shapeselected
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

local bonefilter = lists.listfilter()
local categorylist = lists.listbox { sort = "yes" }
local bonelist = lists.listbox {}

local categories = { "All", "Props", "Room" }
for cat,_ in pairs(bones.categories) do
	table.insert(categories, cat)
end
categorylist.setlist(categories)

local function setcategory(category)
	if category == "All" then
		bonelist.setlist(bones.bones)
	else
		bonelist.setlist(bones.categories[category] or {})
	end
end
setcategory("All")

signals.connect(categorylist, "selectionchanged", setcategory)
signals.connect(bonefilter, "setfilter", bonelist, "setfilter")

local characterlist = lists.listbox { lines = 2, expand = "horizontal" }
local stylelist = lists.listbox { lines = 4, expand = "horizontal" }

local characterschanged = signals.signal()

local function updatecurrentcharacter(_, index)
	charamgr.setcurrentcharacter(tonumber(index))
end
signals.connect(characterlist, "selectionchanged", updatecurrentcharacter)

local function updatecharacterlist()
	log.spam("Updating character list")
	local cur
	local list = {}
	for i,v in ipairs(charamgr.characters) do
		if v == charamgr.current then
			cur = i
		end
		table.insert(list, v.name)
	end
	characterlist.setlist(list)
	characterlist.value = cur
end
signals.connect(charamgr, "characterschanged", updatecharacterlist)

local modifiers = {
	{ 30 * math.pi / 180, 90 * math.pi / 180, 180 * math.pi / 180 },
	{ 1, 10, 100 },
	{ 1, 10, 100 },
}
local modifier = modifiers[1][1]
local currentmodifier = 1
local currentoperation = 1
local currentslider
local dummyslider = {}
local dummymt = {
	__call = function()
		log.warn("calling dummy slider")
		return dummyslider
	end,
	
	__index = function()
		log.warn("indexing dummy slider")
		return dummyslider
	end
}
setmetatable(dummyslider, dummymt)

local sliders = require "poser.sliders"
local sliderx = sliders.slider { title = "X", data = 0 }
local slidery = sliders.slider { title = "Y", data = 1 }
local sliderz = sliders.slider { title = "Z", data = 2 }

local rotatebutton = toggles.button { title = "Rotate", data = 1 }
local scalebutton = toggles.button { title = "Scale", data = 3 }
local translatebutton = toggles.button { title = "Translate", data = 2 }

local modifierx1 = toggles.button { title = "x1", data = 1 }
local modifierx10 = toggles.button { title = "x10", data = 2 }
local modifierx100 = toggles.button { title = "x100", data = 3 }

local slidermod
local sliderop
local slider

local function sliderincrement(amount, axis)
	if currentslider then
		currentslider:Increment(amount * modifier, axis)
	end
end

local function slidestarted()
	if currentslider then
		currentslider:StartSlide()
	end
end

local function slidestopped()
	if currentslider then
		currentslider:StopSlide()
	end
end

local function updatemodifier()
	modifier = modifiers[currentoperation][currentmodifier]
	log.spam("increment modifier set to %f", modifier)
end

local function slidersetmodifier(modifier)
	log.spam("set current modifier to %d", modifier)
	currentmodifier = modifier
	updatemodifier()
end

local function slidersetoperation(operation)
	currentoperation = operation
	log.spam("set current slider operation to %d", currentoperation)
	if currentslider then
		currentslider:SetCurrentOperation(operation - 1)
	end
	updatemodifier()
end

local function sliderchanged()
	currentslider = dummyslider
	local slidername = bones.bonemap[bonelist[bonelist.value or ""]] or ""
	log.spam("Try to get slider %s from %s", slidername, charamgr.current)
	if charamgr.current then
		local slider = charamgr.current:GetSlider(slidername)
		currentslider = slider or dummyslider
		log.spam("Poser: Set slider to %s", currentslider)
	end
	slidersetoperation(currentoperation)
end
signals.connect(bonelist, "selectionchanged", sliderchanged)
signals.connect(charamgr, "currentcharacterchanged", sliderchanged)
signals.connect(charamgr, "characterschanged", sliderchanged)
signals.connect(sliderx, "increment", sliderincrement)
signals.connect(slidery, "increment", sliderincrement)
signals.connect(sliderz, "increment", sliderincrement)

signals.connect(sliderx, "slidestarted", slidestarted)
signals.connect(slidery, "slidestarted", slidestarted)
signals.connect(sliderz, "slidestarted", slidestarted)

signals.connect(sliderx, "slidestopped", slidestopped)
signals.connect(slidery, "slidestopped", slidestopped)
signals.connect(sliderz, "slidestopped", slidestopped)

signals.connect(rotatebutton, "selected", slidersetoperation)
signals.connect(translatebutton, "selected", slidersetoperation)
signals.connect(scalebutton, "selected", slidersetoperation)

signals.connect(modifierx1, "selected", slidersetmodifier)
signals.connect(modifierx10, "selected", slidersetmodifier)
signals.connect(modifierx100, "selected", slidersetmodifier)

local selectroom = iup.list { dropdown="yes",  expand = "horizontal" }
local current_room

function selectroom:map_cb()
	local added={}
	local list = PPReadFile(play_path("data","jg2p09_00_00.pp"),"MP_ITEM.lst")
	self.appenditem = "None"
	for w in list:gmatch("%S+") do
		if w ~= "-" and (not w:match("^MP_ITEM")) and w:match("^MP_") and (not added[w]) then
			added[w] = true
			self.appenditem = w
		end
	end
end

function selectroom:action(text,itno)
	local xxlist
	-- TODO: detect if we're in h, and don't load our XX there
	if exe_type == "edit" then
		xxlist = GameBase + 0x00353290
	else
		xxlist = GameBase + 0x00376298
	end
	if current_room then
		current_room:Unload(xxlist)
	end
	current_room = text ~= "None" and LoadXX(xxlist, play_path("data","jg2p01_00_00.pp"),text .. ".xx",0) or nil
end

local mouthshapes = shapecontrols("Mouth", { ":|", ":)", ":(", ":3", ":3" , ":O", ":s", "", ":[]", ":o", ":·", ":D", ":]", "", ":]", ":>"}, { name = "mouth", cols = 4 })
local eyeshapes = shapecontrols("Eyes", { "u_u", "n_n", "^_^", "-_-", "o_u", "u_o", "o_n", "n_o" }, { name = "eye", cols = 2 })

local dialogsliders = iup.dialog {
	iup.hbox {
		nmargin = "7x7",
		iup.frame {
			title = "Characters",
			iup.vbox {
				characterlist,
				iup.label { title = "Styles" },
				stylelist,
				iup.button { title = "Edit clothes", expand = "horizontal" },
				iup.button { title = "Edit overrides", expand = "horizontal" },
				selectroom,
			},
		},
		iup.frame {
			title = "Bones",
			ncmargin = "3x3",
			iup.hbox {
				categorylist,
				iup.vbox {
					bonefilter, bonelist,
					expand = "yes",
				},
			},
		},
		iup.vbox {
			iup.frame {
				title = "Sliders",
				iup.vbox {
					iup.hbox {
						iup.label { title = "Operation" },
						iup.radio {
							iup.hbox {
								rotatebutton,
								scalebutton,
								translatebutton,
								gap = 3,
							}
						},
						iup.label { separator = "vertical" },
						iup.label { title = "Modifier" },
						iup.radio {
							iup.hbox {
								modifierx1,
								modifierx10,
								modifierx100,
								gap = 3,
							}
						},
						iup.vbox {
							iup.flatbutton { title = "Z-Copy", toggle = "no", border = "yes", padding = 3 },
							alignment = "aright",
							expand = "horizontal",
						},
						alignment = "acenter",
						expand = "horizontal",
						gap = 7
					},
					sliderx,
					slidery,
					sliderz,
				},
			},
			iup.hbox {
				mouthshapes,
				eyeshapes,
				facecontrols(),
				expand = "yes",
			}
		},
		--gap = 3,
	},
	nmargin = "3x3",
	maxbox = "no",
	minbox = "no",
}

local dialogposes = require "poser.posemgr"
signals.connect(dialogposes, "loadpose", _M, "loadpose")
signals.connect(dialogposes, "savepose", _M, "savepose")
signals.connect(dialogposes, "loadscene", _M, "loadscene")
signals.connect(dialogposes, "savescene", _M, "savescene")

-- This hack is inspired by
-- https://www.codeproject.com/Articles/11114/Move-window-form-without-Titlebar-in-C
-- Of course iup supports nothing of the sorts, so we have to do it the dirty way
local function adjust_parenting(v)
	if not _M.forceparenting then return end
	v.menubox = "no"
	v:map()
	set_window_proc(v.hwnd, function(orig, this, hwnd, msg, wparam, lparam)
		if msg == 0x0201 and hwnd == fixptr(v.hwnd) then
			ReleaseCapture()
			SendMessageW(hwnd, 0xA1, 2, 0)
		end
		return CallWindowProcW(orig, hwnd, msg, wparam, lparam)
	end)
end

function _M.togglevisible()
	dialogs = dialogs or { dialogposes, dialogsliders }
	if not _M.visible then
		for _,v in ipairs(dialogs) do
			adjust_parenting(v)
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
	if _M.forceparenting then parent = _M.parentHWND end
	for _,v in ipairs(d) do
		SetParent(v.hwnd, parent)
	end
end

function _M.hotkey_cb(dialog, k, ...)
	if k == iup.K_F12 then
		_M.togglevisible()
	end
end

function _M.close_all()
	dialogs = dialogs or { dialogposes, dialogsliders }
	for _,d in ipairs(dialogs or {}) do
		d:destroy()
	end
end

signals.connect(dialogsliders, "k_any", _M, "hotkey_cb")
signals.connect(dialogposes, "k_any", _M, "hotkey_cb")

return _M
