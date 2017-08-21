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

local signals = require "poser.signals"
local lists = require "poser.lists"
local toggles = require "poser.toggles"
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

local characters = {}
local currentcharacter
local characterschanged = signals.signal()

local function updatecurrentcharacter(_, index)
	currentcharacter = characters[index]
end
signals.connect(characterlist, "selectionchanged", updatecurrentcharacter)

local function updatecharacterlist()
	log.info("Updating character list")
	local cur
	local list = {}
	for i,v in ipairs(characters) do
		if currentcharacter and v.struct == currentcharacter.struct then
			cur = i
		end
		table.insert(list, v.name)
	end
	log.info("%s characters", #list)
	characterlist.setlist(list)
	characterlist.value = cur
end
characterschanged.connect(updatecharacterlist)

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

local rotatebutton = toggles.button { title = "Rotate", data = 0 }
local scalebutton = toggles.button { title = "Scale", data = 2 }
local translatebutton = toggles.button { title = "Translate", data = 1 }

local modifierx1 = toggles.button { title = "x1", data = 1 }
local modifierx10 = toggles.button { title = "x10", data = 10 }
local modifierx100 = toggles.button { title = "x100", data = 100 }

local slidermod
local sliderop
local slider
local function sliderchanged()
	currentslider = dummyslider
	local slidername = bones.bonemap[bonelist[bonelist.value or ""]] or ""
	if currentcharacter then
		local slider = currentcharacter.poser:GetSlider(slidername)
		currentslider = slider or dummyslider
	end
end
signals.connect(bonelist, "selectionchanged", sliderchanged)

local function sliderincrement(amount, axis)
	if currentslider then
		currentslider:Increment(amount, axis)
	end
end
characterschanged.connect(sliderchanged)
signals.connect(sliderx, "increment", sliderincrement)
signals.connect(slidery, "increment", sliderincrement)
signals.connect(sliderz, "increment", sliderincrement)

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
				shapecontrols("Mouth", { ":|", ":)", ":(", ":3", ":3" , ":O", ":s", "", ":[]", ":o", ":·", ":D", ":]", "", ":]", ":>"}, 4),
				shapecontrols("Eyes", { "u_u", "n_n", "^_^", "-_-", "o_u", "u_o", "o_n", "n_o" }, 2),
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

dialogposes = iup.dialog {
	iup.vbox {
		iup.tabs {
			iup.vbox {
				lists.listbox { editbox = "yes" },
				tabtitle = "Poses"
			},
			iup.vbox {
				lists.listbox { editbox = "yes" },
				tabtitle = "Scenes"
			},
		},
		iup.hbox { 
			iup.button { title = "Load", expand = "horizontal" },
			iup.button { title = "Save", expand = "horizontal" },
			iup.button { title = "Delete" },
		},
		iup.hbox { 
			iup.label { title = "Clip" },
			iup.text { expand = "horizontal" },
			iup.label { title = "Frame" },
			iup.text { expand = "horizontal" },
			gap = 3,
			alignment = "acenter"
		},
	},
	nmargin = "3x3",
	maxbox = "no",
	minbox = "no",
}

function _M.addcharacter(character)
	local new = true
	local last = 0
	for i,v in ipairs(characters) do
		if v.struct == character then
			new = false
		end
		last = i
	end
	if new then
		local data = character.m_charData
		local name = string.format("%s %s", data.m_forename, data.m_surname)
		new = { name = name, struct = character, poser = GetPoserCharacter(character) }
		characters[last + 1] = new
	end
	currentcharacter = currentcharacter or new
	log.info("add character = %s", currentcharacter or "no currentcharacter")
	characterschanged()
end

function _M.removecharacter(character)
	if currentcharacter and currentcharacter.struct == character then
		currentcharacter = nil
	end
	for k,v in pairs(characters) do
		if v.struct == character then
			table.remove(characters, k)
		end
	end
	characterschanged()
end

function _M.updatereferences()
end

function _M.togglevisible()
	dialogs = dialogs or { dialogposes, dialogsliders }
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

signals.connect(dialogsliders, "k_any", _M, "hotkey_cb")
signals.connect(dialogposes, "k_any", _M, "hotkey_cb")

return _M
