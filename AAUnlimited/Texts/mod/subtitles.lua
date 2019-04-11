--@INFO Subtitles loader

local _M = {}
local opts = {
	{ "fontfam", 0, "Font family: %l|Arial|Tahoma|" },
	{ "fontsize", 24, "Font size, px: %i[1,100]" },
	{ "lineheight", 120, "Line height, percents: %i[100,300]{Percent of Font size param}" },
	{ "duration", 5, "Show duration, sec: %i[1,60]" },
	{ "maxlines", 4, "Maximum number of lines: %i[1,20]"},
	
	{ "textcolfemale", "255, 155, 255", "Text color (female) RGB/HEX: %s{Example `255,45,23` / `F9D0e1` / `c7F` etc. }"},
	{ "textcolmale", "155, 244, 244", "Text color (male) RGB/HEX: %s{Example `255,45,23` / `F9D0e1` / `c7F` etc. }"},
	
	{ "outlineW", 2, "Text outline width, px: %i[1,8]"},
	{ "outlinecolor", "0, 0, 0", "Outline color RGB/HEX: %s{Example `255,45,23` / `F9D0e1` / `c7F` etc. }"},
	{ "outlinecolorA", 255, "Outline Alpha: %i[0,255]"},
	
	{ "textalign", 0, "Text Alignment: %l|Left|Center|{(if `Center`, param `Position X` not working)}"},
	{ "areaposX", 15, "Subs Position X, px: %i[0,3000]{not works, if param `Alignment` set to `Center`}"},
	{ "areaposY", 45, "Subs Position Y, px: %i[0,3000]"},
}

local subtitles = {}

local function reload_subtitles()
	local dialogue
	local count = 0
	local subtitles_path = aau_path("subtitles.txt")
	local file = io.open(subtitles_path, "r")
	if not file then return end
	for line in file:lines() do
		file, dialogue = line:match("%s*([%w_.]+).+\"(.*)\"%s*$")
		if file and dialogue then
			subtitles[file] = dialogue
			count = count + 1
		end
	end
	log.info("Loaded %d subtitles", count)
end

function on.load_audio(fname)
	local sub = subtitles[fname]
	if sub then AddSubtitles(sub, fname) end
end

function on.launch()
	InitSubtitlesParams(opts.fontfam, opts.fontsize, opts.lineheight, opts.duration, opts.maxlines,
		opts.textcolfemale, opts.textcolmale, opts.outlineW, opts.outlinecolor, opts.outlinecolorA,
		opts.textalign, opts.areaposX, opts.areaposY)
end

function _M:load()
	mod_load_config(self, opts)
	reload_subtitles()
end

function _M:unload()
	subtitles = {}
end

function _M:config()
	mod_edit_config(self, opts, "Subtitles options")
end

return _M

