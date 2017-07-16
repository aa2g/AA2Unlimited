function peek_dword(addr)
	return string.unpack("<I4", _BINDING.peek(addr, 4))
end

function poke_dword(addr, d)
	return _BINDING.poke(addr, string.pack("<I4", d))
end


function g_wrap(f)
	assert(f)
	return function(addr, ...)
		return f(addr + GameBase, ...)
	end
end

function xchg_dword(addr, d)
	local orig = peek_dword(addr)
	poke_dword(addr, d)
	return orig
end

g_peek = g_wrap(_BINDING.peek)
g_poke = g_wrap(_BINDING.poke)
g_peek_dword = g_wrap(peek_dword)

g_poke_dword = g_wrap(poke_dword)
g_xchg_dword = g_wrap(xchg_dword)

_EVENTS.callback = {}

local dbg = "\x90"
--dbg="\xcc"
local asm = 
	dbg .. "\x8d\x44\x24\x04\x53\x51\x56\x57"
local asm2 =
	"\x50\x51\xe8\x08\x00\x00\x00\x5f\x5e\x59\x5b"..dbg.."\xc2"

local cidx = 0

local function push(dd)
	return "\x68" .. string.pack("<I4", dd)
end

-- create callback and returns pointer to it
function make_callback(f, nargs)
	cidx = cidx+1
	local page = x_pages(4096)
	local buf =
		asm .. push(cidx) .. push(nargs) ..
		asm2 .. string.pack("<I2", nargs*4) ..
		push(_BINDING.callback) .. "\xc3"
	poke(page, buf)
	_EVENTS.callback[cidx] = f
	return page
end

function hook_vptr(addr, nargs, f)
	local orig = peek_dword(addr)
	local ff = function(...)
		return f(orig, ...)
	end
	poke_dword(addr, make_callback(ff, nargs))
	return orig
end

g_hook_vptr = g_wrap(hook_vptr)
