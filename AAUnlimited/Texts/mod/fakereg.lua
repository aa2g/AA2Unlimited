--@INFO Fakes registry to make game portable

require "memory"


-- We redirect game directories, so we must do that very early for rest of AAU to take note
local fakepath = _BINDING.GameExeName:match("^.*\\")
_BINDING.SetAAPlayPath(fakepath)
_BINDING.SetAAEditPath(fakepath)

local _M = {}

function on.launch()
	local query = 0x2C4000
	local close = 0x2C4004
	local open = 0x2C4008
	if _BINDING.IsAAPlay then
		query = 0x2E3000
		close = 0x2E3004
		open = 0x2E3008
	end
	g_hook_vptr(open, 5, function(orig, this, key, subkey, opt, sam, result)
		if (key == 0x80000001) then
			poke_dword(result, 0xdeadbabe)
			return 0
		end
		return proc_invoke(orig, this, key, subkey, opt, sam, result)
	end)

	g_hook_vptr(query, 6, function(orig, this, key, value, res, type, pdata, pcdata)
		if key == 0xdeadbabe then
			local str = utf8_to_unicode(fakepath .. "\x00")
			if type ~= 0 then
				poke_dword(type, 1)
			end
			poke(pdata, str)
			poke_dword(pcdata, #str)
			return 0
		end
		return proc_invoke(orig, this, key, value, res, type, pdata, pcdata)
	end)
	local opt = g_hook_vptr(close, 1, function(orig, this, hk)
		if (hk == 0xdeadbabe) then
			return 0
		end
		return proc_invoke(orig, this, hk)
	end)

end

function _M.load()
end

return _M