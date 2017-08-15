--@INFO Dump relationship stats commands

local _M = {}

function char_relations(char)
	local pos = 0
	return function()
		local rel = char:GetRelation(pos)
		pos = pos+1
		return rel
	end
end

function get_char_name(seat)
	local char = GetCharacter(seat).m_charData
	return "%s %s" % {char.m_forename, char.m_surname}
end

function get_relations(seat)
	local char = GetCharacter(seat)
	if not char then return nil end
	local res = {}
	for rel in char_relations(char) do
--		res[rel.m_targetSeat] = { rel.m_lovePoints, rel.m_likePoints, rel.m_dislikePoints, rel.m_hatePoints }
		res[rel.m_targetSeat] = { rel.m_loveCount, rel.m_likeCount, rel.m_dislikeCount, rel.m_hateCount }
	end
	return res
end

function get_all_relations()
	local reltab = {}
	for i=0,24 do
		reltab[i] = get_relations(i)
	end
	return reltab
end

-- saved relations at start of period
local reltab = {}
function init_relations()
	reltab = get_all_relations()
end
function on.period(x)
	init_relations()
	return x
end


function relations_of(source,target,onlychanged)
	local function n(d)
		if d == 0 then return "   " end
		return ("%s%-2d" % {(d>0 and "+") or (d<0 and "-"),math.abs(d)})
	end

	for towards=0,24 do
		local rel = get_relations(source)
		if not rel then
			break
		end
		local v = rel[towards]
		if v then 
			local o = ((reltab[source] or {})[towards]) or {0,0,0,0}
			if ((not target) or (towards == target)) then
				local diff1 = v[1] - o[1]
				local diff2 = v[2] - o[2]
				local diff3 = v[3] - o[3]
				local diff4 = v[4] - o[4]
				local haschange = diff1 ~= 0 or diff2 ~= 0 or diff3 ~= 0 or diff4 ~= 0

				if (not onlychanged) or haschange then
					raw_print("%02d>%02d %-2d%s %-2d%s %-2d%s %-2d%s\n" % {
						source, towards,
						v[1], n(diff1),
						v[2], n(diff2),
						v[3], n(diff3),
						v[4], n(diff4)})
				end
			end
		end
	end
end

function relations(from, towards, onlychanged)

	if from and (not GetCharacter(from)) then
		info("Source seat  %d is not occupied", from)
		return
	end

	if towards and (not GetCharacter(towards)) then
		info("Source seat %d is not occupied", towards)
		return
	end	
	info("Relations of '%s' towards '%s'" % {
		from and get_char_name(from) or "*everyone*",
		towards and get_char_name(towards) or "*everyone*"
	})

	if onlychanged then
		info("Listing changes since period start")
	end

	raw_print("Seats Love  Like  Disl. Hate\n")

	for seat=0,24 do
		if ((not from) or (from == seat)) then
			relations_of(seat, towards,onlychanged)
		end
	end
end

function reldiff(from,towards)
	return relations(from,towards,true)
end

local lastc
function on.answer(resp, as)
	if (as.askingChar.m_thisChar == GetPlayerCharacter()) then
		lastc = as.answerChar.m_thisChar.m_seat
	elseif (as.answerChar.m_thisChar == GetPlayerCharacter()) then
		lastc = as.askingChar.m_thisChar.m_seat
	end
	return resp
end
function relanswer()
	local pc = GetPlayerCharacter().m_seat
	relations(lastc, pc)
	relations(pc, lastc)
end

function get_characters()
	local pos=0
	return function()
		while pos < 25 do
			local char = GetCharacter(pos)
			pos = pos+1
			if char then
				return char
			end
		end
	end
end

function cards()
	info("Dumping class cards:")
	raw_print " # | Name\n"
	raw_print "------------------------------\n"

	for char in get_characters() do
		local data = char.m_charData
		raw_print("%2d | %s %s\n" % {char.m_seat, data.m_forename, data.m_surname})
	end
end



function _M:load()
	init_relations()
end

function _M:unload()
end

function rel()
	reload_module("stats")
end

return _M
