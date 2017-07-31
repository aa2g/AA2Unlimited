--@INFO Implements POV facecam (camera tracks eye bones)

local _M = {}

local hinfo

function on.keydown(k)
	print("got key", k)
	return k
end

function on.start_h(hi)
	hinfo = hi
end

function on.end_h()
	hinfo = nil
end


function _M.load()
end

function _M.unload()
end

return _M