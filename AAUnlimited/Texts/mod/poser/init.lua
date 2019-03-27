--@INFO Poser

require "iuplua"
require "iupluacontrols"


local _M = {}
local dlg
local signals = require "poser.signals"
local charamgr = require "poser.charamgr"
local posemgr
local propmgr = require "poser.propmgr"
local opts = {
	{"autoloading", 0, "Autoload character pose: %b"},
	{"ontop", 2, "Single-window mode: %l|Never|In fullscreen|Always|"},
	{"notitle", 1, "No titlebar: %l|Never|In fullscreen|Always|"},
	{"grab", 1, "Mouse grab-move: %b"},
	{"autofocus", 1, "Window auto-focus: %b"},
	{"prunecharacters", 1, "Auto-prune extra characters on scene end: %b"},
	{"pruneprops", 1, "Auto-prune props on scene end: %b"},
}


local function detect_fs(hwnd)
	dlg.parentHWND = hwnd

	local rect = malloc(16)
	GetWindowRect(GetDesktopWindow(), rect)
	local screenw, screenh = string.unpack("<II", peek(rect+8, 8))

	GetClientRect(hwnd, rect)
	local left, top, right, bottom = string.unpack("<IIII", peek(rect, 16))
	if right == screenw and bottom == screenh then
		log.spam("Forcing non-floating poser windows")
		return true
	end
end

function on.first_tick(hwnd)
	if dlg then
		dlg.fullscreen = detect_fs(hwnd)
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
	-- XXX TODO: distinguish update and skeleton load from reload
	--charamgr.character_updated(character)
	charamgr.addcharacter(character)
end

function on.char_despawn(character)
	charamgr.removecharacter(character)
end

function on.char_xa_end(character,pp,xa,pose)
	xa = xa:match("[^\\]*$")
	charamgr.character_updated(character, {xa=xa,startpose=pose})
end

--[[
local lastwnd = 0
function on.mousemove(wparam,lparam,hwnd,x,y)
	if dlg.visible and dlg[hwnd] and (opts.autofocus == 1) and (lastwnd ~= hwnd) then
		lastwnd = hwnd
		SetFocus(hwnd)
	end
end]]

function _M:load()
	self.cfg = self.cfg or {}
	self.cfg.autoload = self.cfg.autoload or {}
	mod_load_config(self, opts)
	posemgr = require "poser.posemgr"
	dlg = require "poser.dlg"
	dlg.opts = opts
	dlg.cfg = self.cfg
	charamgr.opts = opts
	charamgr.cfg = self.cfg
	posemgr.opts = opts
	posemgr.cfg = self.cfg
	if GetGameTick() > 0 then
		dlg.fullscreen = detect_fs(GetGameHwnd())
	end
	propmgr:init()
end

function on.open_card2()
	if exe_type == "edit" then
		charamgr.clear_characters()
	end
end

function _M:unload()
	-- close all dialogs
	if dlg then dlg:close_all() end
	propmgr:cleanup()
end

function _M:config()
	mod_edit_config(self, opts, "Poser settings")
end

function on.pose_load(charidx, posename)
	local ok, ret = pcall(posemgr.loadpose, charamgr.characters[charidx+1], posename)
	if not ok then
		log.error("Error loading pose %s:", posename)
		log.error(ret)
	end
end

return _M
