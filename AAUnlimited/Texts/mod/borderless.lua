--@INFO Faux fullscreen

require "memory"

local _M = {}

function on.post_d3d(hwnd)
	require 'iuplua'
--[[	tex=_BINDING.CreateTexture(64,64)
	tex:Clear(0x80808080)
	tex:Draw(0,0)]]

	local rect = malloc(16)

	GetWindowRect(GetDesktopWindow(), rect)
	local screenw, screenh = string.unpack("<II", peek(rect+8, 8))
	GetClientRect(hwnd, rect)

	local left, top, right, bottom = string.unpack("<IIII", peek(rect, 16))
	print(left,top,right,bottom,screenw,screenh)
	if right == screenw and bottom == screenh then
		local st = GetWindowLongW(hwnd, -16)
		st = 0x90000000
		SetWindowLongW(hwnd, -16, st)
		SetWindowLongW(hwnd, -20, 0x00040000)
		AdjustWindowRectEx(rect,  st, 0, 0x00040000)
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