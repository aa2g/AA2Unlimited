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
Config = {}
Config.keys = setmetatable(_CONFIG, {__index=_G, __newindex=function(t,k,v)
	rawset(t,k,v)
	-- propagate the setting to C++
	if _BINDING.config then
		_BINDING.config(k,v)
	end
end
})

-- load config chunk, rudimentarily converts .txt to .lua if necessary
function Config.load(name)
	local txt = Path(name .. ".txt")
	local lua = Path(name .. ".lua")

	local ch, err = Check("load config", loadfile(lua, nil, Config.keys))
	local f = io.open(txt,"r")
	if not ch and f and and not io.open(lua,"r") then
		Log.info("Converting config.txt to config.lua")
		local out = Check("convert config", io.open(lua, "w+"))
		if out then
			out:write(f:gsub(";","--"))
			f:close()
		end
		return Config.load(name)
	end
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
