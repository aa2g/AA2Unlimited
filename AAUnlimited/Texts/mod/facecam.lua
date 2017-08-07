--@INFO Implements POV facecam (camera tracks eye bones)

local _M = {}
local cfg

-- key controls
local UP = 1
local DOWN = 0
local LEFT = 0
local RIGHT = 0

local PGUP = 0
local PGDN = 0

local activate = 'a'
local reset = 'r'

--local
hinfo = hinfo or false

local tohide = { 0, 3, 4, 5, 6, 10, 11}

local current = nil
local eye = nil
local center = {0,0,0}
local xyz = {0,0,0,xrot=0,yrot=0}

--local
function update_eye()
	SetFocusBone(eye, xyz[1] + center[1], xyz[2] + center[2], xyz[3] + center[3])
	local cam = hinfo:GetCamera()
	cam.m_xRotRad = xyz.xrot
	cam.m_yRotRad = xyz.yrot
end

-- find eyes center
--local
function set_eye_focus(who)
	if who == nil then
		update_eye()
		return
	end
	local part = who and hinfo.m_activeParticipant or hinfo.m_passiveParticipant
	local cptr = part.m_charPtr
	local face = cptr:GetXXFile(0)
	local left = face:FindBone("A00_J_meL2",-1)
	local right = face:FindBone("A00_J_meR2",-1)
	local lx, ly, lz = 0,0,0
	local rx, ry, rz = 0,0,0
	while left ~= nil do
		lx = lx + left:m_matrix5(3*4+0)
		ly = ly + left:m_matrix5(3*4+1)
		lz = lz + left:m_matrix5(3*4+2)
		left = left.m_parent
	end

	while right ~= nil do
		rx = rx + right:m_matrix5(3*4+0)
		ry = ry + right:m_matrix5(3*4+1)
		rz = rz + right:m_matrix5(3*4+2)
		right = right.m_parent
	end

	local x, y, z = rx - lx, ry - ly, rz - lz
	x = x/2 + lx
	y = y/2 + ly
	z = z/2 + lz
	center = {x,y,z}
	eye = cptr:m_bonePtrArray(0)
end

--local
function hide_heda(who, hide)
	log("hide heda %s %s", who, hide)
	local flag = hide and 2 or 0
	local part = who and hinfo.m_activeParticipant or hinfo.m_passiveParticipant
	local cptr = part.m_charPtr
	for _,v in ipairs(tohide) do
		pcall(function() cptr:GetXXFile(v).m_root:m_children(0):m_children(0).m_renderFlag = flag end)
	end
end

--local
function set_status(current)
	if current == nil then
		hide_heda(true,false)
		hide_heda(false,false)
		set_eye_focus(nil)
	else
		hide_heda(not current, false)
		hide_heda(current, true)
		set_eye_focus(current)
	end
end

function on.char(k)
	if not hinfo then return k end
	local chr = string.char(k)
	if chr == activate then
		current = not current
	elseif chr == reset then
		current = nil
	else
		return k
	end
	set_status(current)
	return k
end

function on.start_h(hi)
	hinfo = hi
end


function on.hipoly()
	if not hinfo then return end
	set_status(current)
end

function on.end_h()
	if not hinfo then return end
	set_status(nil)
	hinfo = false
end


-- support reloading
function _M.load()
	cfg = info.cfg or {}
	info.cfg = cfg
end

function _M.unload()
end

return _M