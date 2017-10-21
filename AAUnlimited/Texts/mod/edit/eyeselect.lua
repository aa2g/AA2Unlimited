-- replace eye texture dialogs with a sane one

local eye = {}
local super

function select_eye(dir,field)
	require "iuplua"
	local path = host_path("data","texture",dir)
	local orig = GetPlayerCharacter().m_charData.m_eyes[field]
	local dlg = iup.filedlg {
		allownew = "yes",
		dialogtype = "OPEN",
		filter = "*.bmp;*.tga",
		filterinfo =  "bmp or tga "..dir.." texture",
		directory=path,
		file=orig,
		showpreview="yes",
	}
	dlg:popup()
	if dlg.status == "-1" then return end
	local eye_select = dlg.value:match("[^\\]*$")
	GetPlayerCharacter().m_charData.m_eyes[field] = eye_select
	super.update_ui(true)
end

return function(_M, opts, p)
	super = _M
	-- eye control hook
	p:g_hook_vptr(0x304CCC, 4, function(orig,this,a2,a3,a4,hwnd)
		if a3 == 273 then
			local ctrl = a4 & 0xffff
			if ctrl == 10215 then
				select_eye("eye", "texture")
				return 0
			elseif ctrl == 10214 then
				select_eye("hilight", "highlight")
				return 0
			end
		end
		return proc_invoke(orig,this,a2,a3,a4,hwnd)
	end)
end