require "iuplua"
require "iupluacontrols"
require "iupluaim"

local _M = {}
-- _M.dialogposes = {}

local  playbacksymbols  = 
{	
	-- first = "⏮",
	-- prev = "⏪",
	-- playpause = "⏯",
	-- next = "⏩",
	-- last = "⏭",
	
	-- first = "|<<",
	-- prev = "|<",
	-- playpause = ">||",
	-- next = ">|",
	-- last = ">>|",
	
	first = "First",	
	prev = "Prev",
	playpause = "Play/Pause",
	next = "Next",
	last = "Last",
}

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
local albumsdir = aau_path("poser\\albums")
local embed_file = nil
local embed_magic = nil
local embed_save_path = nil
local png_magic_pose = "POSE\x00\x00\x00\x00"
local png_magic_scene = "SCENE\x00\x00\x00"
local png_magic_album = "ALBUM\x00\x00\x00"
local save_restore_ui = false

local timer1 = iup.timer{time=1000}
local timer2 = iup.timer{time=1000}

local lock_camera = false
local lock_light = true
local lock_face = false
local lock_props = false
local lock_world = false
local auto_load = false
local auto_play = false -- loop
-- local auto_load_props = false -- loop
local lock_world_bone = "a01_N_Zentai_010"

local lockworldtoggle = iup.toggle { title = "Lock World Bone", action = function(self, state) lock_world = state == 1 end }
local lockfacetoggle3
local lockfacetoggle2
local lockfacetoggle1 = iup.toggle { title = "Lock Face", action = function(self, state)
	lock_face = state == 1
	lockfacetoggle2.value = (state == 1 and "ON") or "OFF"
	lockfacetoggle3.value = (state == 1 and "ON") or "OFF"
end }
lockfacetoggle2 = iup.toggle { title = "Lock Face", action = function(self, state)
	lock_face = state == 1
	lockfacetoggle1.value = (state == 1 and "ON") or "OFF"
	lockfacetoggle3.value = (state == 1 and "ON") or "OFF"
end }
local lockpropstoggle2
local lockpropstoggle1 = iup.toggle { title = "Lock Props", action = function(self, state)
	lock_props = state == 1
	lockpropstoggle2.value = (state == 1 and "ON") or "OFF"
end }
local lockcameratoggle2
local lockcameratoggle1 = iup.toggle { title = "Lock Camera", action = function(self, state)
	lock_camera = state == 1
	lockcameratoggle2.value = (state == 1 and "ON") or "OFF"
end }
local locklighttoggle2
local locklighttoggle1 = iup.toggle { title = "Lock Light", action = function(self, state)
	lock_light = state == 1
	locklighttoggle2.value = (state == 1 and "ON") or "OFF"
end, value = "ON" }
lockfacetoggle3 = iup.toggle { title = "Face", action = function(self, state)
	lock_face = state == 1
	lockfacetoggle1.value = (state == 1 and "ON") or "OFF"
	lockfacetoggle2.value = (state == 1 and "ON") or "OFF"
end }
lockpropstoggle2 = iup.toggle { title = "Props", action = function(self, state)
	lock_props = state == 1
	lockpropstoggle1.value = (state == 1 and "ON") or "OFF"
end }
lockcameratoggle2 = iup.toggle { title = "Camera", action = function(self, state)
	lock_camera = state == 1
	lockcameratoggle1.value = (state == 1 and "ON") or "OFF"
end }
local autoloadtoggle = iup.toggle { title = "Load Scene", action = function(self, state) auto_load = state == 1 end }
-- local delay
-- local autoplaytoggle = iup.toggle { title = "Advance", action = function(self, state)
	-- auto_play = state == 1
	-- if auto_play then
		-- local timer;
		-- if timer1.run ~= "YES" then
			-- timer = timer1
		-- else
			-- timer = timer2
		-- end
		-- timer.time = tonumber(delay.value) or 1000	-- default delay = 1s
		-- timer.run = "YES"
	-- else
		-- timer1.run = "NO"
		-- timer2.run = "NO"
	-- end
-- end }
-- local autoloadpropstoggle = iup.toggle { title = "Load Props", action = function(self, state) auto_load_props = state == 1 end }
locklighttoggle2 = iup.toggle { title = "Light", action = function(self, state)
	lock_light = state == 1
	locklighttoggle1.value = (state == 1 and "ON") or "OFF"
end, value = "ON" }

local thumbnailbtn = iup.flatbutton {expand="yes"}
local thumbnailvbox = iup.vbox {
	iup.label { title = "Preview" },
	thumbnailbtn
}
		
local function setclip(clip)
	if charamgr.current then
		charamgr.current:setclip(clip)
	end
end
clipchanged.connect(setclip)

local cliptext
local posename = iup.text { expand = "horizontal", visiblecolumns = 20 }
local scenename = iup.text { expand = "horizontal", visiblecolumns = 20 }
local albumname = iup.text { expand = "horizontal", visiblecolumns = 13 }
local pagenumber = iup.text { expand = "horizontal", visiblecolumns = 2 }
local delay = iup.text { expand = "horizontal", visiblecolumns = 3 }
local function getalbumfilename()
	return albumname.value .. "-" .. pagenumber.value .. "-" .. delay.value .. "";
end
local posefilter = lists.listfilter()
local poselist = lists.listbox { expand = "yes", chars = 20, sort = "yes" }
local scenefilter = lists.listfilter()
local scenefilter2 = lists.listfilter()
local scenelist = lists.listbox { expand = "yes", chars = 20, sort = "yes" }
local scenelist2 = lists.listbox { expand = "yes", chars = 20, sort = "yes" }
local loadposebutton = iup.button { title = "Load", expand = "horizontal" }
local saveposebutton = iup.button { title = "Save", expand = "horizontal" }
local saveposetexttoggle = iup.toggle { title = "Save as .pose", value = "OFF" }
local loadscenebutton = iup.button { title = "Load", expand = "horizontal" }
local savescenebutton = iup.button { title = "Save", expand = "horizontal" }
local loadscenebutton2 = iup.button { title = "Load", expand = "horizontal" }
local savescenebutton2 = iup.button { title = "Save", expand = "horizontal" }
local savescenetexttoggle = iup.toggle { title = "Save as .scene", value = "OFF" }
local deleteposebutton = iup.button { title = "Delete" }
local deletescenebutton = iup.button { title = "Delete" }
local deletescenebutton2 = iup.button { title = "Delete" }
local refreshposelistbutton = iup.button { title = "Refresh" }
local refreshscenelistbutton = iup.button { title = "Refresh" }
local refreshscenelistbutton2 = iup.button { title = "Refresh" }
local useposesfolderbutton = iup.button { title = "Use Folder" }
local usescenesfolderbutton = iup.button { title = "Use Folder" }
local usealbumsfolderbutton = iup.button { title = "Open Album" }
local playbackfirstbtn = iup.flatbutton { title = playbacksymbols.first, expand = "horizontal", padding = 3, size = "15x12"  }
local playbackprevbtn = iup.flatbutton { title = playbacksymbols.prev, expand = "horizontal", padding = 3, size = "15x12"  }
local playbackplaypausebtn = iup.flatbutton { title = playbacksymbols.playpause, toggle = "yes", expand = "horizontal", padding = 3, size = "15x12"  }
local playbacknextbtn = iup.flatbutton { title = playbacksymbols.next, expand = "horizontal", padding = 3, size = "15x12"  }
local playbacklastbtn = iup.flatbutton { title = playbacksymbols.last, expand = "horizontal", padding = 3, size = "15x12"  }

local function drawscenethumbnail(dir, filename)
	log.spam("Poser: drawscenethumbnail: %s", dir .. "\\" .. filename .. ".png")	
		
	-- put away the old image on the button
	local oldImg = thumbnailbtn.image
	-- load a png  of the scene
	local newImg = iup.LoadImage(dir .. "\\" .. filename .. ".png")
	-- scale it down
	newImg.autoscale = 0.45
	-- apply to the button
	thumbnailbtn.image = newImg
	-- if button had an image before - nuke that
	if oldImg ~= nil then iup.Destroy(oldImg) end
	-- redraw the button
	iup.Redraw(thumbnailbtn, 0)	
end

signals.connect(poselist, "selectionchanged", function() posename.value = poselist[poselist.value] end )
signals.connect(scenelist, "selectionchanged", function() scenename.value = scenelist[scenelist.value]; if auto_load then loadscenebutton.action() end end )
signals.connect(scenelist2, "selectionchanged", function()
	albumname.value = ""
	pagenumber.value = ""
	delay.value = ""
		
	local text = require("pl.text");
	local args = text.split(scenelist2[scenelist2.value], "-");	
		
	delay.value = table.remove(args)
	pagenumber.value = table.remove(args)	
	albumname.value = table.concat(args, "-")
		
	if auto_load then
		loadscenebutton2.action()
	end
	
	drawscenethumbnail(albumsdir, getalbumfilename())
end )

signals.connect(posefilter, "setfilter", poselist, "setfilter")
signals.connect(scenefilter, "setfilter", scenelist, "setfilter")
signals.connect(scenefilter2, "setfilter", scenelist2, "setfilter")

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

local function populatescenelist(list, dir)
	list.setlist(findposerfiles(dir, "^(.*)%.scene$"))
end

function on.launch()
	if exe_type == "edit" then
		create_thumbnail_function = 0x36C6C1
	else
		create_thumbnail_function = 0x38F6C9
	end
	populateposelist()
	populatescenelist(scenelist, scenesdir)
	populatescenelist(scenelist2, albumsdir)
end

local function get_thumbnail_rotation(angle)
	if is_key_pressed(0x10) then return 0 end
	local half = math.pi / 6
	local right = math.pi / 2
	local left = math.pi / 2 * 3
	if math.abs(angle - right) < half then return 2 end
	if math.abs(angle - left) < half then return 1 end
	return 0
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
	populatescenelist(scenelist, scenesdir)
end

function usealbumsfolderbutton.action()
	albumsdir = fileutils.getfolderdialog(albumsdir)
	populatescenelist(scenelist2, albumsdir)
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
	log.spam("Poser: readpng(%s)", path)
	local file = io.open(path, "rb")
	if not file then return nil end
	log.spam("Poser: readpng: file found")
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
	log.spam("Poser: readfile(%s)", path)
	local file = io.open(path, "rb")
	if not file then return nil end
	log.spam("Poser: readfile: file found")
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
		local thumbnail_message = 0x20 + get_thumbnail_rotation(camera.rotz)
		g_poke(create_thumbnail_function, string.char(thumbnail_message))
	end
end

function saveposebutton.action()
	savepose(posename.value)
end

function refreshposelistbutton.action()
	populateposelist()
end

-- == Scenes ==

local function loadscene(filename, dir)
	local isPng = true
	log.spam("Poser: Loading scene %s\\%s", dir or scenesdir, filename)
	local data = readpng((dir or scenesdir) .. "\\" .. filename .. ".png")
	if data == nil then
		isPng = false
		data = readfile((dir or scenesdir) .. "\\" .. filename .. ".scene")
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
	else
		log.spam("Couldn't find %s\\%s", (dir or scenesdir), filename)
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


local function savescene(filename, dir, list)
	local path = dir .. "\\" .. filename .. ".scene"
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
	embed_save_path = (dir or scenesdir) .. "\\" .. filename .. ".png"
	save_restore_ui = SetHideUI(true)
	local thumbnail_message = 0x10 + get_thumbnail_rotation(camera.rotz)
	g_poke(create_thumbnail_function, string.char(thumbnail_message))

	log.spam("Poser: Scene %s saved", filename)
	local currentvalue = (list or scenelist).value
	
	if list == scenelist then
		if scenename.value ~= list.valuestring then
			log.spam("Poser: savescene: scenename.value: ", scenename.value);
			log.spam("Poser: savescene: list.valuestring: ", list.valuestring);
			list.valuestring = filename
			if list.value == currentvalue then
				list.appenditem = filename
				list.valuestring = filename
			end
		end
	elseif list == scenelist2 then
		if getalbumfilename() ~= list.valuestring then
			log.spam("Poser: savealbumscene: getalbumfilename(): ", getalbumfilename());
			log.spam("Poser: savealbumscene: list.valuestring: ", list.valuestring);
			list.valuestring = filename
			if list.value == currentvalue then
				list.appenditem = filename
				list.valuestring = filename
			end
		end
	end
end

function savescenebutton.action()
	savescene(scenename.value, scenesdir, scenelist)
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

function refreshscenelistbutton.action()	
	populatescenelist(scenelist, scenesdir)
end

-- == Anims ==

function timer1.action_cb()
	local idx = tonumber(scenelist2.value)
	
	if idx == nil or idx < 1 then
	-- if no scene selected - select the first one
		scenelist2.value = 1
	else
	-- select the next scene, loop if needed
		scenelist2.value = ((idx % scenelist2.count) + 1)
	end
	
	-- update the scene name
	albumname.value = ""
	pagenumber.value = ""
	delay.value = ""
		
	local text = require("pl.text");
	local args = text.split(scenelist2[scenelist2.value], "-");	
		
	delay.value = table.remove(args)
	pagenumber.value = table.remove(args)	
	albumname.value = table.concat(args, "-")
	-- and autoload if needed
	if auto_load then
		loadscenebutton2.action()
	end	
	
	if auto_play then
		-- prepare the next frame
		timer2.time = tonumber(delay.value) or 1000
		timer2.run = "YES"
	end
	timer1.run = "NO"
	drawscenethumbnail(albumsdir, getalbumfilename())
  return iup.DEFAULT
end

function timer2.action_cb()
	local idx = tonumber(scenelist2.value)
	
	if idx == nil or idx < 1 then
	-- if no scene selected - select the first one
		scenelist2.value = 1
	else
	-- select the next scene, loop if needed
		scenelist2.value = ((idx % scenelist2.count) + 1)
	end
	
	-- update the scene name
	albumname.value = ""
	pagenumber.value = ""
	delay.value = ""
		
	local text = require("pl.text");
	local args = text.split(scenelist2[scenelist2.value], "-");	
		
	delay.value = table.remove(args)
	pagenumber.value = table.remove(args)	
	albumname.value = table.concat(args, "-")
	-- and autoload if needed
	if auto_load then
		loadscenebutton2.action()
	end	
	
	if auto_play then
		-- prepare the next frame
		timer1.time = tonumber(delay.value) or 1000
		timer1.run = "YES"
	end
	
	timer2.run = "NO"
	drawscenethumbnail(albumsdir, getalbumfilename())
  return iup.DEFAULT
end

function loadscenebutton2.action()
	local ok, ret = pcall(loadscene, getalbumfilename(), albumsdir)
	if not ok then
		log.error("Error loading scene %s/%s:", albumsdir, getalbumfilename())
		log.error(ret)
	end
end

function savescenebutton2.action()
	savescene(getalbumfilename(), albumsdir, scenelist2)
end

function deletescenebutton2.action()
	local resp = iup.Alarm("Confirmation", "Are you sure you want to delete this scene?", "Yes", "No")
	if resp == 1 then
		os.remove(albumsdir .. "\\" .. getalbumfilename() .. ".scene")
		os.remove(albumsdir .. "\\" .. getalbumfilename() .. ".png")
		scenelist2.valuestring = getalbumfilename()
		if scenelist2.valuestring == getalbumfilename() then
			scenelist2.removeitem = scenelist2.value
		end
	end
end

function refreshscenelistbutton2.action()
	populatescenelist(scenelist2, albumsdir)
end


function playbackplaypausebtn.valuechanged_cb(self)
	auto_play = playbackplaypausebtn.value == "ON" and 1 or 0
	if auto_play == 1 then
		local timer;
		if timer1.run ~= "YES" then
			timer = timer1
		else
			timer = timer2
		end
		timer.time = tonumber(delay.value) or 1000	-- default delay = 1s
		timer.run = "YES"
	else
		timer1.run = "NO"
		timer2.run = "NO"
	end
end

function playbackprevbtn.flat_action(self)
	local idx = tonumber(scenelist2.value)
	
	if idx == nil or idx < 1 then
	-- if no scene selected - select the first one
		scenelist2.value = 1
	else
	-- select the next scene, loop if needed
		scenelist2.value = ((idx - 2) % scenelist2.count) + 1
	end
	
	-- update the scene name
	albumname.value = ""
	pagenumber.value = ""
	delay.value = ""
		
	local text = require("pl.text");
	local args = text.split(scenelist2[scenelist2.value], "-");	
		
	delay.value = table.remove(args)
	pagenumber.value = table.remove(args)	
	albumname.value = table.concat(args, "-")
	-- and autoload if needed
	if auto_load then
		loadscenebutton2.action()
	end	
	-- TODO: maybe cancel the current timer and start a new one?
	
	drawscenethumbnail(albumsdir, getalbumfilename())
  return iup.DEFAULT
end

function playbacknextbtn.flat_action(self)

	-- log.spam("playbacknextbtn.valuechanged_cb")
	local idx = tonumber(scenelist2.value)
	
	if idx == nil or idx < 1 then
	-- if no scene selected - select the first one
		scenelist2.value = 1
	else
	-- select the next scene, loop if needed
		scenelist2.value = ((idx % scenelist2.count) + 1)
	end
	
	-- update the scene name
	albumname.value = ""
	pagenumber.value = ""
	delay.value = ""
		
	local text = require("pl.text");
	local args = text.split(scenelist2[scenelist2.value], "-");	
		
	delay.value = table.remove(args)
	pagenumber.value = table.remove(args)	
	albumname.value = table.concat(args, "-")
	-- and autoload if needed
	if auto_load then
		loadscenebutton2.action()
	end	
	-- TODO: maybe cancel the current timer and start a new one?
	
	drawscenethumbnail(albumsdir, getalbumfilename())
  return iup.DEFAULT
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
					lockpropstoggle1,
					lockcameratoggle1,
					locklighttoggle1,
				--	autoloadtoggle,
				--	autoplaytoggle,
					usescenesfolderbutton,
					iup.label { title = "Delete scene:" },
					deletescenebutton,
				},
				tabtitle = "Scenes",
				gap = 3,
			},
			iup.hbox  {
				iup.vbox {
					scenefilter2,
					scenelist2,
					refreshscenelistbutton2,
					expand = "no",
				},
				iup.vbox {			
					iup.hbox{
						iup.vbox {
							iup.label { title = "Chapter" },
							albumname,
						},
						iup.vbox {
							iup.label { title = "Page" },
							pagenumber,
						},
						iup.vbox {
							iup.label { title = "Delay(ms)" },
							delay,
						},
					}, 
					iup.hbox {
						loadscenebutton2,
						iup.fill { size = 10, },
						savescenebutton2,
					},
					iup.hbox {
						-- playbackfirstbtn,
						-- iup.fill { size = 2, },
						playbackprevbtn,
						iup.fill { size = 2, },
						playbacknextbtn,
						iup.fill { size = 2, },
						playbackplaypausebtn,
						-- iup.fill { size = 2, },
						-- playbacklastbtn,
					},
					iup.hbox {
						iup.vbox {
							iup.label { title = "Lock ..." },
							lockfacetoggle3,
							lockpropstoggle2,
							lockcameratoggle2,
							locklighttoggle2,
						},
						iup.vbox {
							iup.label { title = "Auto-" },
							autoloadtoggle,
							autoplaytoggle
							-- autoloadpropstoggle
						},
					},
					usealbumsfolderbutton,
					iup.label { title = "Delete scene:" },
					deletescenebutton2,
				},
				tabtitle = "Albums",
				gap = 3,
			},
			tabchangepos_cb = function(self, newpos, oldpos)
				lock_world = newpos == 0 and lockworldtoggle.value == "ON"
			end,
		},
		thumbnailvbox,
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
