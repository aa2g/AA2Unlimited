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

function setfocus(part)
	if part == nil then
		SetFocusBone(nil, 0, 0, 0)
		return
	end
	local part = part and hinfo.m_activeParticipant or hinfo.m_passiveParticipant
	local cptr = part.m_charPtr
	local face = cptr:GetXXFile(0)
	local left = face:FindBone("A00_J_meL2",-1)
	local right = face:FindBone("A00_J_meR2",-1)
	local lx, ly, lz = 0,0,0
	local rx, ry, rz = 0,0,0
	print("got left eye", left)
	while left ~= nil do
		lx = lx + left:GetMatrix(4):get(4,1)
		ly = ly + left:GetMatrix(4):get(4,2)
		lz = lz + left:GetMatrix(4):get(4,3)
		left = left.m_parent
	end

	print("got right eye", right)
	while right ~= nil do
		rx = lx + right:GetMatrix(4):get(4,1)
		ry = ly + right:GetMatrix(4):get(4,2)
		rz = lz + right:GetMatrix(4):get(4,3)
		right = right.m_parent
	end

	local x,y,z = rx - lx, ry - ly, rz - lz
	x = x/2 + lx
	y = y/2 + ly
	z = z/2 + lz
	print("got ",cptr:GetBone(0))
	SetFocusBone(cptr:GetBone(0), x, y, z)
end

function on.end_h()
	hinfo = false
end


function _M.load()
end

function _M.unload()
end

return _M