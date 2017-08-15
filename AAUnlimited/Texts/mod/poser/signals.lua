local _M = {}

local connectors = {}

local function newmetaconnector(self)
	local metaconnector = {
		__call = function(_, ...)
			print("signal fired")
			for k,v in pairs(self) do
				for _,slot in ipairs(v) do
					k[slot](...)
				end
			end
		end,
		
		connect = function(target, slot)
			local listslots = rawget(self, target)
			if not listslots then
				self[target] = {}
			end
			for _,s in ipairs(self[target]) do
				if slot == s then
					print("signal is already connected to slot")
					return
				end
			end
			table.insert(self[target], slot)
		end
	}
	metaconnector.__metatable = metaconnector
	metaconnector.__index = metaconnector
	return metaconnector
end


function _M.connect(object, signal, target, slot)
	local connector
	local signalfunction = object[signal]
	if not signalfunction then
		connector = {}
		setmetatable(connector, newmetaconnector(connector))
		signalfunction = function(...) connector(...) end
		connectors[signalfunction] = connector
		object[signal] = signalfunction
	end
	connector = connector or connectors[signalfunction]
	if not connector then
		print("can't connect signal: callback slot is already assigned")
		return
	end
	connector.connect(target, slot)
end

return _M
