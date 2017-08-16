--@INFO Poser

require "iuplua"
require "iupluacontrols"

local _M = {}
local dlg
local opts
local signals = require "poser.signals"

function on.first_tick(hwnd)
	dlg.parentHWND = hwnd

	local rect = malloc(16)
	GetWindowRect(GetDesktopWindow(), rect)
	local screenw, screenh = string.unpack("<II", peek(rect+8, 8))

	GetClientRect(hwnd, rect)
	local left, top, right, bottom = string.unpack("<IIII", peek(rect, 16))
	if right == screenw and bottom == screenh then
		opts.forcefullscreen = true
	end
end

function on.keydown(k)
	if k == 0x7B then -- F12
		dlg.togglevisible()
	end
end

function _M:load()
	opts = self
	self.floating = self.floating or 1
	dlg = require "poser.dlg"
	dlg.opts = opts
end

function _M:config()
	if not self then
		return
	end

	local ok, floating = iup.GetParam("Configure poser", nil, [[
Floating windows: %b
]], self.floating)
	if ok then
		self.floating = floating
		
		dlg.opts = opts
		if dlg then dlg.updatefloating() end
	end

	Config.save()
end

return _M
