--@INFO Absolute Obedience Cheat

local _M = {}

local keys = { 0x01, 0x02, 0x04, 0x10, 0x11, 0x12 }
local opts

function on.answer(resp, as)
	-- weird undocumented states
	if resp > 1 then
		info("GEASS: Unknown response", resp)
		return resp
	end

	local ispc = as.askingChar.m_thisChar == GetPlayerCharacter()
	local active = (GetAsyncKeyState(keys[opts.key + 1]) & 0x8000) ~= 0

	if active and (opts.everyone == 1 or ispc) then
		resp = 1
		info("GEASS: forcing NPC answer to yes")
		if opts.chance == 1 then
			as.answerChar.m_lastConversationAnswerPercent = 999
			info("GEASS: forcing NPC answer chance to 999%%")
		end
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
