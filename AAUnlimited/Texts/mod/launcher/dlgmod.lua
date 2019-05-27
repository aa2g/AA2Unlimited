require "iuplua"
require "iupluacontrols"

local unchecked = '☐ '
local checked = '☑ '
local chk = #checked + 1

local list = iup.list { expand="yes", font="Courier, 9", visiblelines = 20 }
local desc = iup.label { name="mod_desc", title="Description", expand="HORIZONTAL" }
local dlglist = require("launcher.dlglist")


local mod = {}

local function strip(n)
	return n:sub(chk):match("%S+")
end

function mod:item(n, desc, enabled)
	desc = desc:sub(1,40)
	return "%s%-10s %s" % {(enabled and checked or unchecked), n, desc }
end

function mod:select(n)
	local n = strip(n)
	local info, d = get_mod_info(n)
	desc.title = d
	return not info.disabled, module_can_unload(n), (module_is_loaded(n) or {}).config
end

--[[
function mod:delete(n)
	local n = strip(n)
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

function mod:toggle(n, tog)
	local n = strip(n)
	local m, desc = get_mod_info(n)
	if tog == true then
		m.disabled = nil
	elseif tog == false then
		m.disabled = true
	else
		m.disabled = not m.disabled
	end
	Config.save()
	if not m.disabled then
		if not module_is_loaded(n) then
			init_module(load_module(get_mod_info(n)))
		end
	else
		if module_can_unload(n) then
			unload_module(n)
		else
			alert(n, "For disable to take effect you need to restart the launcher.")
		end
	end

	return self:item(n, desc, (not m.disabled) and true or false), not m.disabled
end

function mod:swap(a,b)
	Config.mods[a], Config.mods[b] = Config.mods[b], Config.mods[a]
	Config.save()
end

function mod:edit(n)
end

function mod:reload(n)
	local n = strip(n)
	reload_module(n)
end

function mod:config(n)
	local n = strip(n)
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
			local mi, desc = get_mod_info(m[1])
			if mi then
				list[li] = mod:item(n, desc, not mi.disabled)
				li = li+1
			end
		end
	end
	list:action(nil, 1)
	list.value = 1
	return ret
end
