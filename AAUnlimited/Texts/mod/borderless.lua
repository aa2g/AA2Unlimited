--@INFO Faux fullscreen.

require "memory"

local _M = {}

function on.first_tick(hwnd)
	local rect = malloc(16)
	GetWindowRect(GetDesktopWindow(), rect)
	local screenw, screenh = string.unpack("<II", peek(rect+8, 8))

	GetClientRect(hwnd, rect)
	local left, top, right, bottom = string.unpack("<IIII", peek(rect, 16))
	if right == screenw and bottom == screenh then
		AdjustWindowRectEx(rect, GetWindowLongW(hwnd, -16), 0, GetWindowLongW(hwnd, -20))
		left, top, right, bottom = string.unpack("<IIII", peek(rect, 16))
		SetWindowPos(hwnd, 0, left, top, right - left, bottom - top, 0)
	else
		log("borderless: cant reposition window - screen resolution doesn't match game resolution'")
	end
	free(rect)
end


function _M.load()
end

return _M