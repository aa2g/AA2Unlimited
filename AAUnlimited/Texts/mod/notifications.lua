--@INFO Shows some of action notifications

local _M = {}
local opts = {
	{ "fontFam", "Arial", "Font family: %s" },
	{ "fontSize", 24, "Font size, px: %i[1,]" },
	{ "lineHeight", 120, "Line height, percents: %i[100,300]{Percent of Font size param (Not work if params `Separate color for important` and `Outline quality` are disabled)}" },
	{ "duration", 8, "Show duration, sec: %i[1,]" },
	{ "maxLines", 5, "Maximum number of lines: %i[1,]"},
	
	{ "textColNormal", "255 190 100", "Text color (main) RGB: %c"},
	{ "diffColForImp", 1, "Separate color for important: %b"},
	{ "textColImp", "255 70 25", "Text color (important) RGB: %c"},
	
	{ "outlineQuality", 2, "Outline quality: %l|Only text (Off)|With Shadow (Med)|With Outline (High)|{Higher values can slightly affect performance}" },
	{ "outlineSpread", 2, "Text outline spread, px: %i[1,10]"},
	{ "outlineColor", "0 0 0", "Outline color RGB: %c"},
	{ "outlineColorA", 255, "Outline Alpha: %i[0,255]"},
	
	{ "textAlign", 0, "Text Alignment: %l|Left|Center|{(if `Center`, param `Position X` not working)}"},
	{ "areaPosX", 2.0, "Notifications Position X, percents: %r[0,95,0.1]{Percent of Game window Width (not works, if param `Alignment` set to `Center`)}"},
	{ "areaPosY", 75.0, "Notifications Position Y, percents: %r[0,100,0.1]{(Bottom edge coordinate!) Percent of Game window Height}"},
}

function on.launch()
	InitNotificationsParams(opts.fontFam, opts.fontSize, opts.lineHeight, opts.duration, opts.maxLines,
		opts.textColNormal, opts.diffColForImp, opts.textColImp, 
		opts.outlineQuality, opts.outlineSpread, opts.outlineColor, opts.outlineColorA,
		opts.textAlign, math.ceil(opts.areaPosX * 100), math.ceil(opts.areaPosY * 100))
end

function _M:load()
	mod_load_config(self, opts)
	if opts.areaPosX > 95 or opts.areaPosY > 100 then
		opts.areaPosX = 2.0;
		opts.areaPosY = 75.0;
	end
end

function _M:unload()
	
end

function _M:config()
	mod_edit_config(self, opts, "Notifications options")
end

return _M

