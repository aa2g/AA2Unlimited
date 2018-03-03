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

function _M.readfile(path)
    local file = io.open(path, "rb")
    if not file then return nil end
    local data = file:read "*a"
    local size = file:seek()
    file:close()
    return data, size
end

function _M.getfiledialog(pattern)
	local file
	local ret
	file, ret = iup.GetFile(pattern)
	if ret == 0 then
		return file
	end
end


return _M
