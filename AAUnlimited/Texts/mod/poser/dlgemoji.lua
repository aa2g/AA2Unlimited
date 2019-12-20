local fileutils = require "poser.fileutils"
local signals = require "poser.signals"
local lists = require "poser.lists"
local toggles = require "poser.toggles"
local charamgr = require "poser.charamgr"

local function unhidemeshes(frame)
	local count = frame.m_nChildren
	for i = 0, count - 1, 1 do
		unhidemeshes(frame:m_children(i))
	end
	frame.m_meshFlagHide = 0
end

-- #####
-- Emoji
-- #####

local showemoji = function(index, show)
	local char = charamgr.currentcharacter()
	if not char then
		return
	end
	if not char.struct.m_xxFace then
		return
	end
	local emojibaseframe = char.struct.m_xxFace:FindBone("A00_U_emojis")
	if not emojibaseframe then return end
	local frame
	if index < 0 then
		local emojicount = emojibaseframe.m_nChildren
		for i = 0, emojicount - 1, 1 do
			frame = emojibaseframe:m_children(i)
			frame.m_meshFlagHide = show and 0 or 2
		end
	elseif index <= emojibaseframe.m_nChildren then
		frame = emojibaseframe:m_children(index - 1)
		frame.m_meshFlagHide = show and 0 or 2
	end
end

local emojilist = iup.list {
	expand = "yes",
	multiple = "yes",
	visiblelines = 8,
	action = function(self, text, index, state)
		showemoji(index, state == 1)
	end,
}

local populateemoji = function()
	local char = charamgr.currentcharacter()
	if not char then
		return
	end
	emojilist[1] = nil
	if not char.struct.m_xxFace then
		return
	end
	local emojibaseframe = char.struct.m_xxFace:FindBone("A00_U_emojis")
	if not emojibaseframe then
		return
	end
	local emojicount = emojibaseframe.m_nChildren
	for i = 0, emojicount - 1, 1 do
		emojilist[i + 1] = emojibaseframe:m_children(i).m_name
	end
end
charamgr.currentcharacterchanged.connect(populateemoji)

--local showemojibutton = iup.button { title = "Show", expand = "horizontalfree", action = function(self)
--	showemoji(tonumber(emojilist.value), true)
--end}

local hideemojibutton = iup.button { title = "Hide All", expand = "horizontalfree", action = function(self)
	emojilist.value = ""
	showemoji(-1, false)
end}


-- #######
-- Helpers
-- #######

local baseeyebones = { "A00_J_meR1", "A00_J_meL1", "A00_N_hitomiRScaX", "A00_N_hitomiLScaX", "S00_N_hitomiRScaX", "S00_N_hitomiLScaX" }
local features = { "A00_N_mabuga", "A00_N_matugeU", "A00_N_matugeS", "S00_N_mabuga", "S00_N_matugeU", "S00_N_matugeS" }
--local featuresiris = { "A00_N_hitomiRScaX", "A00_N_hitomiLScaX", "S00_N_hitomiRScaX", "S00_N_hitomiLScaX" }
local collapsebutton = iup.button { title = "Collapse eyes", expand = "horizontalfree", action = function(self)
	local character = charamgr.current
	if character and character.ischaracter then
		local face = character.struct.m_xxFace
		for _,bone in pairs(baseeyebones) do
			local slider = charamgr.current:getslider(bone)
			if slider then
				slider:SetCurrentOperation(2)
				slider:SetValues(0, 0, 0)
				slider:Apply()
			end
		end
	end
end}

local hideeyebutton = iup.button { title = "Hide Eyes", expand = "horizontalfree", action = function(self)
	local character = charamgr.current
	if character and character.ischaracter then
		character.eyeopen = 0 -- close eyes
		character.eye = 3 -- set eye shape -_-
		local face = character.struct.m_xxFace
		for _,framename in pairs(features) do
			local baseframe = face:FindBone(framename)
			if baseframe then
				local count = baseframe.m_nChildren
				for i = 1, count, 1 do
					local frame = baseframe:m_children(i - 1)
					--log.info("Frame %s render flag was %d", frame.m_name, frame.m_renderFlag)
					if frame.m_renderFlag == 0 then
						-- remember which facial feature is used
						character[framename] = frame.m_name
						frame.m_renderFlag = 2
					end
				end
			end
		end
	end
end}

local showeyebutton = iup.button { title = "Show Eyes", expand = "horizontalfree", action = function(self)
	local character = charamgr.current
	if character and character.ischaracter then
		for _,bone in pairs(baseeyebones) do
			local slider = character:getslider(bone)
			if slider then
				slider:SetCurrentOperation(2)
				slider:SetValues(1, 1, 1)
				slider:Apply()
			end
		end 
		local face = character.struct.m_xxFace
		for _,framename in pairs(features) do
			local restore = character[framename]
			if restore then
				local baseframe = face:FindBone(framename)
				if baseframe then
					local count = baseframe.m_nChildren
					for i = 1, count, 1 do
						local frame = baseframe:m_children(i - 1)
						if frame.m_name == restore then
							frame.m_renderFlag = 0
						end
					end
				end
			end
		end
	end
end}


local _M = iup.hbox {
	iup.vbox {
		iup.frame {
			title = "Show",
			expand = "horizontalfree",
			iup.vbox {
				--showemojibutton,
				hideemojibutton,
			},
		},
		iup.frame {
			title = "Helpers",
			expand = "horizontalfree",
			iup.vbox {
				collapsebutton,
				hideeyebutton,
				showeyebutton,
			},
		},
	},
	iup.frame {
		title = "Emotes",
		emojilist,
	},
}

return _M
