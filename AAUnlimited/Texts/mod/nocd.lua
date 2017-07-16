local _M = {}
function _M:load()
	g_poke(0x1E36, "\xeb")
end

return _M