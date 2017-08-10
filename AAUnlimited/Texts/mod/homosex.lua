--@INFO Cute girls (male)

local _M = {}

local mcfg

local save = {}

local function patch(addr, bytes)
	save[addr] = g_peek(addr, #bytes)
	--log("poke %x %d", addr, #bytes)
	g_poke(addr, bytes)
end

local function checks()
end

local function do_patch()
	if exe_type ~= "play" then return end
	local patches = {
		{0x7D8DE, "\x39\xc0\x90"},
		{0x7A2C4, "\xff"},
		{0x7FBBF, "\xff"},
		{0x79280, "\x00"},
		{0x83191, "\x00"},
		{0x8BC40, "\x00"},
		{0x957BB, "\xff"},
		{0x96412, "\x00"},
	}
	for idx,v in ipairs(patches) do
		if mcfg.opts[idx] == 1 then
			patch(v[1], v[2])
		end
	end
end

local function do_unpatch()
	for k,v in ipairs(save) do
		--log("unpoke %x %d", k, #v)
		g_poke(k,v)
	end
	save = {}
end

function _M:load()
	assert(self)
	mcfg = self
	mcfg.opts = mcfg.opts or {1,1,1,1,1,1,1,1}
	do_patch()
end

function _M:config()
	if not self then return end
	require "iuplua"
	require "iupluacontrols"

	local opts = {iup.GetParam("Configure Homosex", nil, [[
opt1: %b
opt2: %b
opt3: %b
opt4: %b
opt5: %b
opt6: %b
opt7: %b
opt8: %b
]], table.unpack(self.opts))}
	if not opts[1] then return end
	do_unpatch()
	self.opts = {table.unpack(opts, 2)}
	do_patch()
	Config.save()
end

function _M:unload()
	do_unpatch()
end

return _M
