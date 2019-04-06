require "iuplua"
require "iupluacontrols"

local _M = {}

local fileutils = require "poser.fileutils"
local signals = require "poser.signals"
local json = require "json"
local lists = require "poser.lists"
local camera = require "poser.camera"
local charamgr = require "poser.charamgr"
local propmgr = require "poser.propmgr"
local toggles = require "poser.toggles"

local clipchanged= signals.signal()
local framechanged= signals.signal()
local poseloaded = signals.signal()
local sceneloaded = signals.signal()

_M.poseloaded = poseloaded
_M.sceneloaded = sceneloaded

local posesdir = aau_path("poser\\poses")
local scenesdir = aau_path("poser\\scenes")

local lock_camera = false
local lock_light = true
local lock_face = false
local lock_props = false
local lock_world = false
local lock_world_bone = "a01_N_Zentai_010"

local lockfacetoggle2
local lockworldtoggle = iup.toggle { title = "Lock World Bone", action = function(self, state) lock_world = state == 1 end }
local lockfacetoggle1 = iup.toggle { title = "Lock Face", action = function(self, state) lock_face = state == 1; lockfacetoggle2.value = (state == 1 and "ON") or "OFF" end }
lockfacetoggle2 = iup.toggle { title = "Lock Face", action = function(self, state) lock_face = state == 1; lockfacetoggle1.value = (state == 1 and "ON") or "OFF" end }
local lockpropstoggle = iup.toggle { title = "Lock Props", action = function(self, state) lock_props = state == 1 end }
local lockcameratoggle= iup.toggle { title = "Lock Camera", action = function(self, state) lock_camera = state == 1 end }
local locklighttoggle = iup.toggle { title = "Lock Light", action = function(self, state) lock_light = state == 1 end, value = "ON" }

local function setclip(clip)
	if charamgr.current then
		charamgr.current:setclip(clip)
	end
end
clipchanged.connect(setclip)

local function savedposes()
	return readdir(posesdir .. "\\*.pose")
end

local function savedscenes()
	return readdir(scenesdir .. "\\*.scene")
end


local cliptext
local posename = iup.text { expand = "horizontal", visiblecolumns = 20 }
local scenename = iup.text { expand = "horizontal", visiblecolumns = 20 }
local posefilter = lists.listfilter()
local poselist = lists.listbox { expand = "yes", chars = 20 }
local scenefilter = lists.listfilter()
local scenelist = lists.listbox { expand = "yes", chars = 20 }
local loadposebutton = iup.button { title = "Load", expand = "horizontal" }
local saveposebutton = iup.button { title = "Save", expand = "horizontal" }
local loadscenebutton = iup.button { title = "Load", expand = "horizontal" }
local savescenebutton = iup.button { title = "Save", expand = "horizontal" }
local deleteposebutton = iup.button { title = "Delete" }
local deletescenebutton = iup.button { title = "Delete" }
local refreshposelistbutton = iup.button { title = "Refresh" }
local refreshscenelistbutton = iup.button { title = "Refresh" }
local useposesfolderbutton = iup.button { title = "Use Folder" }
local usescenesfolderbutton = iup.button { title = "Use Folder" }

signals.connect(poselist, "selectionchanged", function() posename.value = poselist[poselist.value] end )
signals.connect(scenelist, "selectionchanged", function() scenename.value = scenelist[scenelist.value] end )
signals.connect(posefilter, "setfilter", poselist, "setfilter")
signals.connect(scenefilter, "setfilter", scenelist, "setfilter")

local function populateposelist()
	local newlist = {}
	local i = 1
	for f in savedposes() do
		f = f:match("^(.*)%.pose$")
		if f then
			newlist[i] = f
			i = i + 1
		end
	end
	poselist.setlist(newlist)
end
populateposelist()

local function populatescenelist()
	local newlist = {}
	local i = 1
	for f in savedscenes() do
		f = f:match("^(.*)%.scene$")
		if f then
			newlist[i] = f
			i = i + 1
		end
	end
	scenelist.setlist(newlist)
end
populatescenelist()

function useposesfolderbutton.action()
	posesdir = fileutils.getfolderdialog(posesdir)
	populateposelist()
end

function usescenesfolderbutton.action()
	scenesdir = fileutils.getfolderdialog(scenesdir)
	populatescenelist()
end

function deleteposebutton.action()
	local resp = iup.Alarm("Confirmation", "Are you sure you want to delete this pose?", "Yes", "No")
	if resp == 1 then
		local path = posesdir  .. "\\" .. posename.value .. ".pose"
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

local function table2pose(pose, character)
	local version = pose._VERSION_ or 1
	local clip = pose.pose
	if clip then
		character:setclip(clip)
		cliptext.value = clip
	end
	local frame = pose.frame
	if pose.sliders then
		for k,v in pairs(pose.sliders) do
			local slider = not (k == lock_world_bone and lock_world) and character:getslider(k)
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
	if face and not lock_face then
		if face.mouth then character.mouth = face.mouth end
		if face.mouthopen then character.mouthopen = face.mouthopen end
		if face.eye then character.eye = face.eye end
		if face.eyeopen then character.eyeopen = face.eyeopen end
		if face.eyebrow then character.eyebrow = face.eyebrow end

		local facestruct = character.struct.m_xxFace
		local material = facestruct:FindMaterial("A00_M_hoho") or facestruct:FindMaterial("S00_M_hoho") 
		if material then
			material:m_lightingAttributes(3, face.blush / 9)
		end
		material = facestruct:FindMaterial("A00_M_hohosen") or facestruct:FindMaterial("S00_M_hohosen") 
		if material then
			material:m_lightingAttributes(3, face.blushlines / 9)
		end
	end
end

local function loadpose(character, filename)
	assert(filename ~= "")
	log.spam("Poser: Loading pose %s", filename)
	if character and character.ischaracter == true then
		local path = posesdir .. "\\" .. filename .. ".pose"
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
	poseloaded(character)
end

function loadposebutton.action()
	local fn = posename.value
	local ok, ret = pcall(loadpose, charamgr.current, fn)
	if not ok then
		log.error("Error loading pose %s:", posename.value)
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
		local function tolist(v)
		  return { v:rotation(0), v:rotation(1), v:rotation(2),	v:rotation(3),
				v:translate(0), v:translate(1), v:translate(2),
				v:scale(0), v:scale(1), v:scale(2) }
		end
		for k,v in character:sliders() do
			sliders[k] = tolist(v)
		end
		for k,v in character:props() do
			sliders[k] = tolist(v)
		end
		t.sliders = sliders
		
		local face = {
			eye = character.eye,
			eyeopen = character.eyeopen,
			eyebrow = character.eyebrow,
			mouth = character.mouth,
			mouthopen = character.mouthopen,
		}

		local facestruct = character.struct.m_xxFace
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
	if filename == "" then return end
	local path = posesdir .. "\\" .. filename .. ".pose"
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
	savepose(posename.value)
end


-- == Scenes ==

local function loadscene(filename)
	local path = scenesdir .. "\\" .. filename .. ".scene"
	log.spam("Poser: Loading scene %s", path)
	local data = readfile(path)
	if data then
		local ok, scene = pcall(json.decode, data)
		if not ok then
			error("Error decoding scene %s data:" % filename)
		end
		if scene then
			local characters = scene.characters or {}
			local props = scene.props or {}
			
			local loadedprops = {}
			for i,v in pairs(propmgr.props) do
				loadedprops[i] = v
			end
			
			for i,readchara in ipairs(characters) do
				local chara = charamgr.characters[i]
				if chara then
					table2pose(readchara.pose, chara)
				end
			end
			
			local not_found = {}

			if not lock_props then
				for i,readprop in ipairs(props) do
					local find
					for j,p in pairs(loadedprops) do
						if p.name == readprop.name then
							find = j
							break
						end
					end
					if not find then
						table.insert(not_found, readprop.name)
					end
					find = table.remove(loadedprops, find)
					if find then
						for k,v in pairs(readprop.sliders) do
							local slider = find:getslider(k)
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
				end
				if #not_found > 0 then
					log.error("Load Scene: Props Not Found: %s", table.concat(not_found, ", "))
				end
			end
			
			if not lock_camera then
				for k,v in pairs(scene.camera or {}) do
					camera[k] = v
				end
			end

			if not lock_light and scene.light then
				local direction = scene.light.direction
				for _,character in pairs(charamgr.characters) do
					local skeleton = character.struct.m_xxSkeleton
					local lightcount = skeleton.m_lightsCount
					for i = 1, lightcount, 1 do
						local light = skeleton:m_lightsArray(i - 1)
						light:SetLightDirection(direction[1], direction[2], direction[3], direction[4])
					end
				end
			end
		end
	end
	sceneloaded()
end

function loadscenebutton.action()
	local name = scenename.value
	local ok, ret = pcall(loadscene, name)
	if not ok then
		log.error("Error loading pose %s:", name)
		log.error(ret)
	end
end

local function savescene(filename)
	local path = scenesdir .. "\\" .. filename .. ".scene"
	log.spam("Poser: Saving scene %s to %s", filename, path)

	local characters = {}
	local props = {}
	local light

	local scene = {
		VERSION = 1,
		characters = characters,
		props = props,
	}
	
	for _,chara in ipairs(charamgr.characters) do
		local character = {
			pose = pose2table(chara)
		}
		table.insert(characters, character)
		if not light then
			light = {}
			local struct = chara.struct.m_xxSkeleton:m_lightsArray(0)
			local x,y,z,w = struct:GetLightDirection()
			light.direction = { x, y, z, w }
			scene.light = light
		end
	end
	
	for _,prop in ipairs(propmgr.props) do
		local sliders = {}
		for k,v in prop:sliders() do
			sliders[k] = {
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
		end
		local prop = {
			name = prop.name,
			sliders = sliders,
		}
		table.insert(props, prop)
	end
	
	local c = {}
	for _,k in ipairs(camera.keys) do
		c[k] = camera[k]
	end
	scene.camera = c
	
	local file = io.open(path, "w")
	if not file then return nil end
	file:write(json.encode(scene))
	file:close()
	log.spam("Poser: Scene %s saved", filename)
	populatescenelist()
end

function savescenebutton.action()
	savescene(scenename.value)
end

function deletescenebutton.action()
	local resp = iup.Alarm("Confirmation", "Are you sure you want to delete this scene?", "Yes", "No")
	if resp == 1 then
		local path = scenesdir .. "\\" .. scenename.value .. ".scene"
		local ret, msg = os.remove(path)
		if not ret then
			log.error(msg)
		end
		log.spam("Removed %s", path)
		populatescenelist()
	end
end

function refreshposelistbutton.action()
	populateposelist()
end

function refreshscenelistbutton.action()
	populatescenelist()
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
		return
	end
	local p = chr.struct.m_xxSkeleton.m_poseNumber
	cliptext.value = p
end)

_M.dialogposes = iup.dialog {
	iup.vbox {
		iup.tabs {
			iup.hbox {
				iup.vbox {
					posefilter,
					poselist,
					iup.hbox {
						refreshposelistbutton,
						iup.label { title = "Animation Clip" },
						cliptext,
						gap = 3,
						alignment = "acenter"
					},
					expand = "no",
				},
				iup.vbox {
					iup.label { title = "Pose:" },
					posename,
					iup.hbox {
						loadposebutton,
						iup.fill { size = 10, },
						saveposebutton,
					},
					iup.label { title = "Locks" },
					lockworldtoggle,
					lockfacetoggle1,
					useposesfolderbutton,
					iup.label { title = "Delete pose:" },
					deleteposebutton,
				},
				tabtitle = "Poses",
				gap = 3,
			},
			iup.hbox {
				iup.vbox {
					scenefilter,
					scenelist,
					refreshscenelistbutton,
					expand = "no",
				},
				iup.vbox {
					iup.label { title = "Scene:" },
					scenename,
					iup.hbox {
						loadscenebutton,
						iup.fill { size = 10, },
						savescenebutton,
					},
					iup.label { title = "Locks" },
					lockfacetoggle2,
					lockpropstoggle,
					lockcameratoggle,
					locklighttoggle,
					usescenesfolderbutton,
					iup.label { title = "Delete scene:" },
					deletescenebutton,
				},
				tabtitle = "Scenes",
				gap = 3,
			},
		},
		iup.hbox {
			iup.button { title = "Show UI", action = function() SetHideUI(false) end },
			iup.button { title = "Hide UI", action = function() SetHideUI(true) end },
		},
	},
	nmargin = "3x3",
	maxbox = "no",
	minbox = "no",
}

_M.loadpose = loadpose

function _M.resetpose(character)
	for _,v in character:sliders() do
		v:Reset()
		v:Apply()
	end
end

function _M.tpose(character)
	_M.resetpose(character)
	character.pose = 0
end

return _M
