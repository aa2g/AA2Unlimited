--@INFO Poser

require "iuplua"
require "iupluacontrols"

local _M = {}
local dlg
local signals = require "poser.signals"
local charamgr = require "poser.charamgr"

local function detect_parenting(hwnd)
	dlg.parentHWND = hwnd

	local rect = malloc(16)
	GetWindowRect(GetDesktopWindow(), rect)
	local screenw, screenh = string.unpack("<II", peek(rect+8, 8))

	GetClientRect(hwnd, rect)
	local left, top, right, bottom = string.unpack("<IIII", peek(rect, 16))
	if right == screenw and bottom == screenh then
		log.info("Forcing non-floating poser windows")
		return true
	end
end

function on.first_tick(hwnd)
	if dlg then
		dlg.forceparenting = detect_parenting(hwnd)
	end
end

function on.keydown(k)
	if k == 0x7B then -- F12
		dlg.togglevisible()
	end
end

function on.char_spawn_end(ret,character)
	charamgr.addcharacter(character)
end

function on.char_update_end(ret,character)
	charamgr.addcharacter(character)
end

function on.char_despawn(character)
	charamgr.removecharacter(character)
end

function _M:load()
	dlg = require "poser.dlg"
	if GetGameTick() > 0 then
		dlg.forceparenting = detect_parenting(GetGameHwnd())
	end
end

function _M:unload()
	-- close all dialogs
	if dlg then dlg:close_all() end
end

return _M