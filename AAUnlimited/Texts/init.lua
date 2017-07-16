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
log = {}

-- XREF: match enum Files/logger.h
for prio,name in ipairs { "spam", "info", "warn", "error", "crit" } do
	local p = prio-1
	log[name] = function(...)
		return _BINDING.logger(p, string.format(...))
	end
	setmetatable(log, {
		__call = function(o,...)
			o.spam(...)
		end
	})
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
		log.spam("tata" .. debug.traceback(err))
	end)

	setmetatable(_G, nil)

	-- _G.Config transparently binds Lua and C++ together
	setmetatable(Config, cfproxy)


end

---------------------------
-- libraries
---------------------------
package.path = Path("?.lua") .. ";" .. package.path -- in top level, only simple lua files are allowed

package.path = Path("mod", "?.lua") .. ";" .. package.path
package.path = Path("mod", "?", "init.lua") .. ";" .. package.path

package.cpath = Path("lib", "?.dll") .. ";" .. package.cpath
package.cpath = Path("lib", "?53.dll") .. ";" .. package.cpath
package.cpath = Path("lib", "?", "init.dll") .. ";" .. package.cpath
package.cpath = Path("lib", "?", "init53.dll") .. ";" .. package.cpath


package.path = Path("lib", "?.lua") .. ";" .. package.path
package.path = Path("lib", "?", "init.lua") .. ";" .. package.path

stdio_print = print
print = function(...)
	local res = {}
	for i,v in ipairs {...} do
		res[i] = tostring(v)
	end
	local msg = table.concat(res, " ")
	log.spam(msg)
end

Config.load("config")

_BINDING.Seats = {}
_EVENTS.Seats = {}
_EVENTS.H = {}
_EVENTS.Convo = {}
_EVENTS.NpcActions = {}
_EVENTS.PcActions = {}
_EVENTS.Time = {}

_WIN32 = {}


-- add more if you need it
local dlls = {"KERNEL32.DLL","USER32.DLL"}
setmetatable(_WIN32, {
	__index = function(t,k)
		for _,v in ipairs(dlls) do
			local got = GetProcAddress(v,k)
			if got then
				_WIN32[k] = function(...)
					return _BINDING.proc_invoke(got,nil,...)
				end
				return _WIN32[k]
			end
		end
	end
})

local modules = {}

function load_modules()

	Config.mods = Config.mods or {}
	for i,mod in ipairs(Config.mods) do
		if not type(mod) == "table" then
			log.error("invalid mod entry at index %d", i)
		else
			local ok, msg = xpcall(require, debug.traceback, mod[1])
			if not ok then
				log.error("Module %s failed: %s", mod[1], msg)
			else
				if type(msg) == "function" then
					log("Loaded %s", mod[1])
					table.insert(modules, {msg,mod})
				end
			end
		end
	end
end

function init_modules()
	-- lock globals post-load
	setmetatable(_G, {
		__index = function(t,k)
			return _BINDING[k] or _WIN32[k] or error("accessing undefined global '"..tostring(k).."'")
		end
	})

	log("Initializing modules")
	for i,mod in ipairs(modules) do
		local ok, msg = xpcall(mod[1], debug.traceback, table.unpack(mod[2]))
		if not ok then
			log.error("Unable to initialize %s: %s", mod[2][1], msg)
		else
			log("Initialized %s", mod[2][1])
		end
	end

end
