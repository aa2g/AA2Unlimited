--@INFO Cute girls (male)

local _M = {}

local save = {}

local function patch(addr, bytes)
	save[addr] = g_peek(addr, #bytes)
	g_poke(addr, bytes)
end

local function checks()
end

function _M:load()
	if exe_type ~= "play" then return end
	patch(0x7D8DE, "\x39\xc0\x90") -- male check for ??
	patch(0x7A2C4, "\xff") -- m/f check for ??
	patch(0x7FBBF, "\xff") -- m/f check for ??
	patch(0x79280, "\x00") -- m/f check for ??
	patch(0x83191, "\x00") -- m/f check for ??
	patch(0x8BC40, "\x00") -- m/f check for ?? H expressions? H_Expression_State.lst not H_Expression_Male.lst
	patch(0x957BB, "\xff") -- m/f check for ?? expressions/sound again ??
	patch(0x96412, "\x00") -- m/f check for ?? 
end

function _M:unload()
	for k,v in ipairs do
		g_poke(k,v)
	end
end

return _M
