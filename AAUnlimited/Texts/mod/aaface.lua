--@INFO Loads aaface into aa2edit

local _M = {}

function on.launch()
	if exe_type ~= "edit" then return end
	LoadLibraryA("aafacedll.dll")
end

function _M:load()
end

return _M