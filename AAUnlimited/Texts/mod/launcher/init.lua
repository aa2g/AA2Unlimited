require "memory"
require "strutil"

local dlg = require "launcher.dlg"

local _M = {}

function _M:load()
	log("starting launcher")
	if true then
	local dlgproc = _BINDING.IsAAEdit and 0x2FD624 or 0x31C66C
	g_hook_vptr(dlgproc, 4, function(orig, this, hdlg, msg, lparam, wparam)
		--log.spam("winmsg %x %x %s/%x %x %x",this,hdlg,rmsg[msg] or "unk", msg,lparam,wparam)
		if msg == 0x18 then
			ret = proc_invoke(orig, this, hdlg, 0x111, 0x3e9, 0)
		else
			ret = proc_invoke(orig, this, hdlg, msg, lparam, wparam)
		end
		return ret
	end)

	dlg()
	end


	--_EVENTS.logger = nil
end

function _M:unload()
	-- TODO reloading
end

return _M