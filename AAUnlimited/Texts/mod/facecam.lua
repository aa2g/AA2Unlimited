--@INFO Implements POV facecam (camera tracks eye bones)

local _M = {}

hinfo = false

function on.keydown(k)

	print("got2 key", k)

	if not hinfo then return k end
	if key == 84 then return -1 end
	return k
end

function on.start_h(hi)
	hinfo = hi
end

function on.end_h()
	hinfo = false
end


function _M.load()
end

function _M.unload()
end

return _M