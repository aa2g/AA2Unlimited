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
local os = require "os"

local clipchanged= signals.signal()
local framechanged= signals.signal()
local poseloaded = signals.signal()
local sceneloaded = signals.signal()

_M.poseloaded = poseloaded
_M.sceneloaded = sceneloaded

local create_thumbnail_function
local posesdir = aau_path("poser\\poses")
local scenesdir = aau_path("poser\\scenes")
local embed_file = nil
local embed_magic = nil
local embed_save_path = nil
local png_magic_pose = "POSE\x00\x00\x00\x00"
local png_magic_scene = "SCENE\x00\x00\x00"
local save_restore_ui = false

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

local cliptext
local posename = iup.text { expand = "horizontal", visiblecolumns = 20 }
local scenename = iup.text { expand = "horizontal", visiblecolumns = 20 }
local posefilter = lists.listfilter()
local poselist = lists.listbox { expand = "yes", chars = 20, sort = "yes" }
local scenefilter = lists.listfilter()
local scenelist = lists.listbox { expand = "yes", chars = 20, sort = "yes" }
local loadposebutton = iup.button { title = "Load", expand = "horizontal" }
local saveposebutton = iup.button { title = "Save", expand = "horizontal" }
local saveposetexttoggle = iup.toggle { title = "Save as .pose", value = "OFF" }
local loadscenebutton = iup.button { title = "Load", expand = "horizontal" }
local savescenebutton = iup.button { title = "Save", expand = "horizontal" }
local savescenetexttoggle = iup.toggle { title = "Save as .scene", value = "OFF" }
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

local function findposerfiles(directory, match)
	local newlist = {}
	local i = 1
	local last
	local pattern = directory .. "\\*.*"
	for f in readdir(pattern) do
		f = f:match(match) or f:match("^(.*)%.png$")
		if f ~= last then
			if f then
				newlist[i] = f
				i = i + 1
				last = f
			end
		end
	end
	return newlist
end

local function populateposelist()
	poselist.setlist(findposerfiles(posesdir, "^(.*)%.pose$"))
end

local function populatescenelist()
	scenelist.setlist(findposerfiles(scenesdir, "^(.*)%.scene$"))
end

function on.launch()
	if exe_type == "edit" then
		create_thumbnail_function = 0x36C6C1
	else
		create_thumbnail_function = 0x38F6C9
	end
	populateposelist()
	populatescenelist()
end

function on.poser_saved_thumbnail()
	log.spam("Saving poser thumbnail")
	local screenshot = play_path("poser-screenshot.png")
	os.remove(embed_save_path)
	os.rename(screenshot, embed_save_path)
	local file = io.open(embed_save_path, "ab")
	file:write(embed_file)
	file:write(string.pack("<I4", #embed_file))
	file:write(embed_magic)
	file:close()
	embed_save_path = nil
	embed_file = nil
	embed_magic = nil
	SetHideUI(save_restore_ui)
end

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
		os.remove(posesdir  .. "\\" .. posename.value .. ".pose")
		os.remove(posesdir  .. "\\" .. posename.value .. ".png")
		poselist.valuestring = posename.value
		if poselist.valuestring == posename.value then
			poselist.removeitem = poselist.value
		end
	end
end

local function readpng(path)
	local file = io.open(path, "rb")
	if not file then return nil end
	file:seek("end", -12)
	local size = string.unpack("<I4", file:read(4))
	local data = file:read()
	if data == png_magic_pose or data == png_magic_scene then
		file:seek("end", (size + 12) * -1)
		data = file:read(size)
	else
		data = nil
	end
	file:close()
	return data
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
		log.spam("Poser: Reading pose %s", filename)
		local data = readpng(posesdir .. "\\" .. filename .. ".png")
		if data == nil then
			data = readfile(posesdir .. "\\" .. filename .. ".pose")
		end
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
	-- log.spam("Poser: Saving pose %s to %s", filename, path)
	local character = charamgr.current
	local pose = pose2table(character)
	if pose then
		local content = json.encode(pose)
		if saveposetexttoggle.value == "ON" then
			local file = io.open(path, "w")
			if file then
				file:write(content)
				file:close()
			end
		end
		-- log.spam("Poser: Pose %s saved", filename)
		local currentvalue = poselist.value
		if posename.value ~= poselist.valuestring then
			poselist.valuestring = filename
			if poselist.value == currentvalue then
				poselist.appenditem = filename
				poselist.valuestring = filename
			end
		end

		embed_file = content
		embed_magic = png_magic_pose
		embed_save_path = posesdir .. "\\" .. filename .. ".png"
		save_restore_ui = SetHideUI(true)
		g_poke(create_thumbnail_function, '\x0F')
	end
end

function saveposebutton.action()
	savepose(posename.value)
end


-- == Scenes ==

local function loadscene(filename)
	log.spam("Poser: Loading scene %s", filename)
	local data = readpng(scenesdir .. "\\" .. filename .. ".png")
	if data == nil then
		data = readfile(scenesdir .. "\\" .. filename .. ".scene")
	end
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
					else
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
	-- log.spam("Poser: Saving scene %s to %s", filename, path)

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
	
	local content = json.encode(scene)
	if savescenetexttoggle.value == "ON" then
		local file = io.open(path, "w")
		if file then
			file:write(content)
			file:close()
		end
	end

	embed_file = content
	embed_magic = png_magic_scene
	embed_save_path = scenesdir .. "\\" .. filename .. ".png"
	save_restore_ui = SetHideUI(true)
	g_poke(create_thumbnail_function, '\x1F')

	log.spam("Poser: Scene %s saved", filename)
	local currentvalue = scenelist.value
	if scenename.value ~= scenelist.valuestring then
		scenelist.valuestring = filename
		if scenelist.value == currentvalue then
			scenelist.appenditem = filename
			scenelist.valuestring = filename
		end
	end
end

function savescenebutton.action()
	savescene(scenename.value)
end

function deletescenebutton.action()
	local resp = iup.Alarm("Confirmation", "Are you sure you want to delete this scene?", "Yes", "No")
	if resp == 1 then
		os.remove(scenesdir .. "\\" .. scenename.value .. ".scene")
		os.remove(scenesdir .. "\\" .. scenename.value .. ".png")
		scenelist.valuestring = scenename.value
		if scenelist.valuestring == scenename.value then
			scenelist.removeitem = scenelist.value
		end
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
					saveposetexttoggle,
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
					savescenetexttoggle,
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
			tabchangepos_cb = function(self, newpos, oldpos)
				lock_world = newpos == 0 and lockworldtoggle.value == "ON"
			end,
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
