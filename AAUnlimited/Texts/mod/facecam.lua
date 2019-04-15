--@INFO POV facecam (camera tracks eye bones)

local c = require "const"
local _M = {}
local cfg -- config for positions
local mcfg -- module config

-- default offset
local def = {
	x = 0,
	y = 0,
	z = 0
}

local INS = 96
local DEL = 110

local MIN = 109
local PLUS = 107

local DIV = 111
local MUL = 106

local SEVEN = 103

--local
hinfo = hinfo or false
local parts = {}

local tohide = { c.FACE, c.HAIR_FRONT, c.HAIR_SIDE, c.HAIR_BACK, c.HAIR_EXT, c.FACE_SLIDERS, c.GLASSES}

local current = nil
local eye = nil
local center = {x=0,y=0,z=0}
local xyz
local backupcamera = {rotx=0,roty=0,rotz=0,rotdist=0,fov=1}
local changehfov = 1

--local
function fetch_rot()
	if not hinfo then return end
	-- must be in facecam mode to fetch something meaningful!
	if current == nil then return end
	--log.info("-LUA-fetch_rot")
	local cam = hinfo:GetCamera()
	xyz.xrot = cam.m_xRotRad
	xyz.yrot = cam.m_yRotRad
	xyz.zrot = cam.m_zRotRad
end

--local
function load_hpos_settings()
	if not hinfo then return end
	if current == nil then return end
	--log.info("-LUA-load_hpos_cfg")
	local pos = tostring(hinfo.m_currPosition) .. ((current and "_active") or "_passive")
	xyz = cfg[pos] or {x=def.x,y=def.y,z=def.z,xrot=0,yrot=math.pi,zrot=0}
	cfg[pos] = xyz
end

--local
function update_eye()
	--log.info("-LUA-update_eye")
	SetFocusBone(eye, xyz.x + center.x, xyz.y + center.y, xyz.z + center.z, mcfg.zunlock)
	local cam = hinfo:GetCamera()
	cam.m_xRotRad = xyz.xrot
	cam.m_yRotRad = xyz.yrot
	cam.m_zRotRad = xyz.zrot
end

--local
function restore_camera()
	--log.info("-LUA-restore_camera")
	SetFocusBone(nil)	
	local cam = hinfo:GetCamera()
	for i,v in ipairs {1.0,0,0,0,0,1.0,0,0,0,0,1.0,0,0,0,0,1.0} do
		cam:m_matrix(i-1, v)
	end
	cam.m_xRotRad = backupcamera.rotx -- restore also Camera FOV, Rotatings and Distance
	cam.m_yRotRad = backupcamera.roty
	cam.m_zRotRad = backupcamera.rotz
	cam.m_distToMid = backupcamera.rotdist
	cam.m_fov = backupcamera.fov
end


-- find eyes center
--local
function set_eye_focus(who)
	--log("set eye", who)
	if who == nil then
		restore_camera()
		return
	end
	local cptr = parts[who and 1 or 2]
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
	center = {
		x=x,
		y=y,
		z=z
	}
	eye = cptr:m_bonePtrArray(0)
	update_eye()
end

--local
function hide_heda(who, hide)
	--log("hide heda %s %s", who, hide)
	local flag = hide and 2 or 0
	local cptr = parts[who and 1 or 2]
	for _,v in ipairs(tohide) do
		pcall(function() cptr:GetXXFile(v).m_root:m_children(0):m_children(0).m_renderFlag = flag end)
	end
end

--local
function set_status(current)
	if not hinfo then return end
	--log("set_status", current)
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
	if chr == mcfg.activate then
		fetch_rot()
		if current == nil then -- On activate facecam
			current = true
			-- backup the Camera FOV, Rotatings and Distance and set the starting FOV
			local cam = hinfo:GetCamera()
			backupcamera = {
				rotx=cam.m_xRotRad,
				roty=cam.m_yRotRad,
				rotz=cam.m_zRotRad,
				rotdist=cam.m_distToMid,
				fov=cam.m_fov
			}
			cam.m_fov = mcfg.startfov
		else
			current = not current
		end
	elseif mcfg.reset:find(chr,1,true) and current ~= nil then
		fetch_rot()
		current = nil
	else
		return k
	end
	-- save previous pos settings
	Config.save()
	load_hpos_settings()
	set_status(current)
	return k
end

function on.keydown(k)
	if (not hinfo) or (current == nil) then return k end
	local step = mcfg.step
	if k == PLUS then
		xyz.y = xyz.y - step
	elseif k == MIN then
		xyz.y = xyz.y + step

	elseif k == DEL then
		xyz.x = xyz.x - step
	elseif k == INS then
		xyz.x = xyz.x + step

	elseif k == DIV then
		xyz.z = xyz.z - step
	elseif k == MUL then
		xyz.z = xyz.z + step

	elseif k == SEVEN then
		xyz.x = 0
		xyz.y = 0
		xyz.z = 0
		xyz.xrot = 0
		xyz.yrot = math.pi
		xyz.zrot = 0
		-- note, no fetch_rot coz we force 0
		update_eye()
		return k
	else return k end
	--log("keydown! %f %f %f", xyz.x, xyz.y, xyz.z)
	fetch_rot()
	update_eye()
	return k
end

-- this fires after the pos is actually changed
function on.change_h(hi, currpos, active, passive, aface, pface)
	Config.save()
	if current ~= nil then
		--log.info("-LUA-change h!")
		--[[
			fetch_rot() ??
			load_hpos_settings() ??
			]]
		set_status(current)
		hinfo:GetCamera().m_fov = changehfov -- restore the FOV
	end
end

function on.start_h(hi)
	hinfo = hi
	parts[1] = hinfo.m_activeParticipant.m_charPtr
	parts[2] = hinfo.m_passiveParticipant.m_charPtr
end


--[[function on.hipoly()
	if not hinfo then return end
	set_status(current)
end]]

local function kill_h()
	if not hinfo then return end
	fetch_rot()
	Config.save()
	set_status(nil)
	hinfo = false
end

function on.end_h()
	kill_h()
end

function on.convo()
	kill_h()
end

local function on_hposchange(arg)
	changehfov = hinfo:GetCamera().m_fov   -- backup FOV val for 'after changed H pose'
	fetch_rot()
	Config.save()
--	log("injected hpos change! %d", arg)
	return arg
end

local function on_hposchange2(arg)
	load_hpos_settings()
--	log("injected hpos change2! %d", arg)
	return arg
end

local orig_hook
-- support reloading
function _M:load()
	assert(self)
	mcfg = self
	cfg = self.cfg or {}
	self.cfg = cfg
	self.activate = self.activate or 'a'
	self.reset = self.reset or 'qwer'
	self.zunlock = self.zunlock or false
	self.step = self.step or 0.05
	self.startfov = self.startfov or 1.2
	-- tu, hackanon
	orig_hook = g_hook_vptr(0x326FC4, 1, function(orig, this, arg)
		arg = on_hposchange(arg)
		local ret = proc_invoke(orig, this, arg)
		log("(tbd generalize this inject) hpos change %x %x %d", this, arg, ret)
		ret = on_hposchange2(ret)
		return ret
	end)
end

function _M:unload()
	g_poke_dword(0x326FC4, orig_hook)
	Config.save()
end

function _M:config()
	require "iuplua"
	require "iupluacontrols"
	local okay, akey, rkeys, z, stfov, step = iup.GetParam("Configure Facecam", nil, [[
Activate key: %s{alphanum key to enter facecam}
Reset keys: %s{any of these keys reset facecam}
Allow Roll (Z) axis: %b{Descent mode}
Starting FOV: %r[0.1,1.5,0.1]{Field of view on Activating Facecam}
Offset step: %r[0.01,0.5,0.01]{Eye offset. Use numpad Ins,Del,+,-,*,/}
]], self.activate, self.reset, self.zunlock and 1 or 0, self.startfov, self.step)
	if okay then
		self.activate = akey
		self.reset = rkeys
		self.zunlock = z == 1
		self.startfov = stfov
		self.step = step
		set_status(current)
		Config.save()
	end
end

return _M