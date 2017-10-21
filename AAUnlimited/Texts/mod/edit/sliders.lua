-- fix edit slider ranges

local c = require 'const'

return function(_M, opts, p)
	local avoid = {
		[10176] = true,
		[10292] = true,
		[10356] = true,
		[10440] = true,
		[10551] = true,
		[10355] = true,
	}

	-- SendMessageW
	p:g_hook_vptr(0x002C43E0, 4, function(orig, this, hdlg, msg, wparam, lparam)
		-- slider range - something asking for 100, make it 255
		local itemid = GetWindowLongW(hdlg, -12)
		if not avoid[itemid] then

			if msg == c.TBM_SETRANGEMAX and wparam == 1 and lparam == 100 then
				--log("OVERRIDE 100->255 %x %x %x %x %x %x", orig,this,hdlg,msg,wparam,lparam)
				lparam = 255 
			end

			-- text field size, 6 and 26 -> 255
			if msg == c.EM_SETLIMITTEXT and (wparam == 26) or (wparam == 6) then
				wparam = 255
			end
		end

		local ret = proc_invoke(orig, this, hdlg, msg, wparam, lparam)
		return ret
	end)
end