--@INFO Implements POV facecam (camera tracks eye bones)

local _M = {}

hinfo = hinfo or false

local tohide = { 0, 3, 4, 5, 6, 10, 11}

function on.keydown(k)

	print("got2 key", k)

	if not hinfo then return k end
	if key == 84 then return -1 end
	return k
end

function on.start_h(hi)
	hinfo = hi
end
--hinfo.m_passiveParticipant.m_charPtr:GetXXFile(1).m_root:GetChild(0):GetChild(0).m_renderFlag  = 0
function hideshow(part, flag)
	flag = flag and 2 or 0
	local part = part and hinfo.m_activeParticipant or hinfo.m_passiveParticipant
	local cptr = part.m_charPtr
	for _,v in ipairs(tohide) do
		pcall(function() cptr:GetXXFile(v).m_root:GetChild(0):GetChild(0).m_renderFlag = flag end)
	end
end

function on.end_h()
	hinfo = false
end


function _M.load()
end

function _M.unload()
end

return _M