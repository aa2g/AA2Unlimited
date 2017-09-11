local _M = {}

-- splits a path in its base directory, final component name and extension, if any of those exist
function _M.splitfilepath(path)
	local directory
	local file
	local filename
	local extension
	local dot
	
	directory, file = string.match(path, "(.*\\)(.*)")
	if not directory then
		file = path
	end
	
	filename, dot, extension =  string.match(file, "(.*)(%.)(.*)")
	filename = filename or file
	
	return directory, filename, extension
end

return _M
