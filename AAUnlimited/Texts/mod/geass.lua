--@INFO Absolute Obedience Cheat

local _M = {}

local keys = { 0x01, 0x02, 0x04, 0x10, 0x11, 0x12 }
local opts

function on.answer(resp, answering, asking)
	-- not everyone, not us either
	if (opts.everyone == 0) and (asking.m_thisChar ~= GetPlayerCharacter()) then
		return resp
	end

	-- activation key is down
	if (GetAsyncKeyState(keys[opts.key + 1]) & 0x8000) ~= 0 then
		print("GEASS: forcing NPC answer to yes")
		return true
	end

	if opts.chance == 1 then
		answering.m_lastConversationAnswerPercent = 999
	end

	return resp
end

function _M:load()
	opts = self
	self.key = self.key or 1
	self.everyone = self.everyone or 0
	self.chance = self.chance or 0
end

function _M:unload()
end

function _M:config()
	log("config geass")
	if not self then
		log("not having self?")
		return
	end
	require "iuplua"
	require "iupluacontrols"

	local ok, newkey, everyone, chance = iup.GetParam("Set the activation key for Geass", nil, [[
Activation Key: %l|Left Mouse Button|Right Mouse Button|Middle Mouse Button|Shift Key|Control Key|Alt Key|
Geass for everyone: %b
Force 999%% chance: %b
]], self.key, self.everyone, self.chance)
	if ok then
		self.key = newkey
		self.everyone = everyone
		self.chance = chance
	end

	Config.save()
end

return _M
