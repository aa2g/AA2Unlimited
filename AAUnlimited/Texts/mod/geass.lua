--@INFO Reimplements frontier pack cheats (right click for geass)

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

return _M