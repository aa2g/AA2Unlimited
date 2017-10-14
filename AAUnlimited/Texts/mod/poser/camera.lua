local _M = {}

local keys = {
	rotx = "m_xRotRad",
	roty = "m_yRotRad",
	rotz = "m_zRotRad",

	dist_to_mid = "m_distToMid",
	fov = "m_fov",

	shiftz = "m_zShift",
	shifty = "m_yShift",
	shiftx = "m_xShift",
}

_M.keys = {}
for k,_ in pairs(keys) do
	table.insert(_M.keys, k)
end

local mt = {
	__index = function(self, k)
		return GetCamera()[keys[k]]
	end,
	
	__newindex = function(self, k, v)
		GetCamera()[keys[k]] = v
	end
}
setmetatable(_M, mt)

return _M
