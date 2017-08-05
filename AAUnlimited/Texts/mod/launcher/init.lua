--@INFO This launcher, should be always last in the list

require "memory"
require "strutil"
local dlg = require "launcher.dlg"

local _M = {}

function _M:load()
	if true then

	local dlgproc = _BINDING.IsAAEdit and 0x2FD624 or 0x31C66C
	log("starting launcher, detected exe type is %s", exe_type)
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

	-- some simple patches too easy to warrant custom module
	if exe_type == "play" then
		g_poke(0x0032E48A, Config.bUseMKIII and "t\0g\0a\0" or "b\0m\0p\0")
	end

	if exe_type == "edit" and Config.bUseAA2Face then
		LoadLibraryA("..\\AAFaceDLL.DLL")
	end

end


return _M