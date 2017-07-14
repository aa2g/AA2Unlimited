require "debug"

---------------------------
-- C++ interfacing globals
---------------------------
assert(_BINDING)
_EVENTS = _EVENTS or {}
_EVENTS.Config = _EVENTS.Config or {}
_CONFIG = _CONFIG or {}


---------------------------
-- basic utils
---------------------------
function Check(x,a,b,...)
	if a == nil then
		Log.warn(x .. "failed with: " .. b)
	end
	return a,b,...
end

function Path(...)
	return table.concat({_BINDING.AAUPath, ...}, "/")
end



---------------------------
-- logger
---------------------------
assert(_BINDING.logger, "C++ logger missing")
Log = {}

-- XREF: match enum Files/logger.h
for prio,name in ipairs { "spam", "info", "warn", "err", "crit" } do
	local p = prio-1
	Log[name] = function(...)
		return _BINDING.logger(p, ...)
	end
end

-- empty logger hook
function _EVENTS.logger(...)
	return false, ...
end

-- fictional config field via config event
function _EVENTS.Config.logPrio(v)
	_BINDING.setlogprio(v)
	return v
end


---------------------------
-- config processing
---------------------------
local cfproxy = {}
function cfproxy:__index(k)
	return _CONFIG[k] or _BINDING.Config[k]
end
function cfproxy:__newindex(k,v)
	if _EVENTS.Config and _EVENTS.Config[k] then
		v = _EVENTS.Config[k](v)
	end
	if v then
		-- if setting a binding fails, its not a C++ setting
		if not pcall(function() _BINDING.Config[k] = v end) then
			_CONFIG[k] = v
		end
	end
end
Config = setmetatable({}, cfproxy)


-- load config
function Config.load(name)


	local lua = Path(name .. ".lua")

	local ch = Check("load config", loadfile(lua))

	setmetatable(_G, cfproxy)

	local ret, msg = xpcall(ch, function()
		Log.spam(debug.traceback(err))
	end)

	setmetatable(_G, nil)

	-- _G.Config transparently binds Lua and C++ together
	setmetatable(Config, cfproxy)


end

---------------------------
-- libraries
---------------------------
package.path = Path("?.lua") .. ";" .. package.path -- in top level, only simple lua files are allowed

package.path = Path("lib", "?.lua") .. ";" .. package.path
package.path = Path("lib", "?", "init.lua") .. ";" .. package.path

package.path = Path("mod", "?.lua") .. ";" .. package.path
package.path = Path("mod", "?", "init.lua") .. ";" .. package.path

package.cpath = Path("lib", "?.dll") .. ";" .. package.cpath
package.cpath = Path("lib", "?", "init.dll") .. ";" .. package.cpath

stdio_print = print
print = function(...)
	local res = {}
	for i,v in ipairs {...} do
		res[i] = tostring(v)
	end
	local msg = table.concat(res, " ")
	local i = debug.getinfo(2, "nSl")
	Log.spam(
		(i.name or "??") .. "() line "..(i.currentline or "?")..": ".. msg
	)
end

Config.load("config")

_BINDING.Seats = {}
_EVENTS.Seats = {}
_EVENTS.H = {}
_EVENTS.Convo = {}
_EVENTS.NpcActions = {}
_EVENTS.PcActions = {}
_EVENTS.Time = {}

function load_modules()
	setmetatable(_G, {
		__index = _BINDING
	})

	Config.mods = Config.mods or {}
	for i,mod in ipairs(Config.mods) do
		if not type(mod) == "table" then
			Log.err("invalid mod entry")
		else
			local ok, msg = xpcall(require, debug.traceback, mod[1])
			if not ok then
				Log.err(msg)
			else
				Log.spam("Loading module "..mod[1])
				if type(msg) == "function" then
					ok, msg = xpcall(msg, debug.traceback, table.unpack(mod))
					if not ok then
						Log.err(msg)
					end
				end
			end
		end
	end
end
