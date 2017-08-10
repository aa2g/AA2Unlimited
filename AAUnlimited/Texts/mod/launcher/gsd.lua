local _M = {}
local pad = "dummy"
local zero = "\x00"
local int = "I"
local bool = "b"
local byte = "B"
local format = {
	--"\x03\x00\x00\x00\x00",
	"\x03\x00\x00\x00\x00",
	{ x=int },
	{ y=int },

	{ zoom=bool },

	{ aspectx=byte },
	{ aspecty=byte },

	"\xdf\xff\xff\xff\x00\x00", -- mandatory

	{ fullscreen=bool },
	{ mipmap=byte },

	--"\x00\x00\x00",
	"\x01\x00\x02",

	{ sharp=bool },
	{ bilinear=bool },
	"\x00",
	{ fastrender=bool },
	{ svp=bool },

	"\x00\x01\x00\x01\x00\x00\x00\x00\x00\x00",
	--zero:rep(10),

	{ blur=bool },
	"\x00",
	{ shadowmap=byte },
	"\x00",
	{ rim=bool },

	"\x01", -- shadowmap for body enable?
	{ dynlight=bool },
	 "\x00\x00",
	{ outline=bool },

	zero:rep(1046),

	{ aa=byte }
}

function _M.check_gsd(cfg)
	--[[assert(cfg.aa <= 7)
	assert(cfg.shadowmap <= 2)
	assert(cfg.mipmap <= 2)]]

end

function _M.load_gsd()
	local path = host_path("data","gsd.cfg")
	local f = check("load " .. path, io.open(path, "rb"))
	local data = f:read("*a")
	f:close()
	local res = {}
	local pos = 1
	for _,v in ipairs(format) do
		local buf = data:sub(pos,pos+#v-1)
		if type(v) == "string" then
--[[			log("skipping %d %s ", pos-1,
				buf:gsub("(.)",
					function(d)
						return string.format("%02x ", string.byte(d))
					end
				)
			)]]
			pos = pos + #v
		else
			local name = next(v)
			local format = v[name]
			local val = format:unpack(data, pos)
--[[			if format == bool then
				val = val == 1
			end]]
			if format == int then
				val = val ~ 0xffffffff
			end
			--log("loaded "..name.." with value "..tostring(val))
			res[name] = val
--			log("loaded %s %s", name, tostring(val))
			pos = pos + format:packsize()
		end
	end
	_M.check_gsd(res)
	return res
end

function _M.save_gsd(cfg)
	_M.check_gsd(cfg)
	local fn = host_path("data", "gsd.cfg")
	--log("saving gsd to "..fn)
	local f = io.open(fn, "w")
	for _,v in ipairs(format) do
		if type(v) == "string" then
			f:write(v)
		else
			local name = next(v)
			local format = v[name]
			local val = cfg[name]
			if format == int then
				val = val ~ 0xffffffff
			end
--[[			if format == bool then
				val == val and 1 or 0
			end]]
			f:write(format:pack(val))
		end
	end
	f:close()
end

return _M