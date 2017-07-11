---------------------------
-- basic utils
---------------------------

function Check(x,a,b,...)
	if a == nil then
		Log.warn(x .. "failed with: " .. b)
	end
	return a,b,...
end

function Path(x)
	return BASEDIR .. "/" .. x
end

---------------------------
-- logger
---------------------------

assert(logger, "C++ logger missing")

Log = {}

// XREF: match enum Files/logger.h
for prio,name in ipairs { "spam", "info", "warn", "err", "crit" } do
	local p = prio
	Log[name] = function(...)
		return logger(p, ...)
	end
end

-- empty logger hook
function on_logger(...)
	return ...
end

---------------------------
-- config processing
---------------------------

Config = {}
assert(_CF, "Config subsystem not initialized")
Config.keys = setmetatable(_CF, {__index=_G, __newindex=function(t,k,v)
	rawset(t,k,v)
	-- propagate the setting to C++
	get_set_config(k,v)
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

require "main"
