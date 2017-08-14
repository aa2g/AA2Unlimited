--@INFO Sideloads win10 low FPS fix (needs game restart)

local _M = {}

function on.d3d9_preload(prev)
	if prev ~= 0 then
		alert("win10fix", "D3D conflict, this one must be first one to load and is mutually exclusive with wined3d")
		os.exit(1)
	end
	return LoadLibraryA("win10fix.dll")
end

function _M:load()
end
function _M:unload() end

return _M