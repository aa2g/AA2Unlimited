--@INFO Win10 low FPS fix (needs game restart)

local _M = {}

local variants  = { "win10fix.dll", "dgd3d9.dll", "dxvk.dll" }

local opts = {
	{"variant", 0, "Select d3d9 variant (needs restart): %l|Original d3d9|dgVoodoo 2|DXVK|"},
}

function on.d3d9_preload(prev)
	if prev ~= 0 then
		alert("win10fix", "D3D conflict, this one must be first one to load and is mutually exclusive with wined3d")
		os.exit(1)
	end
	return LoadLibraryA(variants[opts.variant + 1])
end

function _M:load()
	self.cfg = self.cfg or {}
	mod_load_config(self, opts)
end

function _M:unload() end

function _M:config()
	mod_edit_config(self, opts, "Windows 10 d3d9 fix settings")
end

return _M
