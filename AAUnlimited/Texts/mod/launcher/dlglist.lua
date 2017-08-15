require "iuplua"
require "iupluacontrols"

return function(handlers, list)
	local enable = iup.button{title="Enable", active=false}
	local disable = iup.button{title="Disable", active=false}
	local up = iup.button{title="Move up", active=false}
	local down = iup.button{title="Move down", active=false}
	--[[local add = iup.button{title="Add"}
	local del = iup.button{title="Delete", active=false}]]
	local edit, reload, config


	local selected
	local selected_idx

--[[	local function config_buttons()
		if list.count == 0 then
			enable.active = "no"
			disable.active = "no"
			up.active = "no"
			down.active = "no"
			--del.active = "no"
			list.active = "no"
			if handlers.edit then
				edit.active = "no"
				edit.reload = "no"
			end
		else
			enable.active = "yes"
			disable.active = "yes"
			up.active = "yes"
			down.active = "yes"
			--del.active = "yes"
		end
	end]]

	function list:dblclick_cb(idx,text)
--		idx = tonumber(idx)
		text = text or self[idx]
		local state
		self[idx], state = handlers:toggle(text or self[idx])
		list.value = selected_idx
		disable.active = state and "yes" or "no"
		enable.active = state and "no" or "yes"
	end

	function list:k_any(c)
		if c == iup.K_SP then
			list:dblclick_cb(selected_idx, selected)
			return iup.IGNORE 
		end
		return iup.CONTINUE
	end

	function list:action(text,idx,state)
--		config_buttons()
		idx = tonumber(idx)
		if state == 0 then return end
		text = text or self[idx]
		selected = text
		selected_idx = idx
		local is_active, canreload, canconfig = handlers:select(text)
		disable.active = is_active and "yes" or "no"
		enable.active = is_active and "no" or "yes"
		if reload then
			reload.active = canreload and "yes" or "no"
		end
		if config then
			config.active = canconfig and "yes" or "no"
		end
	end


	--[[function del:action()
		handlers:delete(selected)
		list.removeitem = selected_idx
		selected = list[selected_idx] or list[selected_idx-1]
		config_buttons()
		iup.SetFocus(list)
	end
	function add:action()
		local it = handlers:add()
		list.appenditem = handlers:item(it, true)
		handlers:enable(it)
		iup.SetFocus(list)
	end]]
	function disable:action()
		list[selected_idx] = handlers:toggle(selected, false)
		list.value = selected_idx
		disable.active = "no"
		enable.active = "yes"
		iup.SetFocus(list)
	end
	function enable:action()
--		handlers:enable(selected)
		list[selected_idx] = handlers:toggle(selected, true)
		list.value = selected_idx
		enable.active = "no"
		disable.active = "yes"
		iup.SetFocus(list)
	end
	function up:action()
		if selected_idx > 1 then
			local n = selected_idx - 1
			handlers:swap(n,selected_idx)
			list[n], list[selected_idx] = list[selected_idx], list[n]
			selected_idx = n
			list.value = selected_idx
		end
	end
	function down:action()
		if selected_idx < tonumber(list.count) then
			local n = selected_idx + 1
			handlers:swap(n,selected_idx)
			list[n], list[selected_idx] = list[selected_idx], list[n]
			selected_idx = n
			list.value = selected_idx
		end
	end
	if handlers.edit then
		edit = iup.button{title="Edit / Debug",active="no"}
		function edit:action()
			handlers:edit(selected)
		end
		reload = iup.button{title="Reload", active="no"}
		function reload:action()
			handlers:reload(selected)
		end
	end

	if handlers.config then
		config = iup.button{title="Settings", active="no"}
		function config:action()
			handlers:config(selected)
		end
	end

	list.expand = true
	return iup.hbox {
		list, 
		iup.vbox {
			normalizesize="HORIZONTAL",
			enable,
			disable,
			up,
			down,
			--[[add,
			del,]]
			edit,
			reload,
			config
		}
	}
end


