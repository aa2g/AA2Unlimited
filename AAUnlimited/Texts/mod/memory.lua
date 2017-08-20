function poke(...)
	local ret = _BINDING.poke(...)
	FlushInstructionCache(-1, 0, 0)
end

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

--[[
00000000  8D442404          lea eax,[esp+0x4]
00000004  53                push ebx
00000005  51                push ecx
00000006  56                push esi
00000007  57                push edi
00000008  68UUUUUUUU        push dword 0xUUUUUUUU ; callback index
0000000D  68WWWWWWWW        push dword 0xWWWWWWWW ; nargs
00000012  50                push eax
00000013  51                push ecx
00000014  E807000000        call dword 0x20
00000019  5F                pop edi
0000001A  5E                pop esi
0000001B  59                pop ecx
0000001C  5B                pop ebx
0000001D  C2YYYY            ret 0xYYYY            ; (nargs+1) * 4
00000020  68XXXXXXXX        push dword 0xXXXXXXXX ; lua dispatch handler
00000025  C3                ret
]]
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




function hook_func(addr, fixbytes, nargs, f)
	if type(fixbytes) == "number" then
		fixbytes = peek(addr, fixbytes)
	end
	assert(#fixbytes >= 6, "fixbytes must be at least 6, got " .. tostring(#fixbytes))
	local orig = x_pages(4096)
	poke(orig, fixbytes .. push(addr + #fixbytes) .. "\xc3")
	local ff = function(...)
		return f(orig, ...)
	end
	local cb = make_callback(ff, nargs)
	poke(addr, push(cb) .. "\xc3")
	return orig, fixbytes
end

function callto(addr, addr2)
	poke(addr, "\xe8" .. string.pack("<I", addr2-addr-5))
end

g_callto = g_wrap(callto)

g_hook_func = g_wrap(hook_func)

_alloced = 0
function malloc(n)
	_alloced = _alloced + n
	return _WIN32.LocalAlloc(0, n)
end

function free(m)
	_alloced = _alloced - _WIN32.LocalSize(m)
	_WIN32.LocalFree(m)
end


-- parses nasm output to opcodes
function parse_asm(s)
	local ret = s:gsub("%S+%s+(%S+)[^\n]*\n", function(r)
        return r:gsub("(..)", function(x)
                return string.char(tonumber(x, 16))
        end)
	end)
	log("asm parsed %d", #ret)
	return ret
end

function f_patch(bytes, offs)
	if offs >= 0 then
		offs = offs + 0xc00
	else
		offs = -offs
	end
	local save = g_peek(offs, #bytes)
	log.spam("f_patch %x, %s=> %s", offs, hexdump(save), hexdump(bytes))
	g_poke(offs, bytes)
	return save
end

function fixptr(p)
	if type(p) ~= "number" then
		p = tostring(p)
		local h = p:match(".-([0-9A-F]+)$")
		return tonumber(h, 16)
	end
	return p
end

function peek_walk(ptr,...)
	ptr = fixptr(ptr)
	for idx, off in ipairs {...} do
		ptr = peek_dword(ptr + off)
		if not ptr then break end
	end
	return ptr
end

function poke_walk(val,ptr,...)
	ptr = fixptr(ptr)
	local rules = {...}
	for idx, off in ipairs(rules) do
		if idx==#rules then
			return xchg_dword(ptr + off, val)
		end
		ptr = peek_dword(ptr + off)
		if not ptr then break end
	end
end