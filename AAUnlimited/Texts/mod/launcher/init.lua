require "memory"
local dlg = require "launcher.dlg"

local _M = {}

function _M:load()
	if IsAAPlay then
		g_hook_vptr(0x31C66C, 4, function(orig, this, hdlg, msg, lparam, wparam)
			log.spam("winmsg %x %x %x %x %x",this,hdlg,msg,lparam,wparam)
			if msg == 0x18 then
				ret = proc_invoke(orig, this, hdlg, 0x111, 0x3e9, 0)
			else
				ret = proc_invoke(orig, this, hdlg, msg, lparam, wparam)
			end
			return ret
		end)
	end
	log("starting launcher")
	dlg()
end

function _M:unload()
	-- TODO reloading
end

return _M