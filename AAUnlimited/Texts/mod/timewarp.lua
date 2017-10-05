--@INFO Travel in time between periods and days

local _M = {}
local keys = { 0x02, 0x04, 0x10, 0x11, 0x12 }
local keyf = "%l|None|Right Mouse Button|Middle Mouse Button|Shift Key|Control Key|Alt Key|"
local opts = {
	{ "back", 1, "Travel back a period (or day): "..keyf},
	{ "days", 3, "Travel days instead of period: "..keyf },
}

function on.period(new,old)
	if new == old then return end
	local daymode = is_key_pressed(keys[opts.days])
	local back =  is_key_pressed(keys[opts.back]) and -1 or 1

	local oldnew = new
	if daymode then
		info("timewarp: %d day" % back)
		local d = GetGameTimeData()
		d.day = (d.day + 7 + back) % 7
		d.nDays = math.min(d.nDays + back, 0)
		-- keep period the same
		new = old
	else
		new = 1 + (old - 1 + 10 + back) % 10
		info("timewarp: %d period, new = %d, old = %d" % {back, new, old})
		-- if we're going forward, the result should be same as vanilla game
		if back == 1 then
			assert(oldnew == new)
		end
	end
	return new
end

function _M:load()
	mod_load_config(self, opts)
end

function _M:unload()
end

function _M:config()
	mod_edit_config(self, opts, "Extended save options")
end

return _M