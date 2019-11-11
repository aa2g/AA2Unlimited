--@INFO Club/Place translations for AA2Play.exe

-- some entries need a string length, too
local fixup = {
	[0xDBE64] = 0xDBE5F,
	[0xDBF06] = 0xDBEFE,
	[0xDBFA6] = 0xDBF9E,
	[0xDC04C] = 0xDC044,
	[0xDC0F2] = 0xDC0EA,
	[0xDC198] = 0xDC190,
	[0xDC23C] = 0xDC238,
	[0xDC2EA] = 0xDC2E2,
}

local dict = {
[0x2D154] = "None",
[0xF054] = "None",
[0xEFE9] = "None",

[0x55322] = "Club Tournament",
[0x55329] = "Physical test",
[0x55330] = "Academic test",

[0xDBE64] = "Track",
[0xF4DE4] = "Track",
[0xF4E21] = "Swimming",
[0xDBF06] = "Swimming",
[0xF4E52] = "Baseball",
[0xDBFA6] = "Baseball",
[0xF4E94] = "Volley",
[0xDC04C] = "Volley",
[0xF4ECD] = "Dojo",
[0xDC0F2] = "Dojo",
[0xF4EE5] = "Fine arts",
[0xDC198] = "Fine arts",
[0xF4F27] = "Culture",
[0xDC23C] = "Culture",
[0xF4F47] = "Other",
[0xDC2EA] = "Other",


[0xE0FE4] = "School gates",
[0xE0FF2] = "Back street",
[0xE1000] = "Outside gymnasium",
[0xE100E] = "School route",
[0xE101C] = "Mens changing room",
[0xE102A] = "Girls changing room",
[0xE1038] = "Mens shower",
[0xE1046] = "Girls shower",
[0xE1054] = "Lockers",
[0xE1062] = "Outside lounge",
[0xE1070] = "Outside toilets",
[0xE107E] = "Outside classroom"
[0xE108C] = "Rooftop access",,
[0xE109A] = "Old building 1st floor",
[0xE10A8] = "Old building 2nd floor",
[0xE10B6] = "Old building 3rd floor",
[0xE10C4] = "Teachers lounge",
[0xE10D2] = "Infirmary",
[0xE10E0] = "Library",
[0xE10EE] = "Classroom",
[0xE10FC] = "Mens Toilets",
[0xE110A] = "Girls Toilets",
[0xE1118] = "Rooftop",
[0xE1126] = "Outside counsel",
[0xE1134] = "Outside cafeteria",
[0xE1142] = "Courtyard",
[0xE1150] = "2nd floor hallway",
[0xE115E] = "3rd floor passage",
[0xE116C] = "Swimming pool",
[0xE117A] = "Track",
[0xE1188] = "Sports facility",
[0xE1196] = "Dojo",
[0xE11A4] = "Gymnasium",
[0xE11B2] = "Arts room",
[0xE11C0] = "Multipurpose room",
[0xE11CE‬] = "Japanese room",
[0xE11DC] = "Behind Dojo",
[0xE11EA] = "Outside dojo",
[0xE11F8] = "Cafeteria",
[0xE1206] = "Outside Station",
[0xe123E] = "Boys' room",
[0xE124C] = "Girls' room",
[0xE125A] = "Boys's Shower Stall",
[0xE1268] = "Girl's Shower Stall",
[0xE1276] = "Boys' Toilet Stall",
[0xE1284] = "Girls' Toilet Stall",
[0xE1292] = "Counseling Room",
[0xE12A0] = "Gym Storeroom",
[0xE12AE] = "Love Hotel",
[0xE12BC] = "Machine Room",
}

local _M = {}

local save = {}
local stringbuf

function _M:load()
	if exe_type ~= "play" then return end

	-- translate the strings from utf8 to unicode
	local buf = ""
	local rel = {}
	for k,v in pairs(dict) do
		rel[k] = #buf
		buf = buf .. utf8_to_unicode(v) .. "\x00\x00"
	end

	-- copy it to their own memory page
	stringbuf = malloc(#buf)
	poke(stringbuf, buf)

	for ptr, off in pairs(rel) do
		local orig_bytes = g_peek(ptr, 4)
		local orig = g_peek_dword(ptr)
		local ran = orig-GameBase-0x00320000
		if (ran > 0xffff) or (ran < 0) then
			log.warn("playtrans: Invalid pointer %x at %x for '%s', skipping translation", ran, ptr, dict[ptr])
		else
			g_poke_dword(ptr, stringbuf + off)
			save[ptr] = orig_bytes
			local ptr2 = fixup[ptr]
			if ptr2 then
				save[ptr2] = g_poke(ptr2, string.char(#dict[ptr]))
			end
		end
	end
end

function _M:unload()
	for ptr,old in ipairs(save) do
		g_poke(ptr, old)
	end
	free(stringbuf)
	save = {}
	stringbuf = nil
end

return _M
