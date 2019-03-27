--@INFO Subtitles loader

local _M = {}
--local opts = {
--	{ "maxlines", 4, "Maximum number of lines to show %i[1,10]"},
--	{ "duration", 3, "Subtitles duration %i[1,10]:" },
--}

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
	if sub then AddSubtitles(sub) end
end

function _M:load()
	-- mod_load_config(self, opts)
	reload_subtitles()
end

function _M:unload()
	subtitles = {}
end

--function _M:config()
--	mod_edit_config(self, opts, "Subtitles options")
--end

return _M

