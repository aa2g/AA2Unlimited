--@INFO Poser

require "iuplua"
require "iupluacontrols"

local _M = {}
local dlg
local opts
local signals = require "poser.signals"

function on.first_tick(hwnd)
	dlg.parentHWND = hwnd
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
