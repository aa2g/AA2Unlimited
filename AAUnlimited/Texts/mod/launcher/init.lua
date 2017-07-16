require "memory"
local dlg = require "launcher.dlg"

return function()
	if true then
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