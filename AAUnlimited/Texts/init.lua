require "debug"

---------------------------
-- C++ interfacing globals
---------------------------
assert(_BINDING)
_EVENTS = _EVENTS or {}
_EVENTS.Config = _EVENTS.Config or {}
_CONFIG = _CONFIG or {}
_BINDING.Seats = {}
_EVENTS.Seats = {}
_EVENTS.H = {}
_EVENTS.Convo = {}
_EVENTS.NpcActions = {}
_EVENTS.PcActions = {}
_EVENTS.Time = {}
_WIN32 = {}



---------------------------
-- basic utils
---------------------------
function check(x,a,b,...)
	if a == nil then
		log.warn(x .. "failed with: " .. b)
	end
	return a,b,...
end

function aau_path(...)
	return table.concat({_BINDING.GetAAUPath(), ...}, "/")
end

function host_path(...)
	return table.concat({_BINDING.IsAAEdit and _BINDING.GetAAEditPath() or _BINDING.GetAAPlayPath(), ...}, "/")
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
	if v ~= nil then
		-- if setting a binding fails, its not a C++ setting
		if not pcall(function() _BINDING.Config[k] = v end) then
			_CONFIG[k] = v
		end
	end
end
Config = setmetatable({}, cfproxy)


-- load config
function Config.load(name)


	local lua = aau_path(name .. ".lua")

	local ch = check("load config", loadfile(lua))
	if not ch then return end

	setmetatable(_G, cfproxy)

	local ret, msg = xpcall(ch, function()
		log.spam("Config failed with " .. debug.traceback(err))
	end)

	setmetatable(_G, nil)
end

local function saveval(of,n,v)
	if type(v) == "function" then return end
	if n ~= "mods" then
--		log(n)
		of:write(string.format("%s=%q\n",n,v))
	end
end

function Config.save(fn)
	local fn = fn or aau_path("savedconfig.lua")
	log("saving config to "..fn)

	local f = io.open(fn, "w+")
	for k,v in pairs(_CONFIG) do
		saveval(f,k,v)
	end
	for k,v in pairs(getmetatable(_BINDING.Config)) do
		local n = k:match("^get_(.*)")
		if n then
			saveval(f,n,_BINDING.Config[n])
		end
	end
	f:close()
end

---------------------------
-- libraries
---------------------------
package.path = aau_path("?.lua") .. ";" .. package.path -- in top level, only simple lua files are allowed

package.path = aau_path("mod", "?.lua") .. ";" .. package.path
package.path = aau_path("mod", "?", "init.lua") .. ";" .. package.path

package.cpath = aau_path("lib", "?.dll") .. ";" .. package.cpath
package.cpath = aau_path("lib", "?53.dll") .. ";" .. package.cpath
package.cpath = aau_path("lib", "?", "init.dll") .. ";" .. package.cpath
package.cpath = aau_path("lib", "?", "init53.dll") .. ";" .. package.cpath


package.path = aau_path("lib", "?.lua") .. ";" .. package.path
package.path = aau_path("lib", "?", "init.lua") .. ";" .. package.path

stdio_print = print
print = function(...)
	local res = {}
	for i,v in ipairs {...} do
		res[i] = tostring(v)
	end
	local msg = table.concat(res, " ")
	log.spam(msg)
end


---------------------------
-- now load config files
---------------------------
exe_type = _BINDING.IsAAEdit and "edit" or "play"
Config.load("config")
Config.load("localconfig")
Config.load("savedconfig")
-- _G.Config transparently binds Lua and C++ together
setmetatable(Config, cfproxy)


-- add more if you need it
local dlls = {"KERNEL32.DLL","USER32.DLL"}
setmetatable(_WIN32, {
	__index = function(t,k)
		for _,v in ipairs(dlls) do
			local got = _BINDING.GetProcAddress(v,k)
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
local mnames = {}

function load_modules()

	Config.mods = Config.mods or {}
	for i,mod in ipairs(Config.mods) do
		log("Trying to load "..mod[1])
		if not type(mod) == "table" then
			log.error("invalid mod entry at index %d", i)
		else
			local ok, data = xpcall(require, debug.traceback, mod[1])
			if not ok then
				log.error("Module %s failed: %s", mod[1], data)
			else
				if type(data) == "table" and data.load then
					data.info = mod
					log("Loaded %s", data.info[1])
					table.insert(modules, data)
					modules[data.info[1]] = data
				else
					log.error("%s is not a valid module", mod[1])
				end
			end
		end
	end
end

function load_module(mod)
	local ok, msg = xpcall(mod.load, debug.traceback, table.unpack(mod.info))
	if not ok then
		log.error("Unable to initialize %s: %s", mod.info[1], msg)
	else
		log("Initialized %s", mod.info[1])
	end
end

function lock_globals()
	setmetatable(_G, {
		__index = function(t,k)
			return _BINDING[k] or _WIN32[k] or error("accessing undefined global '"..tostring(k).."'")
		end
	})
end

function unlock_globals()
	setmetatable(_G, nil)
end

function init_modules()
	lock_globals()
	log("Initializing modules")
	for _,mod in ipairs(modules) do
		load_module(mod)
	end
end

function table.extend(a,b)
	for k,v in pairs(b) do
		a[k] = v
	end
end

function table.append(a,b)
	local last = #a
	for i,v in ipairs(b) do
		a[i+last] = v
	end
	return a
end

function table.indexof(t,q)
	for idx,v in ipairs(t) do
		if v == q then return idx end
	end
	return 0
end

on = {}
local handlers = {}

function __DISPATCH_EVENT(name, arg1, ...)
	if name ~= "plan" then
		print("EVENT: ", name, arg1, ...)
	end

	for _,h in ipairs(handlers[name] or {}) do
		local retv = h(arg1, ...)
		arg1 = retv ~= nil and retv or arg1
	end

	return arg1
end


function add_event_handler(evname, v)
	handlers[evname] = handlers[evname] or {}
	table.insert(handlers[evname], v)
end

setmetatable(on, {
	__newindex = function(t, evname, v)
		add_event_handler(evname, v)
	end
})

function hexdump(addr, size)
	buf = peek(addr, size)
	return buf:gsub("(.)", function(d)
		return string.format("%02x ", string.byte(d))
	end)
end
function tohex(n)
	return string.format("%x",n)
end