local _M = {}

local signals = require "poser.signals"
local json = require "json"
local lists = require "poser.lists"
local charamgr = require "poser.charamgr"

local clipchanged= signals.signal()
local framechanged= signals.signal()

local posesdir = "poser/poses"
local scenesdir = "poser/scenes"

local function setclip(clip)
	if charamgr.current then
		charamgr.current:SetClip(clip)
	end
end
clipchanged.connect(setclip)

local function savedposes()
	return readdir(aau_path(posesdir .. "\\*.pose"))
end

local function savedscenes()
	return readdir(aau_path(scenesdir .. "\\*.scene"))
end

local poselist = lists.listbox { editbox = "yes" }
local scenelist = lists.listbox { editbox = "yes" }
local loadposebutton = iup.button { title = "Load", expand = "horizontal" }
local saveposebutton = iup.button { title = "Save", expand = "horizontal" }
local loadscenebutton = iup.button { title = "Load", expand = "horizontal" }
local savescenebutton = iup.button { title = "Save", expand = "horizontal" }
local deleteposebutton = iup.button { title = "Delete" }
local deletescenebutton = iup.button { title = "Delete" }

local function populateposelist()
	local i = 1
	poselist[i] = nil
	for f in savedposes() do
		f = f:match("^(.*)%.pose$")
		if f then
			poselist[i] = f
			i = i + 1
		end
	end
end
populateposelist()

local function readfile(path)
    local file = io.open(path, "rb")
    if not file then return nil end
    local data = file:read "*a"
    file:close()
    return data
end

local function loadpose(filename)
	log.spam("Poser: Loading pose %s", filename)
	local character = charamgr.current
	if not character then return end
	local path = aau_path(posesdir, filename) .. ".pose"
	log.spam("Poser: Reading %s", path)
	local data = readfile(path)
	if data then
		local ok, ret = pcall(json.decode, data)
		if not ok then
			log.error("Error decoding pose %s data:", filename)
			log.error(ret)
			return
		end
		local jp = ret
		if jp then
			if not jp._VERSION_ or jp._VERSION_ ~= 2 then
				log.error("Poser: %s isn't a valid pose file", filename)
				return
			end
			local clip = jp.pose
			if clip then setclip(clip) end
			local frame = jp.frame
			if jp.sliders then
				log.spam("Setting sliders")
				for k,v in pairs(jp.sliders) do
					local slider = character:GetSlider(k)
					if slider then
						slider:rotation(0,v[1])
						slider:rotation(1,v[2])
						slider:rotation(2,v[3])
						slider:rotation(3,v[4])
						slider:translate(0,v[5])
						slider:translate(1,v[6])
						slider:translate(2,v[7])
						slider:scale(0,v[8])
						slider:scale(1,v[9])
						slider:scale(2,v[10])
						slider:Apply()
					end
				end
			end
			local face = jp.face
			if face then
				local xxface = character:GetXXFileFace()
				if face.mouth then character.mouth = face.mouth end
				if face.mouthopen then character.mouthopen = face.mouthopen end
				if face.eye then character.eye = face.eye end
				if face.eyeopen then character.eyeopen = face.eyeopen end
				if face.eyebrow then
					local base = character.eyebrow
					base = base - (base % 7)
					character.eyebrow = base + (face.eyebrow % 7)
				end
				if face.blush then character.blush = face.blush / 9 end
				if face.blushlines then character.blushlines = face.blushlines / 9 end
			end
		end
	end
end

function loadposebutton.action()
	loadpose(poselist.value)
end

local function savepose(filename)
	log.spam("Poser: Saving pose %s", filename)
	local character = charamgr.current
	if not character then return end
	local path = aau_path(posesdir, filename) .. ".pose"
	log.spam("Poser: Saving to %s", path)
	local t = {}
	t._VERSION_ = 2
	t.pose = character.pose
	t.frame = character.frame
	local sliders = {}
	for k,v in character:Sliders() do
		local slider = {
			v:rotation(0),
			v:rotation(1),
			v:rotation(2),
			v:rotation(3),
			v:translate(0),
			v:translate(1),
			v:translate(2),
			v:scale(0),
			v:scale(1),
			v:scale(2),
		}
		sliders[k] = slider
	end
	t.sliders = sliders
	
	local face = {
		eye = character.eye,
		eyeopen = character.eyeopen,
		eyebrow = character.eyebrow,
		mouth = character.mouth,
		mouthopen = character.mouthopen,
		blush = character.blush * 9,
		blushlines = character.blushlines * 9,
	}
	t.face = face
	
	local file = io.open(path, "w")
    if not file then return nil end
    file:write(json.encode(t))
    file:close()
	log.spam("Poser: Pose %s saved", filename)
	populateposelist()
end

function saveposebutton.action()
	savepose(poselist.value)
end

local cliptext = iup.text { spin = "yes", spinvalue = 0, spinmin = 0, spinmax = 9999, visiblecolumns = 2, expand = "horizontal" }
function cliptext.valuechanged_cb(self)
	log.spam("clip text changed")
	local n = tonumber(self.value)
	if n then clipchanged(n) end
end

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

return dialogposes
