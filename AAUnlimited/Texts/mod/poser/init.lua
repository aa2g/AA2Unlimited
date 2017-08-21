--@INFO Poser

require "iuplua"
require "iupluacontrols"

local _M = {}
local dlg
local opts
local signals = require "poser.signals"

local characters = {}
local forceparenting

function on.first_tick(hwnd)
	dlg.parentHWND = hwnd

	local rect = malloc(16)
	GetWindowRect(GetDesktopWindow(), rect)
	local screenw, screenh = string.unpack("<II", peek(rect+8, 8))

	GetClientRect(hwnd, rect)
	local left, top, right, bottom = string.unpack("<IIII", peek(rect, 16))
	if right == screenw and bottom == screenh then
		log.info("Forcing non-floating poser windows")
		forceparenting = true
	end
end

function on.keydown(k)
	if k == 0x7B then -- F12
		dlg.togglevisible()
	end
end

function on.char_spawn_end(ret,character)
	dlg.addcharacter(character)
end

function on.char_despawn(character)
	dlg.removecharacter(character)
end

function on.poserframemod(xx)
end

function _M:load()
	dlg = require "poser.dlg"
	dlg.forceparenting = forceparenting
end

return _M
