local _M = {}

local signals = require "poser.signals"

function _M.listbox(opts)
	local opts = opts or {}
	local entries = {}
	local currentfilter
	
	local updatelist
	
	local selectionchanged = signals.signal()
	
	local function setlist(newlist)
		entries = newlist
		updatelist()
	end
	
	local function setfilter(filter)
		if not filter or string.len(filter) == 0 then
			currentfilter = nil
		else
			currentfilter = string.lower(filter)
		end
		updatelist()
	end
	
	local list = iup.list {
		visiblelines = opts.lines or 12,
		sort = opts.sort or "no",
		editbox = opts.editbox or "no",
		expand = opts.expand or "yes",
		setlist = setlist,
		setfilter = setfilter,
		selectionchanged = selectionchanged,
		valuechanged_cb = function(self)
			local index = self.value
			if index and index ~= 0 then
				selectionchanged(self[index], index)
			end
		end,
	}
	
	updatelist = function()
		local newlist
		if currentfilter then
			newlist = {}
			for _,v in ipairs(entries) do
				if string.find(string.lower(v), currentfilter, 1, true) then
					table.insert(newlist, v)
				end
			end
		else
			newlist = entries
		end
		list[1] = nil
		for i,v in ipairs(newlist) do
			list[i] = v
		end
	end

	function list.select(val)
		for i,v in ipairs(entries) do
			if v == val then
				list.value = i
			end
		end		
	end
	
	return list
end

function _M.listfilter()
	local setfilter = signals.signal()
	
	local text = iup.text { 
		cuebanner = "Search...",
		expand = "horizontal",
	}
	
	function text.valuechanged_cb(self)
		setfilter(self.value)
	end

	function text.clear()
		text.value = ""
		setfilter()
		iup.SetFocus(text)
	end
	
	local clear = iup.flatbutton { title = "X", font = "Serif, Courier, 8", size = "15x10", border = "yes" }
	signals.connect(clear, "flat_action", text, "clear")
	
	return iup.hbox {
		text,
		clear,
		alignment = "acenter",
		setfilter = setfilter,
	}
end

return _M
