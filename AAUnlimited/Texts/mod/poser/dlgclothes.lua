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

-- ####
-- Legs
-- ####

local legslist = iup.list {
	expand = "yes",
	visiblelines = 8,
}

local populatelegs = function()
	local char = charamgr.currentcharacter()
	if not char then
		return
	end
	legslist[1] = nil
	if not char.struct.m_xxLegs then
		return
	end
	local legsbaseframe = char.struct.m_xxLegs:FindBone("A00_null_kutu")
	if not legsbaseframe then
		return
	end
	local legcount = legsbaseframe.m_nChildren
	for i = 0, legcount - 1, 1 do
		legslist[i + 1] = legsbaseframe:m_children(i).m_name
	end
end
charamgr.currentcharacterchanged.connect(populatelegs)

local selectlegs = function(index)
	local char = charamgr.currentcharacter()
	if not char then
		return
	end
	local legsbaseframe = char.struct.m_xxLegs:FindBone("A00_null_kutu")
	local legcount = legsbaseframe.m_nChildren
	for i = 1, legcount, 1 do
		legsbaseframe:m_children(i - 1).m_renderFlag = i == index and 0 or 2
	end
end

legslist.action = function(self, text, item, state)
	if state == 1 then
		selectlegs(item)
	end
end


-- #####
-- Skirt
-- #####

local skirtlist = iup.list {
	expand = "yes",
	visiblelines = 8,
}

local populateskirt = function()
	local char = charamgr.currentcharacter()
	if not char then
		return
	end
	skirtlist[1] = nil
	if not char.struct.m_xxSkirt then
		return
	end
	local skirtbaseframe = char.struct.m_xxSkirt:FindBone("A00_null_sukato")
	if not skirtbaseframe then
		return
	end
	local legcount = skirtbaseframe.m_nChildren
	for i = 0, legcount - 1, 1 do
		skirtlist[i + 1] = skirtbaseframe:m_children(i).m_name
	end
end
charamgr.currentcharacterchanged.connect(populateskirt)

local selectskirt = function(index)
	local char = charamgr.currentcharacter()
	if not char then
		return
	end
	local skirtbaseframe = char.struct.m_xxSkirt:FindBone("A00_null_sukato")
	local legcount = skirtbaseframe.m_nChildren
	for i = 1, legcount, 1 do
		if i == index then
			local frame = skirtbaseframe:m_children(i - 1)
			unhidemeshes(frame)
			frame.m_renderFlag = 0
		else
			skirtbaseframe:m_children(i - 1).m_renderFlag = 2
		end
	end
end

skirtlist.action = function(self, text, item, state)
	if state == 1 then
		selectskirt(item)
	end
end


-- ########
-- Clothing
-- ########

local clothslot = toggles.slotbuttons("Slot", { "1", "2", "3", "4" }, function(type)
	local character = charamgr.current
	if character then
		character.clothtype = type
	end
end)

local clothstate = toggles.slotbuttons("State", { "0", "1", "2", "3", "4" }, function(state)
	local character = charamgr.current
	if character then
		character:update(state)
	end
end)

local loadstyle = exe_type == "play" and toggles.slotbuttons("Style", { "1", "2", "3", "4", ">" }, function(state)
	local character = charamgr.current
	if character then
		local characterdata = GetCharInstData(character.struct.m_seat)
		local stylecount = characterdata:GetStyleCount()
		if stylecount > 1 then
			local style
			if state == 4 then
				style = (characterdata:GetCurrentStyle() + 1) % stylecount
			else
				style = state
			end
			characterdata:SetCurrentStyle(style)
			character:update(character.struct.m_clothState)
		end
	end
end)

local loadclothbutton = iup.button { title = "Load Cloth", action = function(self)
	local character = charamgr.current
	if character then
		local file = getfile(play_path("data\\save\\cloth\\*.cloth"))
		local size
		if file then file, size = fileutils.readfile(file) end
		if file and size == 92 then character:loadcloth(file) end
	end
end}


local _M = iup.hbox {
	iup.frame {
		title = "Clothing",
		iup.vbox {
			clothslot,
			clothstate,
			loadstyle,
			iup.hbox {
				loadclothbutton,
			},
		},
	},
	iup.frame {
		title = "Leg Wear",
		legslist,
	},
	iup.frame {
		title = "Skirt",
		skirtlist,
	},
}

return _M
