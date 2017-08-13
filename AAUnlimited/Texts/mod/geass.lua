--@INFO Absolute Obedience Cheat

local _M = {}

function on.answer(resp, answering, asking)
	if asking.m_thisChar == GetPlayerCharacter() then
		-- right mouse button is down
		if (GetAsyncKeyState(2) & 0x8000) ~= 0 then
			print("GEASS: forcing NPC answer to yes")
			return true
		end
	end
	return resp
end

function _M.load()
end

function _M.unload()
end

return _M
