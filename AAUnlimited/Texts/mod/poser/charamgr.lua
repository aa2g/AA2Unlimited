local _M = {}

local proxy = require "poser.proxy"
local signals = require "poser.signals"

local dummycharacter = {}
local dummymt = {
	__call = function()
		log.warn("calling dummy character")
		return nil
	end,
	
	__index = function()
		log.warn("indexing dummy character")
		return dummycharacter
	end,
	
	__pairs = function()
		log.warn("iterating dummy character")
		return nil
	end
}
setmetatable(dummycharacter, dummymt)
_M.current = dummycharacter

local characters = {} -- characters in scene
local spawned = {} -- used character material slots
local currentcharacterchanged = signals.signal()
local characterschanged = signals.signal()
local on_character_updated = signals.signal()

function _M.reload(character, clothstate, pose, materialslot)
	-- TODO: light/partial now hardcoded
	character.struct:Spawn(clothstate or character.clothstate or 1, materialslot or character.materialslot, 0, 0)
	_M.load_xa(character, pose or 0)
end


local function setcurrentcharacter(character)
	log.spam("Poser: Set current character %s", character)
	if type(character) == "number" then
		_M.current = characters[character] or dummycharacter
	elseif _M.current ~= character then
		_M.current = character
	end
	currentcharacterchanged(_M.current)
end

function _M.character_updated(character, data)
	for i,v in ipairs(characters) do
		if v.struct == character then
			for i,j in pairs(data or {}) do
				log.spam("setting %s %s",i,j)
				v[i] = j
			end
			log.spam("updating %s",v)
			return on_character_updated(data)
		end
	end
	log.warn("Didn't find character to update", character)
end

local charcount = 0 -- this counts only characters spawned by the game, not us
function _M.addcharacter(character)
	log.spam("Poser: Add character %s", character)
	local new
	for _,v in pairs(characters) do
		if v.struct == character then
			new = v
			break
		end
	end
	log.spam("new: %s", new)
	if not new then
		if not _M.is_spawning then
			charcount = charcount + 1
		end
		new = proxy.wrap(character, _M)
		log.spam("Poser: new character %s", character.name)
		new.spawned = new.spawned or false
		characters[#characters + 1] = new
		for i,v in ipairs(characters) do
			log.spam("%d: %s", i, v.name)
		end
		setcurrentcharacter(new)
	end
	new.first_update = false
	log.spam("Poser: We have %d characters", #characters)
	characterschanged()
end

function _M.removecharacter(character)
	log.spam("Poser: Remove character %s", character)
	if _M.current and _M.current.struct == character then
		setcurrentcharacter(dummycharacter)
	end

	local removed
	for k,v in pairs(characters) do
		if v.struct == character then
			table.remove(characters, k)
			removed = v
			break
		end
	end
	if not removed then return end
	log.spam("Poser: We have %d characters left", #characters)
	characterschanged()
	assert(removed.spawned ~= nil)
	if removed.spawned then
		spawned[removed.spawned] = nil
	end
	charcount = charcount - 1
	assert(charcount >= 0)
	log.spam("Poser: charcount %d", charcount)
	-- the legit character despawned, this means the scene is ending - despawn all our injected characters, too
	if charcount == 0 then
		if _M.opts.prunecharacters == 1 then
			log.spam("Poser despawning extra characters")
			while #characters > 0 do
				--local tch = characters[#characters-1]
				local tch = characters[1]
				-- all characters remaining must be spawned ones
				assert(tch.spawned)
				tch:despawn()
			end
		end
		if _M.opts.pruneprops == 1 then
			log.spam("Poser despawning props")
			local propmgr = require "poser.propmgr"
			local props = propmgr.props
			while #props > 0 do
				propmgr.unloadprop(1)
			end
		end
	end
end

function _M.clear_characters()
	while characters[1] and characters[1] ~= dummycharacter do
		_M.removecharacter(characters[1].struct)
	end
end

-- NOTE: these are separated from the proxy because we want to use those *without* the entity proxy,
-- too. Proxy routes to these delegates via entmgr (our _M) argument.
function _M.spawn(character, clothstate, pose)
	_M.is_spawning = true
	local wrapper = proxy.wrap(character, _M)
	characters[#characters + 1] = wrapper
	local slot = #spawned + 1
	spawned[slot] = wrapper
	wrapper.spawned = slot
	wrapper.struct:SetAnimData(true)
	wrapper:reload(clothstate or 1, pose or 0, slot + 1)
	_M.is_spawning = false
end

function _M.despawn(character)
	if not character.spawned then
		log.warn("Poser: Attempt to despawn a game spawned character %s: %s", character.struct, character.name)
		return false
	end
	if character == _M.current then
		_M.setcurrentcharacter(characters[1] or dummycharacter)
	end
	character.struct:Despawn()
	character.spawned = false
	log.spam("Poser: Despawned character: %s", character.name)
	return true
end

function _M.update(character)
	character.struct:Update(character.clothstate or 0, exe_type == "edit" and 1 or 0)
end


function _M.load_xa(character, pose)
	local pp_typ = exe_type:sub(1,1)
	local xa_typ = exe_type == "play" and "K" or "E"
	pp_typ = 'e'
	xa_typ = "E"
	local pp = host_path("data", "jg2%s01_00_00.pp" % pp_typ)
	local xa = host_path("data", "HA%s00_00_%02d_00.xa" % { xa_typ, character.struct.m_charData.m_figure.height })
--	local xa = host_path("data", "HA%s00_00_%02d_00.xa" % { exe_type == "play" and "K" or "E", peek_dword(fixptr(character.struct.m_charData.m_figure)) & 0xff })
	character.struct:LoadXA(pp, xa, pose or 0, 0, 0)
end


_M.characters = characters
_M.setcurrentcharacter = setcurrentcharacter
_M.currentcharacterchanged = currentcharacterchanged
_M.characterschanged = characterschanged
_M.on_character_updated = on_character_updated -- RFC: would be probably nice to distinguish signals by on_ prefix
_M.is_spawning = false

return _M
