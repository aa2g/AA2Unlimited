require "iuplua"
require "iupluacontrols"
require "memory"

local _M = { 
	visible = false,
	floating = false,
	block_mouse = false,
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
local posemgr = require "poser.posemgr"
local propmgr = require "poser.propmgr"
local fileutils = require "poser.fileutils"
local unpack = table.unpack

local boneentries = {}
local categoryentries = {}

local framecfg = require "poser.framelist"


-- -------
-- Locals
-- -------

local dummyslider = {}
local dummymt = {
	__call = function()
		-- log.spam("calling dummy slider")
		return dummyslider
	end,
	
	__index = function()
		-- log.spam("indexing dummy slider")
		return dummyslider
	end
}
setmetatable(dummyslider, dummymt)
local currentslider = dummyslider

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


-- Signals

local characterschanged = signals.signal()


-- -----------
-- UI Bones
-- -----------

local bonefilter = lists.listfilter()
local categorylist = lists.listbox { }
local bonelist = lists.listbox {}

local propbonefilter = lists.listfilter()
local propbonelist = lists.listbox { sort = "yes" }
signals.connect(propbonefilter, "setfilter", propbonelist, "setfilter")

local function showframe(frame, show)
	show = show and 0 or 2
	frame.m_renderFlag = show
	frame.m_meshFlagHide = show
end

local bonelistzbox = iup.zbox {
	iup.backgroundbox {
		iup.hbox {
			categorylist,
			iup.vbox {
				bonefilter,
				bonelist,
				iup.hbox {
					iup.button {
						title = "Show",
						expand = "horizontal",
						action = function()
							showframe(currentslider.frame, true)
						end,
					},
					iup.button {
						title = "Hide",
						expand = "horizontal",
						action = function()
							showframe(currentslider.frame, false)
						end,
					},
				},
				expand = "yes",
			},
		},
	},
	iup.backgroundbox {
		iup.vbox {
			propbonefilter,
			propbonelist,
			iup.hbox {
				iup.button {
					title = "Show",
					expand = "horizontal",
					action = function()
						showframe(currentslider.frame, true)
					end,
				},
				iup.button {
					title = "Hide",
					expand = "horizontal",
					action = function()
						showframe(currentslider.frame, false)
					end,
				},
			},
			expand = "yes",
		},
	},
	expand = "yes",
}

local categories = { "All", "Torso", "Left Arm", "Right Arm", "Left Hand", "Right Hand", "Left Leg", "Right Leg", "Face", "Breasts", "Skirt", "Props", "Other" }
categorylist.setlist(categories)

local function setcategory(category)
	if category == "All" then
		bonelist.setlist(bones.bones)
	elseif category == "Props" then
		local props = {}
		local character = charamgr.current
		if character and character.ischaracter then
			for p,_ in character:props() do
				table.insert(props, p)
			end
		end
		bonelist.setlist(props)
	elseif category == "Room" then
		bonelist.setlist({})
	else
		bonelist.setlist(bones.categories[category] or {})
	end
end
setcategory("All")

signals.connect(categorylist, "selectionchanged", setcategory)
signals.connect(bonefilter, "setfilter", bonelist, "setfilter")

local characterlist = lists.listbox { lines = 8, expand = "yes" }

local function updatecurrentcharacter(_, index)
	charamgr.setcurrentcharacter(tonumber(index))
	_M.restorecharui()
end
signals.connect(characterlist, "selectionchanged", updatecurrentcharacter)

posemgr.poseloaded.connect(function(character) if character == charamgr.current then _M.restorecharui() end end)

local function updatecharacterlist()
	local cur
	local list = {}
	for i,v in ipairs(charamgr.characters) do
		if v == charamgr.current then
			cur = i
		end
		table.insert(list, '#%d: %s' % {v.struct.m_seat, v.name})
	end
	characterlist.setlist(list)
	characterlist.value = cur
end
signals.connect(charamgr, "characterschanged", updatecharacterlist)

local charswapprevbutton = iup.button { title = "/\\", expand = "vertical", action = function() charamgr.swapprev(tonumber(characterlist.value)) end }
local charswapnextbutton = iup.button { title = "\\/", expand = "vertical", action = function() charamgr.swapnext(tonumber(characterlist.value)) end }


-- -----------------
-- Char spawning
-- -----------------


local addcharbutton = exe_type == "play" and iup.button { title = "Add", expand = "horizontal" } or {}
local removecharbutton = exe_type == "play" and iup.button { title = "Remove", expand = "horizontal" } or {}

function addcharbutton.action()
	local seats = {}
	local list = {
		expand = "horizontal"
	}
	for i=0,24 do
		local char = GetCharacter(i)
		if char then
			local name = sjis_to_utf8(string.format("#%d: %s %s", i, char.m_charData.m_forename, char.m_charData.m_surname))
			table.insert(list, name)
			table.insert(seats, i)
		end
	end

	local pick
	local chlist = iup.list(list)
	local charsel
	local function chsel()
		charsel:destroy()
	end
	function chlist.dblclick_cb()
		pick = chlist.value
		return iup.CLOSE
	end
	
	charsel = iup.dialog {
		iup.vbox {
			chlist,
			iup.button {
				title = "Spawn",
				expand = "horizontal",
				action = function()
					pick = chlist.value
					return iup.CLOSE
				end
			},
		}
	}
	charsel:popup()
	local char = seats[tonumber(pick)]
	if char then
		char = GetCharacter(char)
		charamgr.spawn(char, 1, 0)
		updatecharacterlist()
	end
end

function removecharbutton.action()
	charamgr.current:despawn()
	updatecharacterlist()
end


-- ----------
-- UI Props
-- ----------

local proplist = lists.listbox { lines = 8, expand = "yes" }
local addpropbutton = iup.button { title = "Add", expand = "horizontal" }
local removepropbutton = iup.button { title = "Remove", expand = "horizontal" }

local attachpropsbutton = iup.button { title = "Add CharProps", expand = "horizontal" }
local detachpropsbutton = iup.button { title = "Detach", expand = "horizontal" }

local normalizeraddremove = iup.normalizer { addpropbutton, removepropbutton, attachpropsbutton, detachpropsbutton, addcharbutton, removecharbutton }
normalizeraddremove.normalize = "horizontal"
normalizeraddremove:destroy()

function addpropbutton.action()
	local selected = fileutils.getfiledialog(aau_path("poser\\items\\*.xx"))
	if selected then
		propmgr.loadprop(selected)
	end
end

function removepropbutton.action()
	local index = proplist.value
	if index and index ~= 0 then
		log.spam("removing prop %d", index)
		propmgr.unloadprop(index)
	end
end

function attachpropsbutton.action()
	local path = fileutils.getfiledialog(aau_path("poser\\charitems\\*.xx"))
	if not path then return end
	log.spam("loading charitem %s", path)
	local character = charamgr.current
	if not character.origskel then
		character.origskel = character.skelname
	end
	character:override(character.origskel .. ".xx", path)
	character:reload()
	log.spam("re-spawned character")
end

function detachpropsbutton.action()
	local character = charamgr.current
	if character.origskel then
		character:override(character.origskel .. ".xx", nil)
		character:reload()
	end
end

local function updateproplist()
	-- local cursel =
	local i = 1
	for _,p in ipairs(propmgr.props) do
		proplist[i] = p.name
		i = i + 1
	end
	proplist[i] = nil
end
propmgr.propschanged.connect(updateproplist)


-- -----------
-- UI Sliders
-- -----------

local modifiers = {
	{ 30 * math.pi / 180, 90 * math.pi / 180, 180 * math.pi / 180 },
	{ 1, 10, 100 },
	{ 1, 10, 100 },
}
local modifier = modifiers[1][1]
local currentmodifier = 1
local currentoperation = 1

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

local function setslidervalues()
	local x, y, z = currentslider:Values()
	x = tonumber(x) or ""
	y = tonumber(y) or ""
	z = tonumber(z) or ""
	sliderx.setvalue(x)
	slidery.setvalue(y)
	sliderz.setvalue(z)
end

local function slidervaluechanged(value, axis)
	local va = { sliderx.getvalue(), slidery.getvalue(), sliderz.getvalue() }
	va[axis + 1] = value
	currentslider:SetValues(va[1], va[2], va[3])
	currentslider:Apply()
end

local function sliderincrement(amount, axis)
	currentslider:Increment(amount * modifier, axis)
	setslidervalues()
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
	-- log.spam("increment modifier set to %f", modifier)
end

local function slidersetmodifier(modifier)
	-- log.spam("set current modifier to %d", modifier)
	currentmodifier = modifier
	updatemodifier()
end

local function slidersetoperation(operation)
	currentoperation = operation
	-- log.spam("set current slider operation to %d", currentoperation)
	if currentslider then
		currentslider:SetCurrentOperation(operation - 1)
	end
	updatemodifier()
	setslidervalues()
end

local function setcurrentslider(slider)
	currentslider = slider or dummyslider
	-- log.spam("Poser: Set slider to %s", currentslider)
	setslidervalues()
end

local function propsliderchanged()
	local bone = propbonelist[tonumber(propbonelist.value)]
	local prop = propmgr.props[tonumber(proplist.value)]
	if prop and bone then
		setcurrentslider(prop:getslider(bone))
	else
		setcurrentslider(dummyslider)
	end
	slidersetoperation(currentoperation)
end

local function propchanged()
	local prop = propmgr.props[tonumber(proplist.value)]
	if prop then
		local newlist = {}
		local i = 1
		for k,_ in prop:sliders() do
			newlist[i] = k
			i = i + 1
		end
		propbonelist.setlist(newlist)
		propsliderchanged()
	end
end

local function sliderchanged(slidertype)
	if slidertype == 1 then -- if prop bones tab
		propsliderchanged()
		return
	end
	local slidername = bonelist[bonelist.value or ""]
	slidername = bones.bonemap[slidername] or slidername or ""
	-- log.spam("Try to get slider %s from %s", slidername, charamgr.current)
	if charamgr.current then
		local slider = charamgr.current:getslider(slidername)
		setcurrentslider(slider)
	else
		setcurrentslider(dummyslider)
	end
	slidersetoperation(currentoperation)
end

signals.connect(proplist, "selectionchanged", propchanged)
signals.connect(propbonelist, "selectionchanged", propsliderchanged)

signals.connect(bonelist, "selectionchanged", sliderchanged)
signals.connect(charamgr, "currentcharacterchanged", sliderchanged)
signals.connect(charamgr, "characterschanged", sliderchanged)

signals.connect(sliderx, "valuechanged", slidervaluechanged)
signals.connect(slidery, "valuechanged", slidervaluechanged)
signals.connect(sliderz, "valuechanged", slidervaluechanged)

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


-- ---------
-- UI Face
-- ---------

local dimeyes = iup.flatbutton { title = "Dim Eyes", toggle = "yes", border = "yes", padding = 3, valuechanged_cb = function(self) if charamgr.current then charamgr.current.dimeyes = self.value == "ON" end end, expand = "horizontal" }
dimeyes.size = "x12"
dimeyes.expand = "horizontal"
local tears = iup.flatbutton { title = "Tears", toggle = "yes", border = "yes", padding = 3, valuechanged_cb = function(self) if charamgr.current then charamgr.current.tears = self.value == "OFF" end end, expand = "horizontal" }
tears.size = "x12"
tears.expand = "horizontal"
local eyetracking = iup.flatbutton { title = "Eye Tracking", toggle = "yes", border = "yes", padding = 3, valuechanged_cb = function(self) if charamgr.current then charamgr.current.eyetracking = self.value == "ON" and 1 or 0 end end, expand = "horizontal" }
eyetracking.size = "x12"
eyetracking.expand = "horizontal"
local yogurt = iup.flatbutton { title = "Yogurt", toggle = "yes", border = "yes", padding = 3, valuechanged_cb = function(self) if charamgr.current then charamgr.current.tonguejuice = self.value == "OFF"; charamgr.current.mouthjuice = self.value == "OFF" end end, expand = "horizontal" }
yogurt.size = "x12"
yogurt.expand = "horizontal"

local resetsliderbutton = iup.flatbutton { title = "Reset", toggle = "no", border = "yes", padding = 3 }
function resetsliderbutton.flat_action()
	currentslider:Reset()
	currentslider:Apply()
end

local blushslider = iup.val { orientation = "horizontal", expand = "horizontal", min = 0, max = 1.2, value = 0,
	valuechanged_cb = function(self)
		local face = charamgr.current.struct.m_xxFace
		local material = face:FindMaterial("A00_M_hoho") or face:FindMaterial("S00_M_hoho") 
		if material then
			material:m_lightingAttributes(3, tonumber(self.value))
		end
	end
}
local blushlinesslider = iup.val { orientation = "horizontal", expand = "horizontal", min = 0, max = 1.2, value = 0,
	valuechanged_cb = function(self)
		local face = charamgr.current.struct.m_xxFace
		local material = face:FindMaterial("A00_M_hohosen") or face:FindMaterial("S00_M_hohosen") 
		if material then
			material:m_lightingAttributes(3, tonumber(self.value))
		end
	end
}



-- ------------
-- UI Shapes
-- ------------

local function shapecontrols(shapelist, opts)
	local shapename = opts.name
	local shapeopen = shapename .. "open"
	
	local shapeselected = function(shape)
		local character = charamgr.current
		if character then
			character[shapename] = shape
		end
	end

	local controls = {}
	for i, s in ipairs(shapelist) do
		local button = iup.flatbutton { title = s, toggle = "yes", padding = 3, size = opts.buttonsize }
		function button.valuechanged_cb(self)
			if self.value == "ON" then
				shapeselected(i - 1)
			end
		end
		table.insert(controls, button)
	end
	
	return iup.vbox {
		iup.radio {
			iup.gridbox {
				numdiv = opts.cols,
				unpack(controls),
			},
			gap = 3,
		},
		shapecontrols = controls,
	}
end

local mouthspin = iup.text { 
	spin = "yes",
	spinvalue = 0,
	spinmin = 0,
	spinmax = 9,
	visiblecolumns = 1,
	valuechanged_cb = function(self)
		local character = charamgr.current
		if character then
			character["mouthopen"] = tonumber(self.value)
		end
	end
}

local eyespin = iup.text {
	spin = "yes",
	spinvalue = 0,
	spinmin = 0,
	spinmax = 9,
	visiblecolumns = 1,
	valuechanged_cb = function(self)
		local character = charamgr.current
		if character then
			character["eyeopen"] = tonumber(self.value)
		end
	end
}

local mouthshapescontrols = shapecontrols({ "°_°", "°◡°", "°∩°", "°w°", "°ω°" , "°O°", "°~°", "° °", "°д°", "°o°", "°3°", "°▽°", "°ㅂ°", "°-°", "°ت°", "°v°", "°#°", "°⌓°", "°Ә°" }, { name = "mouth", cols = 4, buttonsize = "20x12" })
local mouthshapes = iup.vbox {
	mouthshapescontrols,
	iup.hbox {
		yogurt,
		iup.label { title = "Open" },
		mouthspin,
		alignment = "acenter",
		gap = 3,
	},
	gap = 3,
}

local eyeshapescontrols = shapecontrols({ "u_u", "n_n", "^_^", "-_-", "o_u", "u_o", "o_n", "n_o" }, { name = "eye", cols = 4, buttonsize = "20x12" })
local eyeshapes = iup.vbox {
	eyeshapescontrols,
	iup.hbox {
		iup.label { title = "Open" },
		eyespin,
		alignment = "acenter",
		gap = 3,
	},
	alignment = "aright",
	gap = 3,
}

local eyebrowshapescontrols = shapecontrols({ "1", "2", "3", "4", "5", "6", "7"}, { name = "eyebrow", cols = 7, buttonsize = "15x12" })
local eyebrowshapes = iup.vbox {
	eyebrowshapescontrols,
}


-- ------------
-- Restore UI
-- ------------

local restoreuitable = {
	eye = function(v)
		eyeshapescontrols.shapecontrols[v + 1].value = "ON"
	end,
	eyeopen = function(v)
		eyespin.value = v
	end,
	eyebrow = function(v)
		eyebrowshapescontrols.shapecontrols[v % 7 + 1].value = "ON"
	end,
	mouth = function(v)
		mouthshapescontrols.shapecontrols[v % 7 + 1].value = "ON"
	end,
	mouthopen = function(v)
		mouthspin.value = v
	end,
}
function _M.restorecharui()
	for k,v in pairs(charamgr.current.cache) do
		if restoreuitable[k] then
			restoreuitable[k](v)
		end
	end
end


-- -----------
-- Clothing UI
-- -----------

-- local stylelist = iup.list { lines = 4, expand = "horizontal", dropdown = "yes" }

local dlgclothes = require "poser.dlgclothes"


-- -----------
-- Light UI
-- -----------

local dlglight = require "poser.dlglight"

-- -----------
-- UI Emoji
-- -----------

local dlgemoji = require "poser.dlgemoji"


-- -----------
-- UI Layout
-- -----------

local dialogsliders = iup.dialog {
	iup.hbox {
		nmargin = "7x7",
		iup.tabs {
			tabchangepos_cb = function(self, new, old)
				bonelistzbox.valuepos = new
				sliderchanged(new)
			end,
			iup.vbox {
				tabtitle = "Characters",
				iup.hbox {
					characterlist,
					iup.vbox {
						charswapprevbutton,
						charswapnextbutton,
					},
				},
				iup.hbox {
					addcharbutton,
					removecharbutton,
				},
				iup.hbox {
					attachpropsbutton,
					detachpropsbutton,
				},
				iup.hbox {
					iup.button { title = "T-Pose", expand = "horizontal", action = function() posemgr.tpose(charamgr.current) end },
					iup.button { title = "Reset Pose", expand = "horizontal", action = function() posemgr.resetpose(charamgr.current) end },
					iup.button { title = "Load XA", expand = "horizontal", action = function()
						local character = charamgr.current
						local path
						if character.ischaracter == true then path = fileutils.getfiledialog(aau_path("poser\\*.xa")) end
						if not path then return end
						local directory, filename, extension = fileutils.splitfilepath(path)
						filename = filename .. ".xa"
						character.struct:LoadXA(directory .. ".pp", filename, 0, 0, 0)
					end},
				},
				expand = "yes",
				gap = 3,
			},
			iup.vbox {
				tabtitle = "Props",
				iup.vbox {
					proplist,
					iup.hbox {
						addpropbutton,
						removepropbutton,
					},
				},
				expand = "yes",
			},
		},
		iup.tabs {
			iup.hbox {
				ncmargin = "3x3",
				tabtitle = "Bones",
				iup.vbox {
					bonelistzbox,
					expand = "yes",
				},
				iup.vbox {
					iup.frame {
						title = "Sliders",
						expand = "horizontal",
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
									resetsliderbutton,
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
						iup.frame { title = "Mouth",
							iup.vbox {
								mouthshapes,
							},
							expand = "no",
						},
						iup.frame { title = "Eyes",
							iup.vbox {
								eyeshapes,
								dimeyes,
								tears,
								eyetracking,
							},
								expand = "no"
						},
						iup.vbox {
							iup.frame { title = "Eyebrow",
								iup.vbox {
									eyebrowshapes
								},
							},
							iup.frame { title = "Blush",
								iup.vbox {
									blushslider,
									blushlinesslider,
								},
								expand ="yes",
							},
							expand = "no",
						},
					},
					expand = "no",
				},
			},
			iup.hbox {
				tabtitle = "Clothes",
				dlgclothes,
			},
			iup.hbox {
				tabtitle = "Light",
				dlglight,
			},
			iup.hbox {
				tabtitle = "Emotes",
				dlgemoji,
			},
		},
		--gap = 3,
	},
	nmargin = "3x3",
	maxbox = "no",
	minbox = "no",
}

dialogposes = posemgr.dialogposes
signals.connect(dialogposes, "loadpose", _M, "loadpose")
signals.connect(dialogposes, "savepose", _M, "savepose")
signals.connect(dialogposes, "loadscene", _M, "loadscene")
signals.connect(dialogposes, "savescene", _M, "savescene")

-- This hack is inspired by
-- https://www.codeproject.com/Articles/11114/Move-window-form-without-Titlebar-in-C
-- Of course iup supports nothing of the sorts, so we have to do it the dirty way
local function adjust_parenting(v)
	if _M.opts.notitle == 2 or (_M.opts.notitle == 1 and _M.fullscreen) then
		v.menubox = "no"
	end
	v:map()
	set_window_proc(v.hwnd, function(orig, this, hwnd, msg, wparam, lparam)
		-- left click, grab-mvoe
		if msg == 0x201 and _M.opts.grab == 1 and hwnd == fixptr(v.hwnd) then
			ReleaseCapture()
			SendMessageW(hwnd, 0xA1, 2, 0)
		-- mouse moves onto the window, grab focus
		elseif msg == 0x20 and (_M.opts.autofocus == 1) and (GetForegroundWindow() == GetGameHwnd()) then
			SetFocus(v.hwnd)
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
		_M.block_mouse = true
	else
		for _,v in ipairs(dialogs) do
			v:hide()
		end
		_M.visible = false
		_M.block_mouse = false
	end
end

function _M.updatefloating(d)
	d = d or dialogs
	if not d then return end
	local parent = 0
	if ((_M.opts.ontop == 2) or (_M.fs and _M.opts.ontop == 1)) then parent = _M.parentHWND end
	for _,v in ipairs(d) do
		SetWindowLongW(v.hwnd, -8, parent)
		--SetParent(v.hwnd, parent)
		--UpdateWindow(v.hwnd)
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
