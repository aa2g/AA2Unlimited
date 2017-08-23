local _M = {}

local signals = require "poser.signals"
local json = require "poser.json"
local lists = require "poser.lists"

local function buildpath(basepath, filename)
end

local loadpose = signals.signal()
local savepose = signals.signal()
local loadscene = signals.signal()
local savescene = signals.signal()

local poselist = lists.listbox { editbox = "yes" }
local scenelist = lists.listbox { editbox = "yes" }
local loadposebutton = iup.button { title = "Load", expand = "horizontal" }
local saveposebutton = iup.button { title = "Save", expand = "horizontal" }
local loadscenebutton = iup.button { title = "Load", expand = "horizontal" }
local savescenebutton = iup.button { title = "Save", expand = "horizontal" }
local deleteposebutton = iup.button { title = "Delete" }
local deletescenebutton = iup.button { title = "Delete" }

local clipchanged= signals.signal()
local cliptext = iup.text { spin = "yes", spinvalue = 0, spinmin = 0, spinmax = 9999, visiblecolumns = 2, expand = "horizontal" }
function cliptext.valuechanged_cb(self)
	log.spam("clip text changed")
	local n = tonumber(self.value)
	if n then clipchanged(n) end
end

local framechanged= signals.signal()
local frametext = iup.text { spin = "yes", spinvalue = 0, spinmin = 0, spinmax = 9999, visiblecolumns = 2, expand = "horizontal" }
function frametext.valuechanged_cb(self)
	log.spam("frame text changed")
	local n = tonumber(self.value)
	if n then framechanged(n) end
end

local dialogposes = iup.dialog {
	iup.vbox {
		iup.tabs {
			iup.vbox {
				poselist,
				iup.hbox { 
					loadposebutton,
					saveposebutton,
					deleteposebutton,
				},
				tabtitle = "Poses"
			},
			iup.vbox {
				scenelist,
				iup.hbox { 
					loadscenebutton,
					savescenebutton,
					deletescenebutton,
				},
				tabtitle = "Scenes"
			},
		},
		iup.frame {
			title = "Animation",
			iup.hbox { 
				iup.label { title = "Clip" },
				cliptext,
				iup.label { title = "Frame" },
				frametext,
				gap = 3,
				alignment = "acenter"
			},
		},
	},
	nmargin = "3x3",
	maxbox = "no",
	minbox = "no",
}

dialogposes.loadpose = loadpose
dialogposes.savepose = savepose
dialogposes.loadscene = loadscene
dialogposes.savescene = savescene

local currentcharacter

local function setclip(clip)
	log.spam("setclip")
	log.spam("dialogposes set clip %d on %s", clip, currentcharacter or "no character")
	if currentcharacter then
		local skel = currentcharacter.m_xxSkeleton
		skel.m_poseNumber = clip
		skel.m_animFrame = 9000
	end
end
clipchanged.connect(setclip)

function dialogposes.characterchanged(character)
	log.spam("characterchanged")
	log.spam("dialogposes characterchanged %s", character or "no character")
	currentcharacter = character.struct
end

return dialogposes
