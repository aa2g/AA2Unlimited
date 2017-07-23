require "memory"

local strbuf = malloc(4096)

function cp_to_unicode(cp, text)
	local nconv = MultiByteToWideChar(cp, 0, text, #text, strbuf, 4096)
	return peek(strbuf, nconv*2)
end

function unicode_to_cp(cp, text)
	local nconv = WideCharToMultiByte(cp, 0, text, #text / 2, strbuf, 4096, 0, 0)
	return peek(strbuf, nconv)
end

function sjis_to_unicode(text)
	return cp_to_unicode(932, text)
end

function utf8_to_unicode(text)
	return cp_to_unicode(65001, text)
end

function unicode_to_utf8(text)
	return unicode_to_cp(65001, text)
end

function sjis_to_utf8(text)
	return unicode_to_utf8(sjis_to_unicode(text))
end
