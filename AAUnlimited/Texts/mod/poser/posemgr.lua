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
local posemap = signals.signal()

local posesdir = "poser\\poses"
local scenesdir = "poser\\scenes"

local function setclip(clip)
	if charamgr.current then
		charamgr.current:setclip(clip)
	end
end
clipchanged.connect(setclip)

local function savedposes()
	return readdir(aau_path(posesdir .. "\\*.pose"))
end

local function savedscenes()
	return readdir(aau_path(scenesdir .. "\\*.scene"))
end


local cliptext
local poselist = lists.listbox { editbox = "yes" }
local scenelist = lists.listbox { editbox = "yes" }
local loadposebutton = iup.button { title = "Load", expand = "horizontal" }
local saveposebutton = iup.button { title = "Save", expand = "horizontal" }
local mapposebutton = iup.button { title = "Map", expand = "horizontal" }
local loadscenebutton = iup.button { title = "Load", expand = "horizontal" }
local savescenebutton = iup.button { title = "Save", expand = "horizontal" }
local deleteposebutton = iup.button { title = "Delete" }
local deletescenebutton = iup.button { title = "Delete" }
local resetposebutton = iup.button { title = "Reset Pose", expand = "horizontal" }
local unmapposebutton = iup.button { title = "Unmap pose", expand = "horizontal" }


local function populateposelist()
	local i = 1
	for f in savedposes() do
		f = f:match("^(.*)%.pose$")
		if f then
			poselist[i] = f
			i = i + 1
		end
	end
	poselist[i] = nil
end
populateposelist()

local function populatescenelist()
	local i = 1
	for f in savedscenes() do
		f = f:match("^(.*)%.scene$")
		if f then
			scenelist[i] = f
			i = i + 1
		end
	end
	scenelist[i] = nil
end
populatescenelist()

function unmapposebutton.action()
	local chr = charamgr.current 
	if chr then
		_M.cfg.autoload[chr:context_name()] = nil
		set_class_key(chr:context_name(), nil)
	end
end

function resetposebutton.action()
	if charamgr.current then
		-- TODO: make this sane
		for _,v in charamgr.current:sliders() do
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
	local chr = charamgr.current
	local ctx = chr:context_name()
	log.info("Autopose: saving autopose %s %s",ctx,fname)
	set_class_key(ctx, fname)
	_M.cfg.autoload[ctx] = fname
	Config.save()
end

local function table2pose(pose, character)
	local version = pose._VERSION_ or 1
	local clip = pose.pose
	if clip then
		character:setclip(clip)
		cliptext.value = clip
	end
	local frame = pose.frame
	if pose.sliders then
		log.spam("Setting sliders")
		for k,v in pairs(pose.sliders) do
			local slider = character:getslider(k)
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
	local face = pose.face
	if face then
		if face.mouth then character.mouth = face.mouth end
		if face.mouthopen then character.mouthopen = face.mouthopen end
		if face.eye then character.eye = face.eye end
		if face.eyeopen then character.eyeopen = face.eyeopen end
		if face.eyebrow then character.eyebrow = face.eyebrow end

		local facestruct = charamgr.current.struct.m_xxFace
		local material = facestruct:FindMaterial("A00_M_hoho") or facestruct:FindMaterial("S00_M_hoho") 
		if material then
			material:m_lightingAttributes(3, face.blush / 9)
		end
		material = facestruct:FindMaterial("A00_M_hohosen") or facestruct:FindMaterial("S00_M_hohosen") 
		if material then
			material:m_lightingAttributes(3, face.blush / 9)
		end
	end
end

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
			if ret then
				table2pose(ret, character)
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
	end
end

local function pose2table(character)
	if character and character.ischaracter == true then
		local t = {}
		t._VERSION_ = 2
		t.pose = character.pose
		t.frame = character.frame
		local sliders = {}
		for k,v in character:sliders() do
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
		}

		local facestruct = charamgr.current.struct.m_xxFace
		local material = facestruct:FindMaterial("A00_M_hoho") or facestruct:FindMaterial("S00_M_hoho")
		if material then
			face.blush = material:m_lightingAttributes(3) * 9
		end
		material = facestruct:FindMaterial("A00_M_hohosen") or facestruct:FindMaterial("S00_M_hohosen")
		if material then
			face.blushlines = material:m_lightingAttributes(3) * 9
		end

		t.face = face
		return t
	end
end

local function savepose(filename)
	local path = aau_path(posesdir, filename) .. ".pose"
	log.spam("Poser: Saving pose %s to %s", filename, path)
	local character = charamgr.current
	local pose = pose2table(character)
	if pose then
		local file = io.open(path, "w")
		if not file then return nil end
		file:write(json.encode(pose))
		file:close()
		log.spam("Poser: Pose %s saved", filename)
		populateposelist()
	end
end

function saveposebutton.action()
	savepose(poselist.value)
end

-- == Scenes ==

local function loadscene(filename)
	assert(filename ~= "")
	local path = aau_path(scenesdir, filename) .. ".scene"
	log.spam("Poser: Loading scene %s", path)
	local data = readfile(path)
	if data then
		local ok, scene = pcall(json.decode, data)
		if not ok then
			error("Error decoding scene %s data:" % filename)
		end
		if scene then
			for i,loadchara in ipairs(scene.characters or {}) do
				local chara = charamgr.characters[i]
				if chara then
					log.spam("table2pose")
					table2pose(loadchara.pose, chara)
				end
			end
		end
	end
end

function loadscenebutton.action()
	local name = scenelist.value
	local ok, ret = pcall(loadscene, name)
	if not ok then
		log.error("Error loading pose %s:", name)
		log.error(ret)
	end
end

local function savescene(filename)
	autopose(filename)
	local path = aau_path(scenesdir, filename) .. ".scene"
	log.spam("Poser: Saving scene %s to %s", filename, path)
	local scene = {
		VERSION = 1,
		characters = {},
	}
	local characters = scene.characters
	
	for _,chara in ipairs(charamgr.characters) do
		local character = {
			pose = pose2table(chara)
		}
		table.insert(characters, character)
	end
	
	local file = io.open(path, "w")
	if not file then return nil end
	file:write(json.encode(scene))
	file:close()
	log.spam("Poser: Scene %s saved", filename)
	populatescenelist()
end

function savescenebutton.action()
	savescene(scenelist.value)
end


-- == Pose Mapping ==

function mapposebutton.action()
	autopose(poselist.value)
	savepose(poselist.value)
end

cliptext = iup.text { spin = "yes", spinvalue = 0, spinmin = 0, spinmax = 9999, visiblecolumns = 2, expand = "horizontal" }
local maptext = iup.text { spin = "yes", spinvalue = 0, spinmin = 0, spinmax = 9999, visiblecolumns = 2, expand = "horizontal" }
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
	maptext.value = chr.startpose
	local p = chr.struct.m_xxSkeleton.m_poseNumber
	cliptext.value = p
	if (_M.opts.autoloading == 1) and (not chr.first_update) then
		chr.first_update = true
		local ctname = chr:context_name()
		local auto = get_class_key(ctname) or _M.cfg.autoload[ctname]
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
--				iup.label { title = "Map" },
--				maptext,
				iup.label { title = "Frame" },
				frametext,
				gap = 3,
				alignment = "acenter"
			},
		},
		resetposebutton,
--		mapposebutton,
--		unmapposebutton,
	},
	nmargin = "3x3",
	maxbox = "no",
	minbox = "no",
}

return _M
