--@INFO POV facecam (camera tracks eye bones)

-- Problems to fix: 
-- 1) In `cumshot` part need restore_camera()  (non POV mode)
-- (Currently, is no way to track this `start_cumshot` events.)

local c = require "const"
local _M = {}
local cfg -- config for positions
local mcfg -- module config

-- default offset
local def = {
	x = 0,
	y = 0,
	z = 0,
	tongue = 2
}

local INS = 96
local DEL = 110

local MIN = 109
local PLUS = 107

local DIV = 111
local MUL = 106

local SEVEN = 103
local NINE = 105

local DecrFOV = 186 -- ;
local IncrFOV = 221 -- ]

--local
hinfo = hinfo or false
local parts = {}

local tohide = { c.FACE, c.HAIR_FRONT, c.HAIR_SIDE, c.HAIR_BACK, c.HAIR_EXT, c.FACE_SLIDERS, c.GLASSES}
local tonguePart = c.TONGUE

local current = nil
local eye = nil
local center = {x=0,y=0,z=0}
local xyz
local normalCamera = {rotx=0,roty=0,rotz=0,rotdist=0,fov=0.5}
local facecamFOV = 0.9
local prevPosId = 0

local function fetch_rot()
	if not hinfo then return end
	-- must be in facecam mode to fetch something meaningful!
	if current == nil then return end
	--log.info("-LUA-fetch_rot")
	local cam = hinfo:GetCamera()
	xyz.xrot = cam.m_xRotRad
	xyz.yrot = cam.m_yRotRad
	xyz.zrot = cam.m_zRotRad
end

local function load_hpos_settings()
	if not hinfo then return end
	if current == nil then return end
	--log.info("-LUA-load_hpos_cfg")
	local pos = tostring(hinfo.m_currPosition) .. ((current and "_active") or "_passive")
	xyz = cfg[pos] or {x=def.x,y=def.y,z=def.z,xrot=0,yrot=math.pi,zrot=0,tong=def.tongue}
	if not xyz.tong then -- fix for old-saved params of poses (without tong param)
		xyz.tong = def.tongue
	end 
	cfg[pos] = xyz
end

local function update_eye()
	--log.info("-LUA-update_eye")
	SetFocusBone(eye, xyz.x + center.x, xyz.y + center.y, xyz.z + center.z, mcfg.zunlock)
	local cam = hinfo:GetCamera()
	cam.m_xRotRad = xyz.xrot
	cam.m_yRotRad = xyz.yrot
	cam.m_zRotRad = xyz.zrot
end

local function restore_camera()
	--log.info("-LUA-restore_camera")
	SetFocusBone(nil)	
	local cam = hinfo:GetCamera()
	for i,v in ipairs {1.0,0,0,0,0,1.0,0,0,0,0,1.0,0,0,0,0,1.0} do
		cam:m_matrix(i-1, v)
	end
	cam.m_xRotRad = normalCamera.rotx -- restore also Camera FOV, Rotatings and Distance
	cam.m_yRotRad = normalCamera.roty
	cam.m_zRotRad = normalCamera.rotz
	cam.m_distToMid = normalCamera.rotdist
	set_FOV(normalCamera.fov)
end


-- find eyes center
local function set_eye_focus(who)
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

local function hide_heda(who, hide)
	--log("hide heda %s %s", who, hide)
	local flag = hide and 2 or 0
	local cptr = parts[who and 1 or 2]
	for _,v in ipairs(tohide) do
		pcall(function() cptr:GetXXFile(v).m_root:m_children(0):m_children(0).m_renderFlag = flag end)
	end
	-- Also hide a tongue, where it's need
	if who == current then
		pcall(function() cptr:GetXXFile(tonguePart).m_root:m_children(0):m_children(0).m_renderFlag = xyz.tong end)
	else
		pcall(function() cptr:GetXXFile(tonguePart).m_root:m_children(0):m_children(0).m_renderFlag = flag end)
	end
end

local function set_status(current)
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

function set_FOV(value) -- from 0.1 to 1.5
	g_poke(0x3A8574, string.pack("<f", value))
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
			normalCamera.rotx=cam.m_xRotRad
			normalCamera.roty=cam.m_yRotRad
			normalCamera.rotz=cam.m_zRotRad
			normalCamera.rotdist=cam.m_distToMid
			set_FOV(facecamFOV)
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
	if (not hinfo) then return k end
	if k == IncrFOV then
		if current ~= nil and facecamFOV <= 1.45 then
			set_FOV(facecamFOV + 0.05)
			facecamFOV = facecamFOV + 0.05
		elseif current == nil and normalCamera.fov <= 1.45 then
			set_FOV(normalCamera.fov + 0.05)
			normalCamera.fov = normalCamera.fov + 0.05
		end
		return k
	elseif k == DecrFOV then
		if current ~= nil and facecamFOV >= 0.15 then 
			set_FOV(facecamFOV - 0.05)
			facecamFOV = facecamFOV - 0.05
		elseif current == nil and normalCamera.fov >= 0.15 then 
			set_FOV(normalCamera.fov - 0.05)
			normalCamera.fov = normalCamera.fov - 0.05
		end
		return k
	end
	
	if (current == nil) then return k end
	
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
	elseif k == NINE then
		xyz.tong = xyz.tong == 2 and 0 or 2
		hide_heda(current,true)
	else return k end
	--log("keydown! %f %f %f", xyz.x, xyz.y, xyz.z)
	fetch_rot()
	update_eye()
	return k
end

-- this fires after the pos is actually changed
local function after_change_h()
	if current ~= nil then
		load_hpos_settings()
		--log.info("-LUA-after_change_h")
		set_status(current)
	end
end
function on.change_h(hi, currpos, active, passive, aface, pface)
	Config.save()
	after_change_h()
end

function on.start_h(hi)
	hinfo = hi
	parts[1] = hinfo.m_activeParticipant.m_charPtr
	parts[2] = hinfo.m_passiveParticipant.m_charPtr
	g_poke(0x3A8578, string.pack("<f", 0.1)) -- decrease `Near clipping distance`(=1) (for `Far`(=1000) use 0x3A857C)
end

function on.launch() -- Sending Motion amplitude to Camera
	local amplitudeval
	if mcfg.amplitudemov > 95 then 
		amplitudeval = mcfg.amplitudemov
	else
		amplitudeval = math.ceil(100 - math.sqrt(9050 - mcfg.amplitudemov^2))
	end
	InitPovParams(amplitudeval)
	-- to unlock `Near clipping distance` and prevent random changes the FOV by game in H scenes
	if exe_type == "play" then g_poke(0x80E9B, "\xEB\x10") end 
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
	current = nil
end

function on.end_h()
	kill_h()
end

function on.convo()
	kill_h()
end

local function on_hposchange(arg)
	fetch_rot()
	Config.save()
--	log("injected hpos change! %d", arg)
	return arg
end

local function on_hposchange2(arg, ret)
	--load_hpos_settings()
--	log("injected hpos change2! %d", arg)
	if prevPosId == arg then -- If user select again current H-pose
		--log.info("-LUA-hpos not changed")
		after_change_h()
	else
		prevPosId = arg
	end
	--log.info("-LUA-H pos id")
	--log.info(arg)
	return ret
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
	self.startfov = self.startfov or 90
	facecamFOV = self.startfov / 100
	self.amplitudemov = self.amplitudemov or 50
	self.step = self.step or 0.05
	self.tonguedef = self.tonguedef or false
	def.tongue = self.tonguedef and 0 or 2
	-- tu, hackanon
	orig_hook = g_hook_vptr(0x326FC4, 1, function(orig, this, arg)
		arg = on_hposchange(arg)
		local ret = proc_invoke(orig, this, arg)
		log("(tbd generalize this inject) hpos change %x %x %d", this, arg, ret)
		ret = on_hposchange2(arg, ret)
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
	local okay, akey, rkeys, z, stfov, amplitude, step, tonguedefault = iup.GetParam("Configure Facecam", nil, [[
Activate key: %s{alphanum key to enter facecam}
Reset keys: %s{any of these keys reset facecam}
Allow Roll (Z) axis: %b{Descent mode}
Starting FOV, degrees: %r[10,150,5]{Field of view on Activating Facecam}
Motion amplitude: %r[5,100,5]{How much will the camera follow head movements. Low values are recommended for greater stabilization}
Offset step: %r[0.01,0.5,0.01]{Eye offset. Use Numpad Ins,Del,+,-,*,/}
Show Tongue by default: %b{Show tongue for a managed character. Use Numpad 9 to toggle}
]], self.activate, self.reset, self.zunlock and 1 or 0, self.startfov, self.amplitudemov, self.step, self.tonguedef and 1 or 0)
	if okay then
		self.activate = akey
		self.reset = rkeys
		self.zunlock = z == 1
		self.startfov = stfov
		facecamFOV = stfov / 100
		self.amplitudemov = amplitude
		self.step = step
		self.tonguedef = tonguedefault == 1
		def.tongue = tonguedefault == 1 and 0 or 2
		set_status(current)
		Config.save()
	end
end

return _M
