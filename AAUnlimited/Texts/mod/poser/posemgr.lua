require "iuplua"
require "iupluacontrols"
require "iupluaim"

local _M = {}
-- _M.dialogposes = {}

local  album_playback_labels  = 
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
	play = "Play",
	pause = "Pause",
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

local settings_state = {
	lock_camera = false,
	lock_light = false,
	lock_face = false,
	lock_props = false,
	lock_world = false,
	unlock_bones = false,
	auto_load_scene = false,
	auto_repeat = false,
	-- auto_play = false, -- loop -- remove/change the uses of a variable directly. later. btw it's global not local atm
	auto_load_props = false, -- loop
	auto_load_chars = false, -- loop
}

local shared_settings_toggles = {
	lock_camera = {},
	lock_light = {},
	lock_face = {},
	lock_props = {},
	unlock_bones = {},
}

local function get_setting(setting)
	return settings_state[setting]
end

local function settings_toggles_update(setting, state, true_value, skip_toggle)
	local value = state == true_value
	settings_state[setting] = value
	local toggles = shared_settings_toggles[setting]
	if toggles ~= nil then
		for index, toggle in ipairs(toggles) do
			if toggle ~= skip_toggle then
				toggle.value = (value and "ON") or "OFF"
			end
		end
	end
end

local lock_world_bone = "a01_N_Zentai_010"

local function create_settings_toggle(toggle_title, setting, true_value)
	local toggle = iup.toggle { title = toggle_title }
	local state = get_setting(setting) and "ON" or "OFF"
	toggle.value = state
	toggle.action = function(self, state)
		settings_toggles_update(setting, state, true_value, toggle)
	end
	local toggles = shared_settings_toggles[setting]
	if toggles ~= nil then
		table.insert(shared_settings_toggles[setting], toggle)
	end
	return toggle
end

local lock_world_toggle = create_settings_toggle("Lock World Bone", "lock_world", 1)
local lock_face_toggle_pose = create_settings_toggle("Lock Face", "lock_face", 1)
local lock_face_toggle_scene = create_settings_toggle("Lock Face", "lock_face", 1)
local lock_face_toggle_album = create_settings_toggle("Lock Face", "lock_face", 1)
local lock_props_toggle_scene = create_settings_toggle("Lock Props", "lock_props", 1)
local lock_props_toggle_album = create_settings_toggle("Lock Props", "lock_props", 1)
local lock_camera_toggle_scene = create_settings_toggle("Lock Camera", "lock_camera", 1)
local lock_camera_toggle_album = create_settings_toggle("Lock Camera", "lock_camera", 1)
local lock_light_toggle_scene = create_settings_toggle("Lock Light", "lock_light", 1)
local lock_light_toggle_album = create_settings_toggle("Lock Light", "lock_light", 1)
local unlock_bones_toggle_scene = create_settings_toggle("Unlock Bones", "unlock_bones", 1)
local unlock_bones_toggle_album = create_settings_toggle("Unlock Bones", "unlock_bones", 1)
-- unlock_bones_toggle_scene.active = "no"
-- unlock_bones_toggle_album.active = "no"

local auto_repeat_album_toggle = create_settings_toggle("Repeat Album", "auto_repeat", 1)
local auto_load_scene_toggle = create_settings_toggle("Load Selected Scene", "auto_load_scene", 1)
local auto_load_props_toggle = create_settings_toggle("Load Props", "auto_load_props", 1)
local auto_unload_props_toggle = create_settings_toggle("Unload Props", "auto_unload_props", 1)
local auto_load_chars_toggle = create_settings_toggle("Load Characters", "auto_load_chars", 1)
local auto_unload_chars_toggle = create_settings_toggle("Unload Characters", "auto_unload_chars", 1)
auto_load_props_toggle.active = "no"
auto_unload_props_toggle.active = "no"
auto_load_chars_toggle.active = "no"
auto_unload_chars_toggle.active = "no"

local thumbnailbtn = iup.flatbutton { expand="VERTICALFREE",
	shrink="yes",
	--floating="yes",
	--image = iup.LoadImage(albumsdir .. "\\" .. "autosave.png"),
	minsize = "492x279",
	maxsize = "1024x558",
}
local thumbnailvbox = iup.vbox{
	iup.label { title = "Preview" , expand="horizontal", shrink="yes" },
	iup.expander {
		iup.vbox{
			thumbnailbtn,
			expandchildren = "yes",
			state = "CLOSE",
		},
	},
	expandchildren = "yes",
}
		
local function setclip(clip)
	if charamgr.current then
		charamgr.current:setclip(clip)
	end
end
clipchanged.connect(setclip)


-- local bonelocks = { a01_N_Skeleton_010 = "[Skeleton]" }
local bonelocks = {}	-- TODO generalize and expand to include other stuff like hidden frames data

function _M.lockBone(idx, frame, name)
	log.spam("lockBone " .. name .. " = " .. frame)	
	local chara = charamgr.characters and charamgr.characters[idx] or charamgr.currentcharacter()
	if chara ~= nil then
		if bonelocks[chara] == nil then bonelocks[chara] = {} end		
		bonelocks[chara][frame] = name
	end

end

function _M.unlockBone(idx, frame)
	log.spam("unlockBone " .. frame)
	local chara = charamgr.characters[idx]	
	if chara ~= nil then
		if bonelocks[chara] == nil then bonelocks[chara] = {} end
		bonelocks[chara][frame] = nil
	end
end


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
local album_frame_filter = lists.listfilter()
local scenelist1 = lists.listbox { expand = "yes", chars = 20, sort = "yes" }
local album_frame_list = lists.listbox { expand = "yes", chars = 20, sort = "yes" }
local loadposebutton = iup.button { title = "Load", expand = "horizontal" }
local saveposebutton = iup.button { title = "Save", expand = "horizontal" }
local saveposetexttoggle = iup.toggle { title = "Save as .pose", value = "OFF" }
local loadscenebutton = iup.button { title = "Load", expand = "horizontal" }
local savescenebutton = iup.button { title = "Save", expand = "horizontal" }
local load_album_frame_button = iup.button { title = "Load", expand = "horizontal" }
local save_album_frame_button = iup.button { title = "Save", expand = "horizontal" }
local savescenetexttoggle = iup.toggle { title = "Save as .scene", value = "OFF" }
local deleteposebutton = iup.button { title = "Delete" }
local deletescenebutton = iup.button { title = "Delete" }
local deletescenebutton2 = iup.button { title = "Delete" }
local refreshposelistbutton = iup.button { title = "Refresh" }
local refreshscenelistbutton = iup.button { title = "Refresh" }
local album_refresh_frame_list_button = iup.button { title = "Refresh" }
local useposesfolderbutton = iup.button { title = "Use Folder" }
local usescenesfolderbutton = iup.button { title = "Use Folder" }
local open_album_button = iup.button { title = "Open Album" }
-- local album_goto_first_frame_button = iup.flatbutton { title = album_playback_labels.first, border="yes", expand = "horizontal", padding = 3, size = "15x12"  }
local album_goto_previous_frame_button = iup.flatbutton { title = album_playback_labels.prev, border="yes", expand = "horizontal", padding = 3, size = "15x12"  }
local album_playback_button = iup.flatbutton { title = album_playback_labels.play, border="yes", toggle = "yes", expand = "horizontal", padding = 3, size = "15x12"  }
local album_goto_next_frame_button = iup.flatbutton { title = album_playback_labels.next, border="yes", expand = "horizontal", padding = 3, size = "15x12"  }
-- local album_goto_last_frame_button = iup.flatbutton { title = album_playback_labels.last, border="yes", expand = "horizontal", padding = 3, size = "15x12"  }
local album_scene_interpolate_button = iup.flatbutton { title = "Interpolate" }
local album_scene_interpolate_slider = iup.val { orientation = "horizontal", value = 0.5, expand = "horizontal" }
local album_scene_interpolate_slider_text = iup.text {
	value = tostring(album_scene_interpolate_slider.value),
	valuechanged_cb = function(self) if tonumber(self.value) then album_scene_interpolate_slider.value = tonumber(self.value) end end
}
album_scene_interpolate_slider.valuechanged_cb = function() album_scene_interpolate_slider_text.value = tostring(album_scene_interpolate_slider.value) end


local function drawscenethumbnail(dir, filename)
-- TODO: refresh thumbnail when we open it and when we save a scene or pose
	log.spam("Poser: drawscenethumbnail: %s", dir .. "\\" .. filename .. ".png")
	-- don't do anything if the preview window is closed
	if thumbnailvbox[2].state == "CLOSE" then return end
	-- put away the old image on the button
	local oldImg = thumbnailbtn.image
	-- load a png  of the scene
	local newImg = iup.LoadImage(dir .. "\\" .. filename .. ".png")
	if newImg == nil then
		newImg = iup.LoadImage(dir .. "\\" .. filename .. ".jpg")
	end
	if newImg ~= nil then	-- skip failed images
		-- scale it down

		-- get the pixel size of the label
		local size_str = iup.GetAttribute(thumbnailbtn, "RASTERSIZE")
		local label_width, label_height = string.match(size_str or "492x279", "(%d+)x(%d+)")

		-- get the pixel size of the image
		local image_width = newImg.width
		local image_height = newImg.height

		-- calculate the aspect ratios of the label and image
		local label_aspect = label_width / label_height
		local image_aspect = image_width / image_height

		log.spam("drawscenethumbnail: labelsize: %sx%s", label_width, label_height)
		log.spam("drawscenethumbnail: imagesize: %sx%s", image_width, image_height)

		-- determine the scale factor to fit the image to the label
		local scale_factor_fit2width = label_width / newImg.width 
		local scale_factor_fit2height = label_height / newImg.height
		local scale_factor = math.min(scale_factor_fit2width,scale_factor_fit2height) -- pick the one that doesn't' stick out
		log.spam("drawscenethumbnail: scale_factor: %s", scale_factor)
		newImg.autoscale = scale_factor

		-- apply to the button
		thumbnailbtn.image = newImg
		-- if button had an image before - nuke that
		if oldImg ~= nil then iup.Destroy(oldImg) end
	end
	-- redraw the button
	iup.Redraw(thumbnailvbox, 1)	
end

--function thumbnailbtn.flat_action()	
--	-- redraw the button
--	iup.Redraw(thumbnailbtn, 0)	
--end

signals.connect(poselist, "selectionchanged", function()
	posename.value = poselist[poselist.value]

	drawscenethumbnail(posesdir, poselist[poselist.value])
end )
signals.connect(scenelist1, "selectionchanged", function()
	scenename.value = scenelist1[scenelist1.value]
	
	drawscenethumbnail(scenesdir, scenelist1[scenelist1.value])
end )
signals.connect(album_frame_list, "selectionchanged", function()
	albumname.value = ""
	pagenumber.value = ""
	delay.value = ""
		
	local text = require("pl.text");
	require 'pl.List'
	local args = text.split(album_frame_list[album_frame_list.value], "-");
	local length = args:len()
		
	if length > 2 then	-- 3+ elements - last two are page and delay, the rest are albumname
		delay.value = table.remove(args)
		pagenumber.value = table.remove(args)
		albumname.value = table.concat(args, "-")
	elseif length == 2  then	-- 2 elements - albumname and pagenumber, delay defaults to 1s
		delay.value = 1000
		pagenumber.value = table.remove(args)
		albumname.value = table.concat(args, "-")
	else -- 1 element - albumname, default pagenumber is the index in the list
		delay.value = 1000
		pagenumber.value = album_frame_list.value
		albumname.value = album_frame_list[album_frame_list.value]
	end

	if get_setting("auto_load_scene") then
		_M.loadalbumscene()
	end
	
	drawscenethumbnail(albumsdir, album_frame_list[album_frame_list.value])
end )

signals.connect(posefilter, "setfilter", poselist, "setfilter")
signals.connect(scenefilter, "setfilter", scenelist1, "setfilter")
signals.connect(album_frame_filter, "setfilter", album_frame_list, "setfilter")

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

function _M.init()
	if exe_type == "edit" then
		create_thumbnail_function = 0x36C6C1
	else
		create_thumbnail_function = 0x38F6C9
	end
	populateposelist()
	populatescenelist(scenelist1, scenesdir)
	populatescenelist(album_frame_list, albumsdir)
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
	if file ~= nil then
		file:write(embed_file)
		file:write(string.pack("<I4", #embed_file))
		file:write(embed_magic)
		file:close()
		log.spam("Thumbnail with data saved")
	else	
		log.error("Error saving thumbnail with data! File not found?")
	end
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
	populatescenelist(scenelist1, scenesdir)
end

function open_album_button.action()
	albumsdir = fileutils.getfolderdialog(albumsdir)
	populatescenelist(album_frame_list, albumsdir)
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

local function slerpSlider(slider, q, val)
	charamgr.current:quatsliderslerp(slider, q, val)
end

local function table2poseInterpolated(pose, character, interpolation)
	local version = pose._VERSION_ or 1
	local clip = pose.pose
	if clip then
		character:setclip(clip)
		cliptext.value = clip
	end
	local frame = pose.frame
	if pose.sliders then
		for k,v in pairs(pose.sliders) do		
			local slider = not ((k == lock_world_bone and get_setting("lock_world")) or (bonelocks[character] ~= nil and bonelocks[character][k] ~= nil and not get_setting("unlock_bones"))) and character:getslider(k)
			
			if bonelocks[character] ~= nil and bonelocks[character][k] ~= nil then
				log.spam("bonelock enabled [%s]: %s", k, bonelocks[character][k])
			end
			
			if slider then
				if version == 1 then
					slider:SetValues(v[1], v[2], v[3])
				else
					slerpSlider(slider, { v[1], v[2], v[3], v[4] }, interpolation)
				end
				slider:translate(0, slider:translate(0) + (v[3 + version] - slider:translate(0)) * interpolation)
				slider:translate(1, slider:translate(1) + (v[4 + version] - slider:translate(1)) * interpolation)
				slider:translate(2, slider:translate(2) + (v[5 + version] - slider:translate(2)) * interpolation)
				
				slider:scale(0,slider:scale(0) + (v[6 + version] - slider:scale(0)) * interpolation)
				slider:scale(1,slider:scale(1) + (v[7 + version] - slider:scale(1)) * interpolation)
				slider:scale(2,slider:scale(2) + (v[8 + version] - slider:scale(2)) * interpolation)

				slider:Apply()
			end
		end
	end
	local face = pose.face
	if face and not get_setting("lock_face") then
		if face.mouth then character.mouth = face.mouth end
		if face.mouthopen then character.mouthopen = face.mouthopen end
		if face.eye then character.eye = face.eye end
		if face.eyeopen then character.eyeopen = face.eyeopen end
		if face.eyebrow then character.eyebrow = face.eyebrow end

		local facestruct = character.struct.m_xxFace
		local material = facestruct:FindMaterial("A00_M_hoho") or facestruct:FindMaterial("S00_M_hoho") 
		if material and face.blush then
			material:m_lightingAttributes(3, face.blush / 9)
		end
		material = facestruct:FindMaterial("A00_M_hohosen") or facestruct:FindMaterial("S00_M_hohosen") 
		if material and face.blushlines then
			material:m_lightingAttributes(3, face.blushlines / 9)
		end
	end
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
			local slider = not ((k == lock_world_bone and get_setting("lock_world")) or (bonelocks[character] ~= nil and bonelocks[character][k] ~= nil and not get_setting("unlock_bones"))) and character:getslider(k)
			
			if bonelocks[character] ~= nil and bonelocks[character][k] ~= nil then
				log.spam("bonelock enabled [%s]: %s", k, bonelocks[character][k])
			end
			
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
	if face and not get_setting("lock_face") then
		if face.mouth then character.mouth = face.mouth end
		if face.mouthopen then character.mouthopen = face.mouthopen end
		if face.eye then character.eye = face.eye end
		if face.eyeopen then character.eyeopen = face.eyeopen end
		if face.eyebrow then character.eyebrow = face.eyebrow end

		local facestruct = character.struct.m_xxFace
		local material = facestruct:FindMaterial("A00_M_hoho") or facestruct:FindMaterial("S00_M_hoho") 
		if material and face.blush then
			material:m_lightingAttributes(3, face.blush / 9)
		end
		material = facestruct:FindMaterial("A00_M_hohosen") or facestruct:FindMaterial("S00_M_hohosen") 
		if material and face.blushlines then
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

function _M.loadSceneData(scene)
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

		if not get_setting("lock_props") then
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
				log.warn("Load Scene: Props Not Found: %s", table.concat(not_found, ", "))
			end
		end
		
		if not get_setting("lock_camera") then
			for k,v in pairs(scene.camera or {}) do
				camera[k] = v
			end
		end

		if not get_setting("lock_light") and scene.light then
			local direction = scene.light.direction
			for _,character in pairs(charamgr.characters) do
				local skeleton = character.struct.m_xxSkeleton
				local lightcount = skeleton.m_lightsCount
				for i = 1, lightcount, 1 do
					local light = skeleton:m_lightsArray(i - 1)
					light:SetLightDirection(direction[1], direction[2], direction[3], direction[4], 1.0)
				end
			end
		end
	end	
end

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
		_M.loadSceneData(scene)
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
	local currentvalue = (list or scenelist1).value
	
	if list == scenelist1 then
		if scenename.value ~= list.valuestring then
			log.spam("Poser: savescene: scenename.value: ", scenename.value);
			log.spam("Poser: savescene: list.valuestring: ", list.valuestring);
			list.valuestring = filename
			if list.value == currentvalue then
				list.appenditem = filename
				list.valuestring = filename
			end
		end
	elseif list == album_frame_list then
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
	savescene(scenename.value, scenesdir, scenelist1)
end

function deletescenebutton.action()
	local resp = iup.Alarm("Confirmation", "Are you sure you want to delete this scene?", "Yes", "No")
	if resp == 1 then
		os.remove(scenesdir .. "\\" .. scenename.value .. ".scene")
		os.remove(scenesdir .. "\\" .. scenename.value .. ".png")
		scenelist1.valuestring = scenename.value
		if scenelist1.valuestring == scenename.value then
			scenelist1.removeitem = scenelist1.value
		end
	end
end

function refreshscenelistbutton.action()	
	populatescenelist(scenelist1, scenesdir)
end


_M.interpolateScene = function(scene)
	if scene then
		local characters = scene.characters or {}
		local props = scene.props or {}
		local interpolationValue = album_scene_interpolate_slider.value
		
		local loadedprops = {}
		for i,v in pairs(propmgr.props) do
			loadedprops[i] = v
		end
		
		for i,readchara in ipairs(characters) do
			local chara = charamgr.characters[i]
			if chara then
				table2poseInterpolated(readchara.pose, chara, interpolationValue)
			end
		end
		
		local not_found = {}

		if not get_setting("lock_props") then
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
								slerpSlider(slider, {v[1], v[2], v[3], v[4]}, interpolationValue);

								slider:translate(0, slider:translate(0) + (v[5] - slider:translate(0)) * interpolationValue)
								slider:translate(1, slider:translate(1) + (v[6] - slider:translate(1)) * interpolationValue)
								slider:translate(2, slider:translate(2) + (v[7] - slider:translate(2)) * interpolationValue)
								
								slider:scale(0, slider:scale(0) + (v[8] - slider:scale(0)) * interpolationValue)
								slider:scale(1, slider:scale(1) + (v[9] - slider:scale(1)) * interpolationValue)
								slider:scale(2, slider:scale(2) + (v[10] - slider:scale(2)) * interpolationValue)

								slider:Apply()
							end
						end
					end
				end
			end
			if #not_found > 0 then
				log.warn("Load Scene: Props Not Found: %s", table.concat(not_found, ", "))
			end
		end
		
		if not get_setting("lock_camera") then
			
			log.spam("interpolateScene: original camera eu: [%s, %s, %s]", camera.rotx, camera.roty, camera.rotz);
			log.spam("interpolateScene:   target camera eu: [%s, %s, %s]", scene.camera.rotx, scene.camera.roty, scene.camera.rotz);
			local oldX, oldY, oldZ, oldW = charamgr.current:euler2quat({ camera.rotx, camera.roty, camera.rotz })
			log.spam("interpolateScene: camera old qu: [%s, %s, %s, %s]", oldX, oldY, oldZ, oldW);
			local newX, newY, newZ, newW = charamgr.current:euler2quat({ scene.camera.rotx, scene.camera.roty, scene.camera.rotz })
			log.spam("interpolateScene: camera new qu: [%s, %s, %s, %s]", newX, newY, newZ, newW);
			local outqX, outqY, outqZ, outqW = charamgr.current:quatslerp(
				{ oldX, oldY, oldZ, oldW },
				{ newX, newY, newZ, newW },
				interpolationValue);
			log.spam("interpolateScene: camera in: [%s, %s, %s, %s]", outqX, outqY, outqZ, outqW);
			local qx, qy, qz = charamgr.current:quat2euler({outqX, outqY, outqZ, outqW});
			camera.rotx = qx
			camera.roty = qy
			camera.rotz = qz
			
			log.spam("interpolateScene: result euler camera: [%s, %s, %s]", qx, qy, qz);

			camera.shiftx = camera.shiftx + (scene.camera.shiftx - camera.shiftx) * interpolationValue
			camera.shifty = camera.shifty + (scene.camera.shifty - camera.shifty) * interpolationValue
			camera.shiftz = camera.shiftz + (scene.camera.shiftz - camera.shiftz) * interpolationValue

			camera.fov = camera.fov + (scene.camera.fov - camera.fov) * interpolationValue
			camera.dist_to_mid = camera.dist_to_mid + (scene.camera.dist_to_mid - camera.dist_to_mid) * interpolationValue -- use x scale ratio to modify current distance
		end

		if not get_setting("lock_light") and scene.light then
			local direction = scene.light.direction
			for _,character in pairs(charamgr.characters) do
				local skeleton = character.struct.m_xxSkeleton
				local lightcount = skeleton.m_lightsCount
				for i = 1, lightcount, 1 do
					local light = skeleton:m_lightsArray(i - 1)
					light:SetLightDirection(direction[1], direction[2], direction[3], direction[4], interpolationValue)
				end
			end
		end
	end	
end

album_scene_interpolate_button.flat_action = function()
	local filename = getalbumfilename();
	local isPng = true
	log.spam("Poser: Interpolating into scene %s\\%s", albumsdir or scenesdir, filename)
	local data = readpng((albumsdir or scenesdir) .. "\\" .. filename .. ".png")
	if data == nil then
		isPng = false
		data = readfile((albumsdir or scenesdir) .. "\\" .. filename .. ".scene")
	end
	if data then
		local ok, scene = pcall(json.decode, data)
		if not ok then
			error("Error decoding scene %s data:" % filename)
		end
		_M.interpolateScene(scene)
	else
		log.spam("Couldn't find %s\\%s", (albumsdir or scenesdir), filename)
	end
	sceneloaded()
end

-- == Albums ==

local album_playlist
local album_playback_position = 1

local animation_time = 0
local animation_program
local animation_program_operation

local album_playback_timer_callback
local album_program_timer_callback

local album_show_frame

local timers = {}
local function create_timer(name, time, callback)
	-- log.spam("Creater timer %s (%d)", name, time)
	local old = timers[name]
	if old then old.run = "NO" end
	local timer = iup.timer {time = time, action_cb = callback}
	timer.run = "YES"
	timers[name] = timer
	return timer
end

local function stop_timer(name)
	local timer = timers[name]
	if timer then
		-- log.spam("Destroyed timer %s", name)
		timers[name].run = "NO"
		timers[name] = nil
	end
end

local function album_program_timer_callback_protected()
	local ok, err = pcall(album_program_timer_callback)
	if not ok then
		stop_timer("program")
		log.error("Error in album program processing!\n%s", err)
	end
end

local function album_playback_timer_callback_protected()
	local ok, err = pcall(album_playback_timer_callback)
	if not ok then
		stop_timer("playback")
		log.error("Error in album playback processing!\n%s", err)
	end
end

album_playback_timer_callback = function()
	if album_playlist == nil then
		log.error("Something is wrong with the album playback state. Aborting")
		stop_timer("playback")
	end

	animation_time = animation_time + timers.playback.time

	if album_playback_position > #album_playlist and get_setting("auto_repeat") then
		album_playback_position = 1
	end

	local current_frame = album_playlist[album_playback_position]
	local delay = current_frame[1]
	local frame_name = current_frame[2]

	local ok, ret = album_show_frame(frame_name)
	if not ok then
		stop_timer("playback")
		return
	end

	album_playback_position = album_playback_position + 1
	create_timer("playback", delay, album_playback_timer_callback_protected)
end

album_program_timer_callback = function()
	animation_time = animation_program_operation[1]
	-- log.info("program callback. time is %d", animation_time)
	local time = 0
	while time do
		time = animation_program_operation[1]
		if animation_time >= time then
			local character = animation_program_operation[2]
			local operations = animation_program_operation[3]
			for _,op in ipairs(operations) do
				character[op[1]] = op[2]
			end
			animation_program_operation = nil
		else
			time = nil
			return
		end
		table.remove(animation_program, 1)
		animation_program_operation = animation_program[1]
		if animation_program_operation == nil then
			time = nil
			animation_program = nil
			stop_timer("program")
		else
			time = animation_program_operation[1]
			local remainder = time - animation_time
			if remainder < 10 then
				log.error("Program difference (%d at %d)is too small! Smaller than 10. Stopped program processing.", remainder, animation_time, time)
				stop_timer("program")
				return
			end
			create_timer("program", remainder, album_program_timer_callback_protected)
			return
		end
	end
end

album_show_frame = function(frame)
	local ok, ret = pcall(loadscene, frame, albumsdir)
	if not ok then
		stop_timer("playback")
		log.error("There was an error playing this album")
		log.error(ret)
	end
	return ok, ret
end

local function album_playback(playlist, starting_position)
	if type(playlist) ~= "table" or type(starting_position) ~= "number" then
		return
	end

	if starting_position < 1 or #playlist > starting_position then
		starting_position = 1
	end

	local frame = playlist[starting_position]
	local ok, ret = album_show_frame(frame[2])
	if not ok then
		return
	end

	album_playlist = playlist
	album_playback_position = starting_position + 1
	create_timer("playback", frame[1], album_playback_timer_callback_protected)
	animation_time = 0

	if animation_program ~= nil then
		create_timer("program", animation_program[1][1], album_program_timer_callback_protected)
	end
end

local function album_pause()
	stop_timer("playback")
	stop_timer("program")
end

local function setup_animation_program()
	animation_program = nil
	local program_path = make_path(albumsdir,"program.data")
	local program_data = io.open(program_path, "r")
	if program_data then
		local new_program = {}
		for line in program_data:lines() do
			for time, character_index, operations in string.gmatch(line, "(%d+) (%d+) (.*)$") do
				local time = tonumber(time)
				local character = tonumber(character_index)
				character = charamgr.characters[character]
				local ops = {}
				local reg = {time, character, ops}
				for property, value in string.gmatch(operations, "(%w+)=(%d+)") do
					table.insert(ops, {property, tonumber(value)})
				end
				-- log.info("Added program data %d %s %s %d", time, character.name, property, value)
				table.insert(new_program, {time, character, ops})
			end
		end
		if #new_program > 0 then
			animation_program = new_program
			animation_program_operation = new_program[1]
		end
	end
end

function _M.loadalbumscene()
	local ok, ret = pcall(loadscene, getalbumfilename(), albumsdir)
	if not ok then
		log.error("Error loading scene %s/%s:", albumsdir, getalbumfilename())
		log.error(ret)
	end
end

function load_album_frame_button.action()
	_M.loadalbumscene()
	album_playback_button.value = "OFF"
end

function save_album_frame_button.action()
	savescene(getalbumfilename(), albumsdir, album_frame_list)
	album_playback_button.value = "OFF"
end

function deletescenebutton2.action()
	local resp = iup.Alarm("Confirmation", "Are you sure you want to delete this scene?", "Yes", "No")
	if resp == 1 then
		os.remove(albumsdir .. "\\" .. getalbumfilename() .. ".scene")
		os.remove(albumsdir .. "\\" .. getalbumfilename() .. ".png")
		album_frame_list.valuestring = getalbumfilename()
		if album_frame_list.valuestring == getalbumfilename() then
			album_frame_list.removeitem = album_frame_list.value
		end
	end
end

function album_refresh_frame_list_button.action()
	populatescenelist(album_frame_list, albumsdir)
end


function album_playback_button.valuechanged_cb(self)
	if album_playback_button.value == "OFF" then
		album_pause()
		album_playback_button.title = album_playback_labels.play
		return
	end

	local starting_frame_index = tonumber(album_frame_list.value)
	
	if starting_frame_index == nil or starting_frame_index < 1 then
	-- if no scene selected - select the first one
		starting_frame_index = 1
	end

	local frame_playlist = {}
	for frame_position = 1,album_frame_list.count do
		local frame_name = album_frame_list[tostring(frame_position)]
		local text = require("pl.text");
		local args = text.split(frame_name, "-");
		local delay = tonumber(table.remove(args)) or 1000
		table.insert(frame_playlist, {delay, frame_name})
	end

	setup_animation_program()

	if album_playback_button.value == "ON" then
		album_playback(frame_playlist, starting_frame_index)
		album_playback_button.title = album_playback_labels.pause
	end
end

function album_goto_previous_frame_button.flat_action(self)
	if not album_frame_list.value then return end

	local idx = tonumber(album_frame_list.value)
	
	if idx == nil or idx < 1 then
	-- if no scene selected - select the first one
		album_frame_list.value = 1
	else
	-- select the next scene, loop if needed
		album_frame_list.value = ((idx - 2) % album_frame_list.count) + 1
	end
	
	-- update the scene name
	albumname.value = ""
	pagenumber.value = ""
	delay.value = ""
		
	local text = require("pl.text");
	local args = text.split(album_frame_list[album_frame_list.value], "-");
		
	delay.value = table.remove(args)
	pagenumber.value = table.remove(args)	
	albumname.value = table.concat(args, "-")
	-- and autoload if needed
	if get_setting("auto_load_scene") then
		_M.loadalbumscene()
	end	
	-- TODO: maybe cancel the current timer and start a new one?
	
	drawscenethumbnail(albumsdir, getalbumfilename())
end

function album_goto_next_frame_button.flat_action(self)
	if not album_frame_list.value then return end

	local idx = tonumber(album_frame_list.value)
	
	if idx == nil or idx < 1 then
	-- if no scene selected - select the first one
		album_frame_list.value = 1
	else
	-- select the next scene, loop if needed
		album_frame_list.value = ((idx % album_frame_list.count) + 1)
	end
	
	-- update the scene name
	albumname.value = ""
	pagenumber.value = ""
	delay.value = ""
		
	local text = require("pl.text");
	local args = text.split(album_frame_list[album_frame_list.value], "-");
		
	delay.value = table.remove(args)
	pagenumber.value = table.remove(args)	
	albumname.value = table.concat(args, "-")
	-- and autoload if needed
	if get_setting("auto_load_scene") then
		_M.loadalbumscene()
	end	
	-- TODO: maybe cancel the current timer and start a new one?
	
	drawscenethumbnail(albumsdir, getalbumfilename())
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
					lock_world_toggle,
					lock_face_toggle_pose,
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
					scenelist1,
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
					lock_face_toggle_scene,
					unlock_bones_toggle_scene,
					lock_props_toggle_scene,
					lock_camera_toggle_scene,
					lock_light_toggle_scene,
				--	auto_load_scene_toggle,
					usescenesfolderbutton,
					iup.label { title = "Delete scene:" },
					deletescenebutton,
				},
				tabtitle = "Scenes",
				gap = 3,
			},
			iup.hbox  {
				iup.vbox {
					album_frame_filter,
					album_frame_list,
					iup.hbox {
						album_refresh_frame_list_button,
						open_album_button,
					},
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
						load_album_frame_button,
						iup.fill { size = 10, },
						save_album_frame_button,
					},
					iup.hbox {
						-- album_goto_first_frame_button,
						-- iup.fill { size = 2, },
						album_goto_previous_frame_button,
						iup.fill { size = 2, },
						album_goto_next_frame_button,
						iup.fill { size = 2, },
						album_playback_button,
						-- iup.fill { size = 2, },
						-- album_goto_last_frame_button,
					},
					iup.tabs {
						iup.vbox {
							-- iup.label { title = "Lock ..." },
							lock_face_toggle_album,
							unlock_bones_toggle_album,
							lock_props_toggle_album,
							lock_camera_toggle_album,
							lock_light_toggle_album,
							gap = 3,
							expandchildren = "yes",
							tabtitle = "Lock",
						},
						iup.vbox {
							-- iup.label { title = "Auto" },
							-- auto_play_toggle,
							auto_repeat_album_toggle,
							auto_load_scene_toggle,
							-- auto_load_props_toggle,
							-- auto_unload_props_toggle,
							-- auto_load_chars_toggle,
							-- auto_unload_chars_toggle,
							gap = 3,
							expandchildren = "yes",
							tabtitle = "Auto",
						},
						iup.vbox {
							iup.hbox {
							album_scene_interpolate_slider_text,
							album_scene_interpolate_slider,
							},
							album_scene_interpolate_button,
							gap = 3,
							expandchildren = "yes",
							tabtitle = "Pro",
						},
						tabtype = "left",
					},
					iup.label { title = "Delete scene:" },
					deletescenebutton2,
				},
				tabtitle = "Albums",
				gap = 3,
			},
		},
		thumbnailvbox,
		iup.fill { size = 3, },
		iup.hbox {
			iup.button { title = "Show UI", action = function() SetHideUI(false) end },
			iup.button { title = "Hide UI", action = function() SetHideUI(true) end },
		},
		expandchildren="yes",
	},
	nmargin = "3x3",
	maxbox = "no",
	minbox = "no",
	shrink = "yes"
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

function _M.albumdir()
	return albumsdir
end

_M.albumfilename = getalbumfilename

return _M
