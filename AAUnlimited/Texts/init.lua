---------------------------
-- C++ interfacing globals
---------------------------
assert(_BINDING)
assert(_HOOKS)
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

// XREF: match enum Files/logger.h
for prio,name in ipairs { "spam", "info", "warn", "err", "crit" } do
	local p = prio-1
	Log[name] = function(...)
		return _BINDING.logger(p, ...)
	end
end

-- empty logger hook
function _HOOKS.logger(...)
	return false, ...
end

---------------------------
-- config processing
---------------------------
local cfproxy = {}
function cfproxy:__index(k)
	return _CONFIG[k] or _BINDING.config(k)
end
function cfproxy:__newindex(k,v)
	_BINDING.config(k,v)
	_CONFIG[k] = v
end
Config = setmetatable(Config, cfroxyy)

-- load config
function Config.load(name)
	local lua = Path(name .. ".lua")

	setmetatable(_G, cfproxy)
	local ch = Check("load config", loadfile(lua))
	setmetatable(_G, nil)
	ch(binding)
	setmetatable(Config.keys, nil)
end

---------------------------
-- libraries
---------------------------
package.path = Path("?.lua") .. ";" .. package.path -- in top level, only simple lua files are allowed
package.path = Path("lib", "?.lua") .. ";" .. package.path
package.path = Path("lib", "?", "init.lua") .. ";" .. package.path
package.cpath = Path"lib", "?.dll") .. ";" .. package.cpath
package.cpath = Path"lib", "?", "init.dll") .. ";" .. package.cpath

require "main"
