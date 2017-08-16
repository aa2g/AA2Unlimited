--@INFO Absolute Obedience Cheat

local _M = {}

local keys = { 0x02, 0x04, 0x10, 0x11, 0x12 }
local opts = {
	{"key", 1, "Activation Key: %l|None|Right Mouse Button|Middle Mouse Button|Shift Key|Control Key|Alt Key|"},
	{"everyone", 0, "Geass for everyone: %b"},
	{"chance", 0, "Force 999%% chance: %b"},
}

function on.answer(resp, as)
	-- weird undocumented states
	if resp > 1 then
		info("GEASS: Unknown response", resp)
		return resp
	end

	local ispc = as.askingChar.m_thisChar == GetPlayerCharacter()
	local active = is_key_pressed(keys[opts.key])

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
	mod_load_config(self, opts)
end

function _M:unload()
end

function _M:config()
	mod_edit_config(self, opts, "Set geass options")
end

return _M
