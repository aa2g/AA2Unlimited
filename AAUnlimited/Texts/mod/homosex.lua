--@INFO Cute girls (male)

local _M = {}

local save = {}

local function patch(addr, bytes)
	save[addr] = g_peek(addr, #bytes)
	g_poke(addr, bytes)
end

function _M:load()
	patch(0x7A2C4, "\x02") -- m/f check for ??
	patch(0x7D8DD, "\xb8\x01\x00\x00\x00\x90\x90") -- male check for ??
	patch(0x7FBBF, "\x02") -- m/f check for ??

	-- linked list trickery to avoid assigning wrong H faces
	patch(0x8DF25, parse_asm[[
00000000  05E0000000        add eax,0xe0
00000005  8B4804            mov ecx,[eax+0x4]
00000008  8B09              mov ecx,[ecx]
0000000A  80794001          cmp byte [ecx+0x40],0x1
0000000E  7426              jz 0x36
00000010  8D4804            lea ecx,[eax+0x4]
00000013  8B31              mov esi,[ecx]
00000015  807E4000          cmp byte [esi+0x40],0x0
00000019  741B              jz 0x36
0000001B  39C8              cmp eax,ecx
0000001D  7409              jz 0x28
0000001F  FF30              push dword [eax]
00000021  FF31              push dword [ecx]
00000023  8F00              pop dword [eax]
00000025  8F01              pop dword [ecx]
00000027  90                nop
]])

		
--[[
original:
.text:0048DF25 8B 88 E4 00 00 00       mov     ecx, [eax+0E4h]
.text:0048DF2B 8B 09                   mov     ecx, [ecx]
.text:0048DF2D 80 79 40 01             cmp     byte ptr [ecx+40h], 1
.text:0048DF31 74 27                   jz      short loc_48DF5A
.text:0048DF33 8D 88 E4 00 00 00       lea     ecx, [eax+0E4h]
.text:0048DF39 05 E0 00 00 00          add     eax, 0E0h
.text:0048DF3E 3B C1                   cmp     eax, ecx
.text:0048DF40 74 0A                   jz      short loc_48DF4C
.text:0048DF42 8B 10                   mov     edx, [eax]
.text:0048DF44 8B 31                   mov     esi, [ecx]
.text:0048DF46 89 30                   mov     [eax], esi
.text:0048DF48 89 11                   mov     [ecx], edx
.text:0048DF4A 33 D2                   xor     edx, edx
-- past this point kept as-is
loc_48DF4C:
.text:0048DF4C 8B 00                   mov     eax, [eax]
.text:0048DF4E 89 50 08                mov     [eax+8], edx
.text:0048DF51 8B 09                   mov     ecx, [ecx]
.text:0048DF53 C7 41 08 01 00 00+      mov     dword ptr [ecx+8], 1
loc_48DF5A:
]]

	patch(0x79280, "\x00") -- m/f check for ??
	patch(0x83191, "\x00") -- m/f check for ??
	patch(0x8BC40, "\x00") -- m/f check for ?? H expressions? H_Expression_State.lst not H_Expression_Male.lst


	patch(0x957BB, "\x02") -- m/f check for ?? expressions/sound again ??
	patch(0x8E122, "\x89\xc8\xeb\x01") -- dynamic_cast<GirlClass> -> static_cast to prevent null deref
end

function _M:unload()
	for k,v in ipairs do
		g_poke(k,v)
	end
end

return _M
