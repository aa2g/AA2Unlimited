--@INFO Sideloads win10 low FPS fix (10586 win10 d3d9.dll)

local _M = {}

function on.d3d9_preload(prev)
	if prev ~= 0 then
		alert("Win 10 fix must be first to load in a chain of d3d dlls (move it up)")
		os.exit(1)
	end
	return LoadLibraryA("win10fix.dll")
end

function _M:load()
end

return _M