local _M = {}

-- lua function to connector map
local connectors = {}

local function newmetaconnector(self)
	local metaconnector = {
		__call = function(_, ...)
			print("signal fired")
			local target
			for k,v in pairs(self) do
				for _,slot in ipairs(v) do
					target = k[slot]
					if target.isconnector or type(target) == "function" then
						target(...)
					else
						--try setting the property
						k[slot] = ...
					end
				end
			end
		end,
		
		connect = function(target, slot)
			-- direct connection to function
			if not slot and type(target) == "function" then
				slot = target
				target = self
				self[slot] = { slot }
			else
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
		end,
		
		isconnector = true
	}
	metaconnector.__index = metaconnector
	return metaconnector
end


function _M.connect(object, signal, target, slot)
	local connector
	local signalfunction = object[signal]
	if not signalfunction then
		connector = { __mode = "k" }
		setmetatable(connector, newmetaconnector(connector))
		signalfunction = function(...) connector(...) end
		connectors[signalfunction] = connector
		object[signal] = signalfunction
	end
	connector = connector or connectors[signalfunction] or (signalfunction.isconnector and signalfunction)
	if not connector then
		print("can't connect signal: callback slot is already assigned")
		return
	end
	connector.connect(target, slot)
end

function _M.signal()
		local connector = {}
		setmetatable(connector, newmetaconnector(connector))
		return connector
end

return _M
