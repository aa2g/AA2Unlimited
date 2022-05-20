--@INFO Character mod experiments

local names = {
	"new",
	"load",
	"update",

	"unk3",
	"unk4",
	"unk5",
	"unk6",
	"unk7",
	"unk8",

	"init_skeletor"
	-- rest are mystery for now
}
local vtable = {
	1,--new
	4,--load
	2,--update

	0,
	0,
	0,

	0,
	0,
	0,

	5,
	0,
	3, -- 11 
	6, -- 12

	1,
	1,

	0,
	0,
	0,

	2,
	1,
	2,
	0
}

local origs = {

}


local _M = {}
local vtaddr
local backup

local exempts = {
	[11]=true,
	[12]=true,
	[20]=true,
}
local last = 0
local ticks = {}
function _M:load()
	if exe_type == "edit" then
		vtaddr = 0x30C324
	else
		vtaddr = 0x32D25C
	end
	backup = g_peek(vtaddr , #vtable * 4)
	log("hooked female vtable")
	for idx, arity in ipairs(vtable) do
		local i = idx-1
		origs[i] = g_hook_vptr(vtaddr + i*4, arity, function(orig, this, ...)
			local artab = {...}
			local argstr = table.concat(artab, ",")
			if (not exempts[i]) and artab[3] ~= 768 then
				log.spam("female vtable %x.%s(%s) " % {this,(names[i+1] or "" ).."#"..i, argstr})
			elseif true then
				local f = "vtable#%d(%x,%s)" % {i, this, argstr}
				if not ticks[f] then
					ticks[f] = 0
				end
				ticks[f] = ticks[f] + 1
				if last < os.time() then
					last = os.time() + 1
					print(p(ticks))
				end
			end
			if i == 9 then
--				log.spam('-> vtcall(GetPlayerCharacter(), 9, utf8_to_unicode(%q), utf8_to_unicode(%q), %d,0,0)',unicode_to_utf8(artab[1]).."\x00", unicode_to_utf8(artab[2]).."\x00", artab[3])
				log.spam('.Skeleton(%q, %q, %d)',unicode_to_utf8(artab[1]).."\x00", unicode_to_utf8(artab[2]).."\x00", artab[3])
				local rt = proc_invoke(orig, this, ...)
				log.spam("result = %x", rt)
				return rt
			end
--			if i == 12 then return 0 end -- mouth
--			if i == 11 then return 0 end -- animate
			return proc_invoke(orig, this, ...)
		end)
	end
end

function vtcall(obj,fn,...)
	return proc_invoke(origs[fn], obj, ...)
end

function _M:unload()
	g_poke(vtaddr, backup)
end
return _M