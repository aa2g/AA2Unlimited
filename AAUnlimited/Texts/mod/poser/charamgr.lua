local _M = {}

local signals = require "poser.signals"

local characters = {}
local currentcharacterchanged = signals.signal()
local characterschanged = signals.signal()

-- Character Helper Functions
local function GetSlider(character, slider)
	return character.poser:GetSlider(slider)
end
local function Sliders(character)
	local poser = character.poser
	return poser:Sliders()
end
local function SetClip(character, clip)
	local skel = character.struct.m_xxSkeleton
	skel.m_poseNumber = clip
	skel.m_animFrame = 9000
end
local function GetXXFileFace(character)
	return character.struct:GetXXFileFace()
end

-- Character Helper Metatables
local facekeys = { eye = "m_eye", eyeopen = "m_eyeOpen", eyebrow = "m_eyebrow", mouth = "m_mouth", mouthopen = "m_mouthOpen", blush = "Blush", blushlines = "BlushLines" }
local skelkeys = { pose = "m_poseNumber", frame = "m_animFrame" }
local charamt = {}
charamt.GetSlider = GetSlider
charamt.Sliders = Sliders
charamt.SetClip = SetClip
charamt.GetXXFileFace = GetXXFileFace

function charamt.__index(character,k)
	if facekeys[k] then
		local face = character.struct:GetXXFileFace()
		return face[k]
	end
	if skelkeys[k] then
		local skel = character.struct.m_xxSkeleton
		return skel[skelkeys[k]]
	end
	if charamt[k] then
		return charamt[k]
	end
end
function charamt.__newindex(character,k,v)
	if facekeys[k] then
		local face = character.struct:GetXXFileFace()
		if k == "eyebrow" then
			local base = face.m_eyebrow % 7
			face.m_eyebrow = base + v
		else
			face[facekeys[k]] = v
		end
	elseif skelkeys[k] then
		local skel = character.struct.m_xxSkeleton
		skel[skelkeys[k]] = v
	end
end

local function setcurrentcharacter(character)
	log.spam("Poser: Set current character %s", character)
	if type(character) == "number" then
		_M.current = characters[character]
		currentcharacterchanged(_M.current)
	elseif _M.current ~= character then
		_M.current = character
		currentcharacterchanged(_M.current)
	end
end

function _M.addcharacter(character)
	log.spam("Poser: Add character %s", character)
	local new = true
	local last = 0
	for i,v in ipairs(characters) do
		if v.struct == character then
			new = false
		end
		last = i
	end
	if new then
		local data = character.m_charData
		local name = string.format("%s %s", data.m_forename, data.m_surname)
		new = { name = name, struct = character, poser = GetPoserCharacter(character), data = data, GetSlider = GetSlider }
		setmetatable(new, charamt)
		characters[last + 1] = new
		if not _M.current then
			setcurrentcharacter(new)
		end
	end
	log.spam("Poser: We have %d characters", #characters)
	characterschanged()
end

function _M.removecharacter(character)
	log.spam("Poser: Remove character %s", character)
	if _M.current and _M.current.struct == character then
		setcurrentcharacter(nil)
	end
	for k,v in pairs(characters) do
		if v.struct == character then
			table.remove(characters, k)
		end
	end
	log.spam("Poser: We have %d characters", #characters)
	characterschanged()
end

_M.characters = characters
_M.setcurrentcharacter = setcurrentcharacter
_M.currentcharacterchanged = currentcharacterchanged
_M.characterschanged = characterschanged

return _M
