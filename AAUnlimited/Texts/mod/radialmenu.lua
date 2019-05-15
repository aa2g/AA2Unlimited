--@INFO Allows to set some actions on the mouse

local _M = {}
local mcfg 				-- module config

local funcs_general = {} -- info for every func
local funcs_h_scene = {}
local funcs_gen_str = "|"
local funcs_h_str = "|"

local function reload_buttons()
	funcs_general = {}
	funcs_h_scene = {}
	funcs_gen_str = "|None|"
	funcs_h_str = "|None|"
	local func_lang
	local scene_type
	local func_name
	local cfg_title
	local title
	local short_desc
	local count_gen = 1
	local count_h = 1
	local functions_path = aau_path("configs", "radmenu_registerfunc.txt")
	local file = io.open(functions_path, "r")
	if not file then return end
	for line in file:lines() do
		func_lang, scene_type, func_name, cfg_title, title, short_desc = line:match("{([^%}]+)}[^%{]*{([^%}]+)}[^%{]*{([^%}]+)}[^%{]*{([^%}]+)}[^%{]*{([^%}]+)}[^%{]*{([^%}]+)}")
		if func_lang and scene_type and func_name and cfg_title and title and short_desc then
			if scene_type == "general" then
				funcs_general[count_gen] = {func_lang = func_lang, func_name = func_name, 
				cfg_title = cfg_title, title = title, short_desc = short_desc }
				funcs_gen_str = funcs_gen_str .. cfg_title .. "|"
				count_gen = count_gen + 1
			elseif scene_type == "h_scene" then
				funcs_h_scene[count_h] = {func_lang = func_lang, func_name = func_name, 
				cfg_title = cfg_title, title = title, short_desc = short_desc }
				funcs_h_str = funcs_h_str .. cfg_title .. "|"
				count_h = count_h + 1
			end
		end
	end
	--log.info("Loaded %d gen funcs", count_gen-1)
	--log.info("Loaded %d H funcs", count_h-1)
end

local function func_name_to_node(scene_type, func_name)
	if func_name == 0 then return 0 end
	local funcs_current = {}
	if scene_type == "general" then 
		funcs_current = funcs_general
	elseif scene_type == "h_scene" then
		funcs_current = funcs_h_scene
	else
		return 0
	end
	
	for func_i, funcdata in ipairs(funcs_current) do
		if funcdata.func_name == func_name then
			return func_i
		end
	end
	
	return 0
end

local function func_node_to_name(scene_type, func_node)
	if func_node == 0 then return 0 end
	
	if scene_type == "general" then 
		return funcs_general[func_node].func_name
	elseif scene_type == "h_scene" then
		return funcs_h_scene[func_node].func_name
	else
		return 0
	end
	
	return 0
end

local function add_func_to_menu(scene_type, func_name)
	if func_name == 0 then return end
	local funcs_current = {}
	local button_arr_node = 0
	if scene_type == "general" then 
		funcs_current = funcs_general
	elseif scene_type == "h_scene" then
		funcs_current = funcs_h_scene
		button_arr_node = 1
	else
		return
	end
	
	for func_i, funcdata in ipairs(funcs_current) do
		if funcdata.func_name == func_name then
			RadMenuAddButton(button_arr_node, funcdata.func_name, funcdata.title, funcdata.short_desc)
			return
		end
	end
end

function on.launch()
	-- add selected functions in right order to RadMenu
	add_func_to_menu("general", mcfg.genBtn1)
	add_func_to_menu("general", mcfg.genBtn2)
	add_func_to_menu("general", mcfg.genBtn3)
	add_func_to_menu("general", mcfg.genBtn4)
	add_func_to_menu("general", mcfg.genBtn5)
	add_func_to_menu("general", mcfg.genBtn6)
	add_func_to_menu("general", mcfg.genBtn7)
	add_func_to_menu("general", mcfg.genBtn8)
	add_func_to_menu("h_scene", mcfg.hBtn1)
	add_func_to_menu("h_scene", mcfg.hBtn2)
	add_func_to_menu("h_scene", mcfg.hBtn3)
	add_func_to_menu("h_scene", mcfg.hBtn4)
	add_func_to_menu("h_scene", mcfg.hBtn5)
	add_func_to_menu("h_scene", mcfg.hBtn6)
	add_func_to_menu("h_scene", mcfg.hBtn7)
	add_func_to_menu("h_scene", mcfg.hBtn8)
	
	InitRadialMenuParams(mcfg.fontfamily, mcfg.miniversion, mcfg.fontsize, mcfg.deadzone, 
		mcfg.canceltime, mcfg.toggletype, "Move cursor to select action", "Canceled")
end

function on.start_h(hi)
	RadMenuHstatus(true)
end

function on.end_h()
	RadMenuHstatus(false)
end

function on.convo()
	RadMenuHstatus(false)
end

function _M:load()
	--mod_load_config(self, opts)
	assert(self)
	mcfg = self
	self.fontfamily = self.fontfamily or 'Arial'
	self.miniversion = self.miniversion or 0
	self.fontsize = self.fontsize or 100
	self.canceltime = self.canceltime or 500
	self.deadzone = self.deadzone or 40
	self.toggletype = self.toggletype or 0
	self.genBtn1 = self.genBtn1 or "Controls_F1"
	self.genBtn2 = self.genBtn2 or "Controls_Space"
	self.genBtn3 = self.genBtn3 or 0
	self.genBtn4 = self.genBtn4 or 0
	self.genBtn5 = self.genBtn5 or 0
	self.genBtn6 = self.genBtn6 or 0
	self.genBtn7 = self.genBtn7 or 0
	self.genBtn8 = self.genBtn8 or 0
	self.hBtn1 = self.hBtn1 or "Controls_F1"
	self.hBtn2 = self.hBtn2 or "Controls_Space"
	self.hBtn3 = self.hBtn3 or 0
	self.hBtn4 = self.hBtn4 or 0
	self.hBtn5 = self.hBtn5 or 0
	self.hBtn6 = self.hBtn6 or 0
	self.hBtn7 = self.hBtn7 or 0
	self.hBtn8 = self.hBtn8 or 0
	
	reload_buttons()
end

function _M:unload()
	Config.save()
	funcs_general = {}
	funcs_h_scene = {}
end

function _M:config()
	
	local opts = {
		"Font family: %s",
		"Minified menu: %b{Smaller version on the right side (more suitable for advanced users)}",
		"Font size, percents: %i[10,500,1]{Percent of basic font size}",
		"Cancel message time, ms: %i[10,10000,100]{1000 ms = 1 sec}",
		"Select Deadzone, px: %i[10,500,1]",
		"Toggle type: %l|Double RMB -> LMB|4th mouse button|5th mouse button|",
		"General Button 1: %l" .. funcs_gen_str,
		"General Button 2: %l" .. funcs_gen_str,
		"General Button 3: %l" .. funcs_gen_str,
		"General Button 4: %l" .. funcs_gen_str,
		"General Button 5: %l" .. funcs_gen_str,
		"General Button 6: %l" .. funcs_gen_str,
		"General Button 7: %l" .. funcs_gen_str,
		"General Button 8: %l" .. funcs_gen_str,
		"H scene Button 1: %l" .. funcs_h_str,
		"H scene Button 2: %l" .. funcs_h_str,
		"H scene Button 3: %l" .. funcs_h_str,
		"H scene Button 4: %l" .. funcs_h_str,
		"H scene Button 5: %l" .. funcs_h_str,
		"H scene Button 6: %l" .. funcs_h_str,
		"H scene Button 7: %l" .. funcs_h_str,
		"H scene Button 8: %l" .. funcs_h_str,
	}

	require "iuplua"
	require "iupluacontrols"
	local okay, fontfam, miniversion, fontsize, canceltime, deadzone, toggletype, 
			genBtn1, genBtn2, genBtn3, genBtn4, genBtn5, genBtn6, genBtn7, genBtn8,
			hBtn1, hBtn2, hBtn3, hBtn4, hBtn5, hBtn6, hBtn7, hBtn8 = iup.GetParam("Configure Radial Menu", nil, 
			table.concat(opts, "\n").."\n",
		self.fontfamily, self.miniversion, self.fontsize, 
		self.canceltime, self.deadzone, self.toggletype,
		func_name_to_node("general", self.genBtn1),
		func_name_to_node("general", self.genBtn2),
		func_name_to_node("general", self.genBtn3),
		func_name_to_node("general", self.genBtn4),
		func_name_to_node("general", self.genBtn5),
		func_name_to_node("general", self.genBtn6),
		func_name_to_node("general", self.genBtn7),
		func_name_to_node("general", self.genBtn8),
		func_name_to_node("h_scene", self.hBtn1),
		func_name_to_node("h_scene", self.hBtn2),
		func_name_to_node("h_scene", self.hBtn3),
		func_name_to_node("h_scene", self.hBtn4),
		func_name_to_node("h_scene", self.hBtn5),
		func_name_to_node("h_scene", self.hBtn6),
		func_name_to_node("h_scene", self.hBtn7),
		func_name_to_node("h_scene", self.hBtn8))
	if okay then
		self.fontfamily = fontfam
		self.miniversion = miniversion
		self.fontsize = fontsize
		self.canceltime = canceltime
		self.deadzone = deadzone
		self.toggletype = toggletype
		-- For buttons need save a uniq func names
		self.genBtn1 = func_node_to_name("general", genBtn1)
		self.genBtn2 = func_node_to_name("general", genBtn2)
		self.genBtn3 = func_node_to_name("general", genBtn3)
		self.genBtn4 = func_node_to_name("general", genBtn4)
		self.genBtn5 = func_node_to_name("general", genBtn5)
		self.genBtn6 = func_node_to_name("general", genBtn6)
		self.genBtn7 = func_node_to_name("general", genBtn7)
		self.genBtn8 = func_node_to_name("general", genBtn8)
		self.hBtn1 = func_node_to_name("h_scene", hBtn1)
		self.hBtn2 = func_node_to_name("h_scene", hBtn2)
		self.hBtn3 = func_node_to_name("h_scene", hBtn3)
		self.hBtn4 = func_node_to_name("h_scene", hBtn4)
		self.hBtn5 = func_node_to_name("h_scene", hBtn5)
		self.hBtn6 = func_node_to_name("h_scene", hBtn6)
		self.hBtn7 = func_node_to_name("h_scene", hBtn7)
		self.hBtn8 = func_node_to_name("h_scene", hBtn8)
		
		Config.save()
	end
end

return _M

