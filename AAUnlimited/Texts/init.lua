require "debug"

AAU_VERSION = "1.5 "
---------------------------
-- C++ interfacing globals
---------------------------
assert(_BINDING)
assert(_BINDING.setlogprio, "C++ logger missing")
assert(_BINDING.logger, "C++ logger missing")
	_BINDING.setlogprio(1)
_CONFIG = _CONFIG or {}
_WIN32 = {}
__LOGGER = function(msg)
	return false -- keep it for later
end
local _, ver = pcall(dofile, _BINDING.GetAAUPath() .. "version.lua")
if ver then
	AAU_VERSION = AAU_VERSION .. " " .. ver
end

---------------------------
-- basic utils
---------------------------
function check(x,a,b,...)
	if a == nil then
		log.spam(x .. " failed with:  " .. b)
	end
	return a,b,...
end

function make_path(pfx,...)
	local npfx = pfx
--[[	repeat
		pfx = npfx
		npfx = pfx:gsub("\\\\","\\"):gsub("//","/")
	until npfx == pfx]]
	npfx = npfx:match("^(.*)[\\/]$") or npfx

	return table.concat({npfx,...}, "\\")
end

function aau_path(...)
	local ret = make_path(_BINDING.GetAAUPath(), ...)
	return ret
end

function host_path(...)
	local ret = make_path(_BINDING.IsAAEdit and _BINDING.GetAAEditPath() or _BINDING.GetAAPlayPath(), ...)
	return ret
end

function play_path(...)
	local ret = make_path(_BINDING.GetAAPlayPath(), ...)
	return ret
end

---------------------------
-- logger
---------------------------
log = {}

-- XREF: match enum Files/logger.h
for prio,name in ipairs { "raw", "spam", "info", "warn", "error", "crit" } do
	local p = prio-2
	log[name] = function(...)
		return _BINDING.logger(p, string.format(...))
	end
	setmetatable(log, {
		__call = function(o,...)
			o.spam(...)
		end
	})
end

---------------------------
-- config processing
---------------------------
local cfproxy = {}
function cfproxy:__index(k)
	return _CONFIG[k] or _BINDING.Config[k]
end
function cfproxy:__newindex(k,v)
	if k == "logPrio" then
		_BINDING.setlogprio(v)
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
		log.error("Config %s failed with %s", lua, debug.traceback(err))
	end)
	_BINDING.setlogprio(_CONFIG.logPrio)
	setmetatable(_G, nil)
end

function table.dump(v, ores)
	local res = ores or {}
	local t = type(v)
	if t == "function" then 
		table.insert(res, "<function>")
		return res
	elseif t ~= "table" then 
		table.insert(res, string.format("%q",v))
		return res
	end
	table.insert(res, "{")
	for tk,tv in pairs(v) do
		table.insert(res, "[")
		table.dump(tk, res)
		table.insert(res, "]=")
		table.dump(tv, res)
		table.insert(res, ",")
	end
	table.insert(res, "}")
	return res
end


local function saveval(of,n,v)
	if type(v) == "function" then return end
	of:write(string.format("%s=%s\n",n,table.concat(table.dump(v))))
end

function Config.save(fn)
	local fn = fn or aau_path("savedconfig.lua")
--	log("saving config to "..fn)

	local f = io.open(fn, "w+")
	for k,v in pairs(_CONFIG) do
		saveval(f,k,v)
	end
	for k,v in pairs(getmetatable(_BINDING.Config)) do
		if not k:match("^__.*") then
			saveval(f,k,_BINDING.Config[k])
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
info = function(...)
	local res = {}
	for i,v in ipairs {...} do
		res[i] = tostring(v)
	end
	local msg = table.concat(res, "\t")
	log.info("%s", msg)
end

print = function(...)
	local res = {}
	for i,v in ipairs {...} do
		res[i] = tostring(v)
	end
	local msg = table.concat(res, "\t")
	log.spam("%s", msg)
end

function raw_print(...)
	local res = {}
	for i,v in ipairs {...} do
		res[i] = tostring(v)
	end
	local msg = table.concat(res, "\t")
	log.raw("%s", msg)
end


---------------------------
-- now load config files
---------------------------
exe_type = _BINDING.IsAAEdit and "edit" or "play"
Config.load("config")
Config.load("localconfig")
Config.load("savedconfig")
Config.load("forcedconfig")
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
					local ret1, ret2 = _BINDING.proc_invoke(got,nil,...)
					return ret1 & 0xffffffff, ret2 & 0xffffffff
				end
				return _WIN32[k]
			end
		end
	end
})

local modules = {}
local init_pending = {}
local mnames = {}
local current_module
local handlers = {}
on = {}

local function _load_module(mod)
	current_module = mod[1]
	--log("Trying to load "..mod[1])
	local ok, data = xpcall(require, debug.traceback, mod[1])
	if not ok then
		log.error("Module %s failed: %s", mod[1], data)
	else
		if type(data) == "table" and data.load then
			data.info = mod
			log("Loaded %s", data.info[1])
			if init_pending then
				table.insert(init_pending, data)
			end
			modules[data.info[1]] = data
			current_module = nil
			return data
		else
			log.error("%s is not a valid module", mod[1])
		end
	end
end

function reload_module(n)
	if module_can_unload(n) then
		unload_module(n)
	else
		log("Module %s can't unload", n)
	end
	if not module_is_loaded(n) then
		init_module(load_module(get_mod_info(n)))
	end
end

function load_module(mod)
	unlock_globals()
	local ret = _load_module(mod)
	lock_globals()
	return ret
end

function unload_module(name)
	log.spam("Unloading %s", name)
	local mod = modules[name]
	if mod.unload then
		mod:unload()
	end
	if not mod then return end
	-- nuke all event handlers of a given module
	for evn,v in pairs(handlers) do
		--log("scanning owners of %s", evn)
		local i = 1
		while i <= #v do
			if not v[i] then
				--log("break skip")
				break
			end
			--log("print id %d owner %s", i, v[i][2])
			if v[i][2] == name then
				log("[%s] removing handler %d for %s", name, i, evn)
				table.remove(v, i)
			else
				i = i + 1
			end
		end
	end

	-- remove its lua space modules, and also submodules
	for k,v in pairs(package.loaded) do
		if k == name or k:find(name .. ".", 1, true) then
			package.loaded[k] = nil
		end
	end

	-- and from our list
	modules[name] = nil

	-- and this should nuke it for good
	collectgarbage("collect")
	collectgarbage("collect")
	collectgarbage("collect")
	collectgarbage("collect")
end


function load_modules()
	Config.mods = Config.mods or {}
	lock_globals()
	for f in readdir(aau_path("mod","*")) do
		local mname = f:match("^(.*)%.lua$") or f
		if mname == f then
			f = f .. "/init.lua"
		end

		-- we dont know about this module yet
		if (not get_mod_info(mname)) and get_mod_desc(aau_path("mod",f)) then
			table.insert(Config.mods, 1, {mname, disabled=true})
		end
	end
	unlock_globals()
	for i,mod in ipairs(Config.mods) do
		if not type(mod) == "table" then
			log.error("invalid mod entry at index %d", i)
		elseif not mod.disabled then
			_load_module(mod)
		end
	end
end

function module_can_unload(mod)
	--print("can unload ",mod, ((modules[mod] or {}).info or {}).unload)
	return (modules[mod] or {}).unload
end

function module_is_loaded(mod)
	return modules[mod]
end

function init_module(mod)
	log("Initializing %s", mod.info[1])
	local ok, msg = xpcall(mod.load, debug.traceback, mod.info)
	if not ok then
		log.error("Unable to initialize %s: %s", mod.info[1], msg)
	else
		--log("Initialized %s", mod.info[1])
	end
end

global_writes = false

function error_trace(msg)
	error(debug.traceback(tostring(msg)))
end

function lock_globals()
	setmetatable(_G, {
		__index = function(t,k)
			return _BINDING[k] or _WIN32[k] or error_trace("accessing undefined global '"..tostring(k).."'")
		end,
		__newindex = function(t,k,v)
			if global_writes then
				rawset(t,k,v)
			else
				error_trace("attempted write to global " .. tostring(k) .. " value " .. tostring(v))
			end
		end
	})
end

function unlock_globals()
	setmetatable(_G, nil)
end

function init_modules()
	lock_globals()
	log.spam("Initializing modules")
	for _,mod in ipairs(init_pending) do
		init_module(mod)
	end
	init_pending = nil
	require "patches"
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


function __DISPATCH_EVENT(name, arg1, ...)
	local filter = {
		plan = true,
		mousemove = true,
		mousedown = true,
		mouseup = true,
	}
	if not filter[name] then
		print("EVENT: ", name, arg1, ...)
	end

	for _,h in ipairs(handlers[name] or {}) do
		local ok, msg = pcall(function(...)
			local retv = h[1](arg1, ...)
			arg1 = retv ~= nil and retv or arg1
		end, ...)
		if not ok then
			log.error("Event dispatch failed: %s", msg)
		end
	end

	return arg1
end


function add_event_handler(evname, v)
	handlers[evname] = handlers[evname] or {}
	table.insert(handlers[evname], {v, current_module})
end

setmetatable(on, {
	__newindex = function(t, evname, v)
		assert(current_module, "event handlers can be installed only on module load (ie top level)")
		add_event_handler(evname, v)
	end
})

function tohex(n)
	return string.format("%x",n)
end


function readdir(path)
	require "memory"
	require "strutil"

	local mem, fh
	return function()
		local ok
		if not fh then
			mem = malloc(592)
			local pa = utf8_to_unicode(path .. "\0")
			fh = FindFirstFileW(pa, mem)
			ok = fh ~= 0xffffffff
		else
			ok = FindNextFileW(fh, mem) ~= 0
		end
		if not ok then
			FindClose(fh)
			free(mem)
			return nil
		end
		return unicode_to_utf8(peek(mem + 44, 256, "\0\0", 2))
	end
end

function get_mod_desc(path)
	local f = io.open(path)
	if not f then
		return
	end
	local ln = f:read("*l")
	local desc = ln:match("^--@INFO (.*)")
	f:close()
	return desc
end

-- grabs all information about module just by its name
function get_mod_info(n)
	local mi
	local idx
	for i,v in ipairs(Config.mods) do
		if v[1] == n then
			mi = v
			idx = i
			break
		end
	end
	if not mi then
		return
	end
	local desc = get_mod_desc(aau_path("mod", n..".lua")) or get_mod_desc(aau_path("mod", n, "init.lua"))
	if not desc then
		return
	end
	return mi, desc, idx
end

getmetatable("").__mod = function(o,p)
	if type(p) == "table" then
		return o:format(table.unpack(p))
	else
		return o:format(p)
	end
end
function alert(who, msg)
	require "iuplua"
	iup.Message(who, msg)
end

function mod_load_config(mod, opts)
	setmetatable(opts, {__index=mod, __newindex=mod})
	for _,v in ipairs(opts) do
		local optn = v[1]
		if optn then
			if opts[optn] == nil then
				opts[optn] = v[2]
			end
		end
	end
end

function mod_edit_config(mod, opts, title)
	if not mod then return end
	require "iuplua"
	require "iupluacontrols"
	local vars = {}
	local varnames = {}
	local fmt = {}
	for _,v in ipairs(opts) do
		local optn = v[1]
		if optn then
			table.insert(vars, opts[optn])
			table.insert(varnames, optn)
		end
		table.insert(fmt, v[3])
	end
	local res = {iup.GetParam(title, nil, table.concat(fmt, "\n").."\n", table.unpack(vars))}
	if res[1] then
		for i, name in ipairs(varnames) do
			opts[name] = res[i+1]
		end
		Config.save()
	end
end

function is_key_pressed(key)
	if key == nil then return false end
	if key < 0 then return false end
	return (GetAsyncKeyState(key) & 0x8000) ~= 0
end

function strdiff(a,b)
	local res = {}
	for i=1,math.min(#a,#b) do
		if a:byte(i) ~= a:byte(i) then
			table.insert(res, i)
		end
	end
	return res
end

function p(t)
	local pl = require 'pl.pretty'
	local r = pl.write(t)
	return r
end

local old_require = require
function require(x)
	local sav = global_writes
	global_writes = true
	local a,b = old_require(x)
	global_writes = sav
	return a,b
end


function set_class_key(k,v)
	local json = require "json"
	SetClassJSONData(k,json.encode(v))
end

function get_class_key(k)
	local json = require "json"
	local js = GetClassJSONData(k)
	if js then
		return json.decode(js)
	end
end

help = "If you don't know what's this for, you shouldn't probably use it. See https://github.com/aa2g/AA2Unlimited/wiki/Lua-cruft"
