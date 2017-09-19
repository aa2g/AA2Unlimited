require "iuplua"
require "iupluacontrols"

local _M = {}

local signals = require "poser.signals"
local json = require "json"
local lists = require "poser.lists"
local charamgr = require "poser.charamgr"
local toggles = require "poser.toggles"

local clipchanged= signals.signal()
local framechanged= signals.signal()

local posesdir = "poser\\poses"
local scenesdir = "poser\\scenes"

local function setclip(clip)
	if charamgr.current then
		charamgr.current.setclip(clip)
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
local resetposebutton = iup.button { title = "Reset Pose", expand = "horizontal" }

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

function resetposebutton.action()
	if charamgr.current then
		for _,v in charamgr.current.sliders() do
			v:Reset()
			v:Apply()
		end
	end
end

function deleteposebutton.action()
	local resp = iup.Alarm("Confirmation", "Are you sure you want to delete the selected pose?", "Yes", "No")
	if resp == 1 then
		local path = aau_path(posesdir, poselist.value .. ".pose")
		local ret, msg = os.remove(path)
		if not ret then
			log.error(msg)
		end
		log.spam("Removed %s", path)
		populateposelist()
	end
end


local function readfile(path)
    local file = io.open(path, "rb")
    if not file then return nil end
    local data = file:read "*a"
    file:close()
    return data
end

local function autopose(fname)
	if _M.opts.autoloading == 1 then
		local ctx = charamgr.current:context_name()
		log.spam("Autopose: saving autopose %s %s",ctx,fname)
		_M.cfg.autoload[ctx] = fname
		Config.save()
	end
end

local cliptext
local function loadpose(filename)
	assert(filename ~= "")
	log.spam("Poser: Loading pose %s", filename)
	local character = charamgr.current
	if character and character.ischaracter == true then
		local path = aau_path(posesdir, filename) .. ".pose"
		log.spam("Poser: Reading %s", path)
		local data = readfile(path)
		if data then
			local ok, ret = pcall(json.decode, data)
			if not ok then
				error("Error decoding pose %s data:" % filename)
			end
			local jp = ret
			if jp then
				local version = jp._VERSION_ or 1
				local clip = jp.pose
				if clip then
					log.spam("setting clip from pose to %d", clip)
					setclip(clip)
					cliptext.value = clip
				end
				local frame = jp.frame
				if jp.sliders then
					log.spam("Setting sliders")
					for k,v in pairs(jp.sliders) do
						local slider = character.getslider(k)
						if slider then
							if version == 1 then
								slider:SetValues(v[1], v[2], v[3])
							else
								slider:rotation(0,v[1])
								slider:rotation(1,v[2])
								slider:rotation(2,v[3])
								slider:rotation(3,v[4])
							end
							slider:translate(0,v[3 + version])
							slider:translate(1,v[4 + version])
							slider:translate(2,v[5 + version])
							slider:scale(0,v[6 + version])
							slider:scale(1,v[7 + version])
							slider:scale(2,v[8 + version])
							slider:Apply()
						end
					end
				end
				local face = jp.face
				if face then
					if face.mouth then character.mouth = face.mouth end
					if face.mouthopen then character.mouthopen = face.mouthopen end
					if face.eye then character.eye = face.eye end
					if face.eyeopen then character.eyeopen = face.eyeopen end
					if face.eyebrow then character.eyebrow = face.eyebrow end
					if face.blush then character.blush = face.blush / 9 end
					if face.blushlines then character.blushlines = face.blushlines / 9 end
				end
			end
		end
	end
end

function loadposebutton.action()
	local fn = poselist.value
	local ok, ret = pcall(loadpose, fn)
	if not ok then
		log.error("Error loading pose %s:", poselist.value)
		log.error(ret)
	else
		-- at this point pose number is rewritten to whatever is in pose file
		autopose(fn)
	end
end

local function savepose(filename)
	autopose(filename)
	log.spam("Poser: Saving pose %s", filename)
	local character = charamgr.current
	if character and character.ischaracter == true then
		local path = aau_path(posesdir, filename) .. ".pose"
		log.spam("Poser: Saving to %s", path)
		local t = {}
		t._VERSION_ = 2
		t.pose = character.pose
		t.frame = character.frame
		local sliders = {}
		for k,v in character.sliders() do
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
end

function saveposebutton.action()
	savepose(poselist.value)
end

cliptext = iup.text { spin = "yes", spinvalue = 0, spinmin = 0, spinmax = 9999, visiblecolumns = 2, expand = "horizontal" }
function cliptext.valuechanged_cb(self)
	log.spam("clip text changed to %s", self.value)
	local n = tonumber(self.value)
	if n then clipchanged(n) end
end

local frametext = iup.text { spin = "yes", spinvalue = 0, spinmin = 0, spinmax = 9999, visiblecolumns = 2, expand = "horizontal" }
function frametext.valuechanged_cb(self)
	log.spam("frame text changed")
	local n = tonumber(self.value)
	if n then framechanged(n) end
end

charamgr.on_character_updated.connect(function(chr)
	if chr ~= charamgr.current then
		log.warn("updating non-current character")
		return
	end
	local p = chr.struct.m_xxSkeleton.m_poseNumber
	cliptext.value = p
	if (_M.opts.autoloading == 1) and (not chr.first_update) then
		chr.first_update = true
		local ctname = chr:context_name()
		local auto = _M.cfg.autoload[ctname]
		if auto then
			log.spam("Autoloading pose %s",auto)
			if pcall(loadpose,auto) then
				-- focus the autoloaded pose in the list
				poselist.select(auto)
			else
				log.warn("Poser: Autoload for %s failed", ctname)
			end
		else
			log.warn("Poser: Autoload for %s not found", ctname)
		end
	end
end)

_M.dialogposes = iup.dialog {
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
		resetposebutton,
	},
	nmargin = "3x3",
	maxbox = "no",
	minbox = "no",
}

return _M
