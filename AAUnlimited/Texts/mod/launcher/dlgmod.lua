require "iuplua"
require "iupluacontrols"

local unchecked = '☐ '
local checked = '☑ '
local chk = #checked + 1

local list = iup.list { expand="yes" }
local desc = iup.label { name="mod_desc", title="Description", expand="HORIZONTAL" }
local dlglist = require("launcher.dlglist")


local mod = {}

function mod:item(n, enabled)
	return (enabled and checked or unchecked) .. n
end

function mod:select(n)
	local n = n:sub(chk)
	local info, d = get_mod_info(n)
	desc.title = d
	return not info.disabled, module_can_unload(n), (module_is_loaded(n) or {}).config
end

--[[
function mod:delete(n)
	local n = n:sub(chk)
	local i = 1
	while i < #Config.mods do
		if Config.mods[i][1] == n then
			table.remove(Config.mods, i)
		else
			i = i+1
		end
	end
	Config.save()
end

function mod:add()
	local pfx = aau_path("mod")
	local sel = iup.filedlg {directory=pfx, extfilter="AAU Lua script|*.lua", title="Select Lua script to add"}
	sel:popup()
	if sel.status ~= 0 then return end
	if (sel.value)
end
]]

function mod:disable(n)
	local n = n:sub(chk)
	if module_can_unload(n) then
		unload_module(n)
	else
		alert(n, "For disable to take effect you need to restart the launcher.")
	end

	get_mod_info(n).disabled = true
	Config.save()
	return unchecked .. n
end

function mod:enable(n)
	local n = n:sub(chk)
	if not module_is_loaded(n) then
		init_module(load_module(get_mod_info(n)))
	end
	get_mod_info(n).disabled = nil
	Config.save()
	return checked .. n
end

function mod:swap(a,b)
	Config.mods[a], Config.mods[b] = Config.mods[b], Config.mods[a]
	Config.save()
end

function mod:edit(n)
end

function mod:reload(n)
	local n = n:sub(chk)
	reload_module(n)
end

function mod:config(n)
	local n = n:sub(chk)
	local mi = module_is_loaded(n)
	if mi and mi.config then
		mi.config(mi.info)
	end
end


return function()
	local ret = iup.hbox {
		tabtitle = "Scripts",
		iup.vbox {
			dlglist(mod, list),
			desc
		}
	}
	local li = 1
	for i,m in ipairs(Config.mods) do
		if not m.hidden then
			local n = m[1]
			local mi = get_mod_info(m[1])
			if mi then
				list[li] = mod:item(n, not mi.disabled)
				li = li+1
			end
		end
	end
	list:action(nil, 1)
	list.value = 1
	return ret
end
