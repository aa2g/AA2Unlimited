local _M = {}

local fileutils = require "poser.fileutils"
local signals = require "poser.signals"
local charamgr = require "poser.charamgr"

local propschanged = signals.signal()

local xxlist
if exe_type == "edit" then
	xxlist = GameBase + 0x00353290
else
	xxlist = GameBase + 0x00376298
end

local loaded = {}

local function loadxx(directory, file)
	local newprop = LoadXX(xxlist, directory .. ".pp", file .. ".xx",0) or nil
	if not newprop then return end
	table.insert(loaded, {
		name = file,
		xx = newprop,
	})
	log.spam("loaded xx %s at index %d", file, #loaded)
	propschanged(#loaded)
end

_M.props = loaded
_M.propschanged = propschanged

function _M.loadprop(path)
	local directory, filename, extension = fileutils.splitfilepath(path)
	local directoryname = string.match(directory, ".*\\(.+)\\")
	if directoryname == "charitems" and extension == "xx" then
		local skeleton = charamgr.current.skelname
		local character = charamgr.current
		character:override(skeleton .. ".xx", path)
		character.spawn(character.struct, character.clothstate, 0, 0, 1)
	else
		if directory and filename and extension == "xx" then
			loadxx(directory, filename)
		end
	end
end

function _M.unloadprop(index)
	local prop = loaded[tonumber(index)]
	if prop then
		log.spam("unloading prop %s", prop.name)
		prop.xx:Unload(xxlist)
		table.remove(loaded, index)
		propschanged()
	end
end

return _M
