pp2matched = {}
function pp2match(n,limit)
	limit = limit or 20
	local matched = {}
	local nmatched = 0
	pp2matched = matched
	n = n:lower()
	info('matching ' .. n)
	for pp2idx,pp2name in ipairs {PP2List()} do
		--print(pp2name)
		for fname,fidx in pairs(PP2GetNames(pp2idx-1)) do
			--print(pp2name,fname,fidx)
			--if true then return end
			if fname:lower():match(n) then
				local ppname, fname2 = fname:match("^(.+)%.pp/([^/]*)$")
				if ppname and (not matched[fname]) then
					matched[fname] = {pp2idx-1,fidx,ppname,fname2}
					if nmatched < limit then
						info("Found %s:\t%s/%s" % {pp2name, ppname, fname2})
					end
					nmatched = nmatched+1
				end
			end
		end
	end
	if nmatched >= limit then
		info("... and %d more files not shown" % (nmatched-limit))
	end
	info("Found %d total. Type pp2dump() to extract those." % {nmatched})
end

function pp2dump(n)
	local dpath = play_path('data')
	info("Dumping files into %s" % {dpath})
	for _,v in pairs(pp2matched) do
		local pp2idx, fidx, ppname, fname = table.unpack(v)
		local data = PP2ReadFile(pp2idx, fidx)
		CreateDirectoryA(play_path('data',ppname), 0)
		local tgt = play_path('data',ppname,fname)
		if tgt:match("%.wav$") then
			tgt = tgt .. ".opus"
		end
		info("Dumping %s" % tgt)
		local fo = io.open(tgt, "wb")
		fo:write(data)
		fo:close()
	end
end