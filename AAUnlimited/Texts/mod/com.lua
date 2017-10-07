require "memory"

function retptr()
	local p = malloc(4)
	local val
	return {
		p = p,
		val = function(s)
			if val then return val end
			val = peek_dword(p)
			free(p)
			return val
		end
	}
end

function create_com_interface(fns)
	if true then return end
	local fns = {}
	local index = 0
	for f in fns:gmatch("%S+") do
		fns[f] = index
		index = index + 1
	end
	local tab = {}
	tab.__index = tab
	function tab:__call(obj)
		return setmetatable({obj=obj}, self)
	end
	function ret:__index(k)
		local vptr = peek(self.obj)
		local method = peek(vptr + fns[k] * 4)
		assert(method, "method "..k.. " not found")
		return function(o,...)
			return proc_invoke(method, 0, o.obj, ...)
		end
	end
	return setmetatable(tab, tab)
end


