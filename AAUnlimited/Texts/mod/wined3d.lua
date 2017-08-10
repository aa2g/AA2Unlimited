--@INFO Opengl renderer

local _M = {}

function on.d3d9_preload(prev)
	if prev ~= 0 then
		alert("wined3d must be first of all d3d hooks (move it up)")
		os.exit(1)
	end
	return LoadLibraryA("wined3d9.dll")
end

function _M:load()
end

return _M