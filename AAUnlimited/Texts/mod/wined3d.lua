--@INFO Opengl renderer (needs game restart)

local _M = {}

local function setvar(var,val)
	SetEnvironmentVariableA(var,val)
	log("Setting WineD3D variable %s=%s",var,val)
end

function on.d3d9_preload(prev)
	setvar("csmt", tostring(opts.csmt))
	setvar("WINED3D_CS_QUERY_POLL_INTERVAL", tostring(opts.csmt_poll))
	setvar("WINED3D_CS_QUEUE_SIZE", tostring((256*1024) << opts.csmt_queue))
	setvar("WINED3D_CS_SPIN_COUNT", tostring(opts.csmt_spin))

	if opts.backbuffer == 1 then
		setvar("OffscreenRenderingMode", "backbuffer")
	end

	if opts.aa > 0 then
		local aav = {2,4,8,16,32}
		local sc = tostring(aav[opts.aa])
		setvar("SampleCount", sc)
	end
	if prev ~= 0 then
		alert("wined3d", "D3D conflict, this one must be first one to load and is mutually exclusive with win10fix")
		os.exit(1)
	end
	return LoadLibraryA("wined3d9.dll")
end

function _M:load()
	opts = self
	self.backbuffer = self.backbuffer or 0
	self.aa = self.aa or 0
	self.csmt = self.csmt or 1
	self.csmt_spin = self.csmt_spin or 10000000
	self.csmt_queue = self.csmt_queue or 2
	self.csmt_poll = self.csmt_poll or 10

end

function _M:unload() end

function _M:config()
	if not self then return end
	require "iuplua"
	require "iupluacontrols"

	local ok,
	bk,
	aa,
	csmt,
	csmt_queue,
	csmt_spin,
	csmt_poll =
	
	iup.GetParam("Configure WineD3D", nil, [[
Backbuffer: %b{Sometimes needed for GPU overrides to work}
Antialiasing: %l|None|2xMSAA|4xMSAA|8xMSAA|16xMSAA|32xMSAA|
CSMT threading: %b{enables wine command stream multithreading}
CSMT settings %t
Queue size: %l|256KB|512KB|1MB|2MB|4MB|8MB|16MB|32MB|
Spin CPU cycles: %i[100000,100000000]{Spin CPU waiting for commands if queue is empty}
Poll after spins: %i[1,100000]{Polls queue for new command after this much spins}
]],
self.backbuffer,
self.aa,
self.csmt,
self.csmt_queue,
self.csmt_spin,
self.csmt_poll

)
	if ok then
		self.backbuffer = bk
		self.csmt = csmt
		self.aa = aa
		self.csmt_spin = csmt_spin
		self.csmt_queue = csmt_queue
		self.csmt_poll = csmt_poll
	end


	Config.save()
end


return _M