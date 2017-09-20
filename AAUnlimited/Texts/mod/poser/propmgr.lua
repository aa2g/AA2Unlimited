local _M = {}

local fileutils = require "poser.fileutils"
local signals = require "poser.signals"
local charamgr = require "poser.charamgr"
local proxy = require "poser.proxy"

local propschanged = signals.signal()

-- XX objects are kept in bunch of global 'xxlists' of mostly unknown layout.
-- both load and unload needs to know this list. We use only the topmost
-- list which is always shown.
local xxlist
local roomptr

local loaded = {}



-- 0x1bd is offset in struct Frame which seems to denote index into some sort of table
-- with lightning entries populated from xl files loaded prior.
--
-- 2 is value used by most of character frames, so we force that value for props as well.
-- Note that character (and its xl) must be loaded for that index to be populated.
--
-- 4 is used by actual 3d rooms when loaded by a game, but that indice disappears
-- when the room is unloaded. At this time, we can't populate our own lightning indices,
-- only piggyback on existing ones. Assigning invalid indices seems to have no-op effect,
-- nothing crashes, just dark light. Suggesting this might be indice either into constant
-- uniform of a fragment shader, or a shader program applied as such.
local function walk_fixlight(f)
	for i=0,f.m_nChildren-1 do
		local c = f:m_children(i)
		c.m_lightData = 2
		walk_fixlight(c)
	end
end

function on.end_h()
	roomptr = nil
end

local function loadxx(directory, file)
	-- If loading a room, nuke the one already present
	if roomptr and file:match("^MP_.*") then
		-- XX todo, perhaps make it a separate button
		local orig_room = cast("ExtClass::XXFile", peek_dword(roomptr))
		if orig_room then
			log.spam("unloading previous room %s", orig_room)
			orig_room:Unload(xxlist)
		end
		poke_dword(roomptr, 0)
	end
	local newprop = LoadXX(xxlist, directory .. ".pp", file .. ".xx",0) or nil
	if not newprop then return end
	log.spam("prop struct %s", getmetatable(newprop).__name)
	walk_fixlight(newprop.m_root)
	table.insert(loaded, proxy.wrap(newprop))
	log.spam("loaded xx %s at index %d = %s", file, #loaded, newprop)
	propschanged(#loaded)
end

_M.props = loaded
_M.propschanged = propschanged

function _M.loadprop(path)
	local directory, filename, extension = fileutils.splitfilepath(path)
	local directoryname = string.match(directory, ".*\\(.+)\\")
	log.spam("loadprop( %s )\ndirectory: %s\nfilename: %s\nextension: %s", path, directory, filename, extension)
	if directoryname == "charitems" and extension == "xx" then
		log.spam("loading charitem %s", path)
		local character = charamgr.current
		if not character.origskel then
			character.origskel = character.skelname
		end
		log.spam("inspect %s, %s", character.override, character.reload)
		character:override(charamgr.current.origskel .. ".xx", path)
		character:reload(character.clothstate, 0, 0, 1)
		log.spam("re-spawned character")
	else
		if directory and filename and extension == "xx" then
			log.spam("loading item %s", filename)
			loadxx(directory, filename)
		end
	end
end

function _M.unloadprop(index)
	local prop = loaded[tonumber(index)]
	if prop then
		log.spam("unloading prop %s", prop.name)
		prop:unload(xxlist)
		table.remove(loaded, index)
		propschanged()
	end
end

local orig_bytes
local load_room_fn = 0x93BB0

function _M.init()
	if exe_type == "edit" then
		xxlist = GameBase + 0x00353290
		return
	else
		xxlist = GameBase + 0x00376298
	end

	orig_bytes = g_hook_func(load_room_fn, 6, 3, function(orig, this, a,b,c)
		roomptr = a + 16
		return proc_invoke(orig, this, a, b, c)
	end)
	log("hooked room loading")
end

function _M.cleanup()
	if orig_bytes then
		g_poke(load_room_fn, orig_bytes)
	end
end

return _M
