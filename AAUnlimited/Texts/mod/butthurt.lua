--@INFO Limit the rate of NPC interruptions


local _M = {}
local c = require 'const'

local opts = {
	{"cooldown", 30, "Cooldown seconds: %i[1,300]{Disable interruptions for this much seconds}"},
	{"stacking", 1, "Stacking cooldown: %b{Cooldowns will stack with each interupt}"},
	{"chancecd", 10, "Intr chance w/ cooldown: %i[0,100]{When cooldown is active, allow this % of interruptions}"},
	{"chancencd", 100, "Intr chance w/o cooldown: %i[0,100]{When cooldown is inactive, allow this % of interruptions}"},
}

local butthurt_actions = {
	[c.INTERRUPT_COMPETE] = true,
	[c.INTERRUPT_STOP_QUARREL] = true,
	[c.INTERRUPT_WHAT_ARE_YOU_DOING] = true,
	[c.BREAK_CHAT] = true,
	[c.BREAK_H] = true,
}

local convo_chars = {}

function on.char_spawn_end(status, char)
	info("butthurt: Adding convo char #%d" % char.m_seat)
	convo_chars[char] = true
	return status
end

function on.convo(state)
	if not state then
		info("butthurt: Resetting convo chars")
		convo_chars = {}
	end
end

function on.end_h()
	convo_chars = {}
end

local cooldown = 0
function on.plan(ok,e,who)
	if not ok then return end
	if not butthurt_actions[e.conversationId] then return ok end
	local target = (e.target1 or e.target2 or {}).m_thisChar or GetPlayerCharacter()
	if (target == GetPlayerCharacter()) or convo_chars[target] then
		info("butthurt: Character #%d got butthurt about us, target = %d" % {who.m_seat,target.m_seat})
		local now = os.time()
		local oldcd = cooldown
		if cooldown < now then
			cooldown = now
		end
		if opts.stacking == 1 then
			cooldown = cooldown + opts.cooldown
		else
			cooldown = now + opts.cooldown
		end
		local chance = opts.chancencd
		if oldcd > now then
			info("butthurt: cooldown active")
			chance = opts.chancecd
		end
		info("butthurt: New cooldown: %.2f seconds" % (cooldown - now))
		if (math.random() * 100) > chance then
			info("butthurt: cancelled, chance was %d" % chance)
			e.conversationId = -1
			ok = false
		end
	else
		info("butthurt: Character #%d got butthurt about somebody else, target = %d" % {who.m_seat,target.m_seat})
	end
	return ok
end


function _M:load()
	mod_load_config(self, opts)
end

function _M:config()
	mod_edit_config(self, opts, "Set butthurt limiter options")
end


function _M.unload()
end

return _M
