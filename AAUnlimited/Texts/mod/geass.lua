--@INFO Absolute Obedience Cheat

local _M = {}

local keys = { 0x01, 0x02, 0x04, 0x10, 0x11, 0x12 }
local opts

function on.answer(resp, answering, asking)
	if asking.m_thisChar == GetPlayerCharacter() then
		-- activation key is down
		if (GetAsyncKeyState(keys[opts.key + 1]) & 0x8000) ~= 0 then
			print("GEASS: forcing NPC answer to yes")
			return true
		end
	end
	return resp
end

function _M:load()
	opts = self
	self.key = self.key or 2
end

function _M:unload()
end

function _M:config()
	if not self then return end
	require "iuplua"
	require "iupluacontrols"

	local ok, newkey = iup.GetParam("Set the activation key for Geass", nil, [[
Activation Key: %l|Left Mouse Button|Right Mouse Button|Middle Mouse Button|Shift Key|Control Key|Alt Key|
]], self.key)
	if ok then
		self.key = newkey
	end

	Config.save()
end

return _M
