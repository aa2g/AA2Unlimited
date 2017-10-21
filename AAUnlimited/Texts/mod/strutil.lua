require "memory"

local strbuf = malloc(4096)

function strdup(s)
	local nb = malloc(#s)
	poke(nb, s)
	return nb
end

function cp_to_unicode(cp, text)
	local nconv = MultiByteToWideChar(cp, 0, text, #text, strbuf, 4096)
	return peek(strbuf, nconv*2)
end

function cp_strdup(cp, text, prepend)
	text = text .. "\0"
	local strbuf = malloc(4096)
	local nconv = MultiByteToWideChar(cp, 0, text, #text, strbuf + prepend, 4096)
	return strbuf + prepend, nconv-1
end

function unicode_to_cp(cp, text)
	local nconv = WideCharToMultiByte(cp, 0, text, #text / 2, strbuf, 4096, 0, 0)
	return peek(strbuf, nconv)
end

function sjis_to_unicode(text)
	return cp_to_unicode(932, text)
end

-- use lua's unicode support, so that we can call this early on
function utf8_to_unicode(text)
	local res = {}
	for _, c in utf8.codes(text) do
		table.insert(res, string.pack("<H", c))
	end
	return table.concat(res)
--	return cp_to_unicode(65001, text)
end

function unicode_to_utf8(text)
	if type(text) == "number" then
		text = peek(text, 256, "\x00\x00", 2)
	end
	return unicode_to_cp(65001, text)
end

function sjis_to_utf8(text)
	return unicode_to_utf8(sjis_to_unicode(text))
end

function illusion_string(s)
	local buf, slen = cp_strdup(65001, s, 16)
	local meta = buf-16
	poke(meta, string.pack("<IIII", GameBase + 0x36f638, slen, 2048, 999))
	return buf
end
