--@INFO Cute girls (male)

local _M = {}

function _M:load()
--[[
.text:0047A2AF                 cmp     byte ptr [edx+40h], 0
.text:0047A2B3                 mov     [esp+2E9Ch+var_2E84], eax
.text:0047A2B7                 jnz     loc_47A3AD
.text:0047A2BD                 mov     ecx, [esp+2E9Ch+var_2E88]
.text:0047A2C1                 cmp     byte ptr [ecx+40h], 0 --> 2
.text:0047A2C5                 jnz     loc_47A3AD
.text:0047A2CB                 mov     eax, [edi+9Ch]
.text:0047A2D1                 mov     edx, ecx
.text:0047A2D3                 mov     ecx, [esp+2E9Ch+var_2E70]
]]
	g_poke(0x7A2C4, "\x02") -- male check

--[[
  grill = *(a1 + 76) == 1
  (*(void (__stdcall **)(_DWORD, _DWORD, signed int))(grill, 0, -1);

  rewrites that to just
  grill = 1
  (*(void (__stdcall **)(signed int, _DWORD, signed int))(**(_DWORD **)(a1 + 1556) + 16))(grill, 0, -1);
]]
	g_poke(0x7D8DD, "\xb8\x01\x00\x00\x00\x90\x90") -- mov eax, 1

--[[
.text:0047FBBB                 cmp     [esp+esi+48h+var_14], 0 --> 2
.text:0047FBC0                 mov     ecx, [edi]
.text:0047FBC2                 mov     eax, [ecx]
]]
	g_poke(0x7FBBF, "\x02") -- yet another male check

	-- Doesn't seem to do anything: 0008D333 8D88E400000005E00000003BC1→8B88E00000008B098079400074

	g_poke(0x79280, "\x00") -- nop jmp; male check
	g_poke(0x83191, "\x00") -- nop jmp

	g_poke(0x8BC40, "\x00") -- male check for H expressions, ie read from H_Expression_State.lst not H_Expression_Male.lst


--[[
    if ( result != -1
      && *(_BYTE *)(**(_DWORD **)((char *)&v64 + 4 * v56 + 224 - (_DWORD)&v64 + *(_DWORD *)(v1 + 8)) + 64) != 0 )
]]
	g_poke(0x94BBB, "\x02") -- again male check, seems like for pitch?

--[[
	again a male check for god knows what
	removes the commented out code
    /*if ( *(_BYTE *)(**(_DWORD **)(v8 + *(_DWORD *)(v13 + 8)) + 64) == 1 )*/
      result = sub_49AE40(v11, 0, *(_BYTE *)(v13 + v5 + 44) != 0 ? 2 : 0);
    /*else
      result = 0;*/
]]

	-- avoid null deref in dynamic_cast<GirlChara>
	-- we simply don't do the dynamic cast, since the the subclass appears to be
	-- plain subclass, not sideways
	g_poke(0x8E122, "\x89\xc8\xeb\x01")
end

return _M
