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
	3,
	6,

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


local _M = {}
local vtaddr
local backup

local exempts = {
	[11]=true,
	[12]=true,
}
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
		g_hook_vptr(vtaddr + i*4, arity, function(orig, this, ...)
			local argstr = table.concat({...}, ",")
			if not exempts[i] then
				log.info("female vtable %x.%s(%s) " % {this,names[i+1] or i, argstr})
			end
			--if i == 9 then return end
			return proc_invoke(orig, this, ...)
		end)
	end
end

function _M:unload()
	g_poke(vtaddr, backup)
end
return _M